#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>
#include <ctype.h>

#define MAX 256

// Prototype Functions
void *get_nth_line(void *params);

int measure_text(char *filename);

void *read_list(void *param);

void *to_upper(void *params);

void *replace(void *params);

void *write_file(void *params);

void duplicate_file(char *dest, char *src);

// Thread arguments
typedef struct t_params {
    pthread_t tid;
    char *filename;
    size_t id;
    char **r_lines;
    sem_t *mutex_arr;
    int length;
    int thread_number;
    int *write_ready;
} t_params;
//typedef t_params *tParams;

sem_t counter_mutex;
sem_t upper_count_mutex;
sem_t replace_count_mutex;
sem_t upper_sem;
sem_t replace_sem;
sem_t write_sem;
sem_t write_count_mutex;
sem_t write_ready_mutex;
sem_t write_mutex;
int line_counter = 0;
int upper_count = 0;
int replace_count = 0;
int write_count = 0;
char *output_file;
//char *read_lines2[20000];


int main(int argc, char *argv[]) {
//    printf("%d", argc);
    if ((argc != 8) || strstr(argv[1], "-d") == NULL || strstr(argv[3], "-n") == NULL) {
        perror("ERROR: INVALID ARGUMENTS");
        perror("USAGE: ./a.out -d <file.txt> -n #readThread #UpperThread #ReplaceThread #WriteThread");
    }
    long NUM_READ_THREADS = atoi(argv[4]);
    long NUM_UPPER_THREADS = atoi(argv[5]);
    long NUM_REPLACE_THREADS = atoi(argv[6]);
    long NUM_WRITE_THREADS = atoi(argv[7]);

    int line_number = measure_text(argv[2]);
    char *read_lines[line_number];
    int write_ready[line_number];
    for (int j = 0; j < line_number; ++j) {
        write_ready[j] = 0;
    }

    output_file = "out.txt";
    duplicate_file(output_file, argv[2]);

    sem_t mutex_arr[line_number];
    for (int l = 0; l < line_number; ++l) {
        sem_init(&mutex_arr[l], 0, 1);
    }
    sem_init(&upper_sem, 0, 0);
    sem_init(&replace_sem, 0, 0);
    sem_init(&upper_count_mutex, 0, 1);
    sem_init(&replace_count_mutex, 0, 1);
    sem_init(&counter_mutex, 0, 1); //init binary semaphore
    sem_init(&write_sem, 0, 0);
    sem_init(&write_count_mutex, 0, 1);
    sem_init(&write_ready_mutex, 0, 1); //
    sem_init(&write_mutex, 0, 1);

/* READ THREAD INITIALIZATION */
    t_params read_param[NUM_READ_THREADS];
    pthread_t readers[NUM_READ_THREADS];
    int rc;
    for (int i = 0; i < NUM_READ_THREADS; ++i) {
        read_param[i].id = i;
        read_param[i].filename = strdup(argv[2]);
        read_param[i].r_lines = read_lines;
        read_param[i].mutex_arr = mutex_arr;
        read_param[i].length = line_number;
        read_param[i].thread_number = NUM_READ_THREADS;
        rc = pthread_create(&readers[i], NULL, get_nth_line, (void *) &read_param[i]);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    /* UPPER THREAD INITIALIZATION */
    t_params upper_param[NUM_UPPER_THREADS];
    pthread_t uppers[NUM_UPPER_THREADS];
    int rc1;
    for (int m = 0; m < NUM_UPPER_THREADS; ++m) {
        upper_param[m].length = line_number;
        upper_param[m].r_lines = read_lines;
        upper_param[m].mutex_arr = mutex_arr;
        upper_param[m].id = m;
        upper_param[m].thread_number = NUM_UPPER_THREADS;
        upper_param[m].write_ready = write_ready;
        rc1 = pthread_create(&uppers[m], NULL, to_upper, (void *) &upper_param[m]);
        if (rc1) {
            printf("ERROR; return code from pthread_create() is %d\n", rc1);
            exit(-1);
        }
    }

/* REPLACE THREAD INITIALIZATION */
    t_params replace_param[NUM_REPLACE_THREADS];
    pthread_t replacer[NUM_REPLACE_THREADS];
    int rc2;
    for (int a = 0; a < NUM_REPLACE_THREADS; ++a) {
        replace_param[a].length = line_number;
        replace_param[a].r_lines = read_lines;
        replace_param[a].mutex_arr = mutex_arr;
        replace_param[a].id = a;
        replace_param[a].thread_number = NUM_REPLACE_THREADS;
        replace_param[a].write_ready = write_ready;
        rc2 = pthread_create(&replacer[a], NULL, replace, (void *) &replace_param[a]);
        if (rc2) {
            printf("ERROR; return code from pthread_create() is %d\n", rc2);
            exit(-1);
        }
    }




/* WRITE THREAD INITIALIZATION */
    t_params write_param[NUM_WRITE_THREADS];
    pthread_t writer[NUM_WRITE_THREADS];
    int rc3;
    for (int k = 0; k < NUM_WRITE_THREADS; ++k) {
        write_param[k].length = line_number;
        write_param[k].r_lines = read_lines;
        write_param[k].mutex_arr = mutex_arr;
        write_param[k].id = k;
        write_param[k].thread_number = NUM_WRITE_THREADS;
        rc3 = pthread_create(&writer[k], NULL, write_file, (void *) &write_param[k]);
        if (rc3) {
            printf("ERROR; return code from pthread_create() is %d\n", rc3);
            exit(-1);
        }
    }

    for (int k = 0; k < NUM_READ_THREADS; ++k) {
        pthread_join(readers[k], NULL);
    }
    for (int n = 0; n < NUM_UPPER_THREADS; ++n) {
        pthread_join(uppers[n], NULL);
    }
    for (int b = 0; b < NUM_REPLACE_THREADS; ++b) {
        pthread_join(replacer[b], NULL);
    }

    for (int l = 0; l < NUM_WRITE_THREADS; ++l) {
        pthread_join(writer[l], NULL);
    }
//    for (int l = 0; l < 500; ++l) {
//        printf("index: %d line: %s\n", l, read_lines[l]);
//    }

//    int sem_val;
//    sem_getvalue(&write_sem, &sem_val);
//    printf("semaphore is: %d\n", sem_val);

/* destroy semaphore */
    sem_destroy(&counter_mutex);
    sem_destroy(&upper_sem);
    sem_destroy(&replace_sem);
    sem_destroy(&upper_count_mutex);
    sem_destroy(&replace_count_mutex);
    for (int j = 0; j < line_number; ++j) {
        sem_destroy(&mutex_arr[j]);
    }
    puts("heloğ");
    for (int m = 0; m < line_number; ++m) {
        printf("%s\n", read_lines[m]);
    }
//    t_params new;
//    new.r_lines = read_lines;
//    to_upper(&new);
    return 0;
}


void *get_nth_line(void *params) {
    t_params *t_p = (t_params *) params;
//    t_p->tid = pthread_self();
    char *filename = strdup(t_p->filename);
    int line_no, index;
    int flag = 1;
    while (1) {
        flag = 1;
        sem_wait(&counter_mutex); // down semaphore
        /* START CRITICAL SECTION */
        line_no = line_counter;
        index = line_counter;
        sem_wait(&t_p->mutex_arr[index]);
        line_counter++;
        /* END CRITICAL SECTION */
        sem_post(&counter_mutex); // up semaphore

//        printf("take mutex %d\n", index);
        FILE *file = fopen(filename, "r");
        char line[256];
        int i = 0;
        while (fgets(line, sizeof(line), file)) {
            if (line == NULL || line_no < 0) {
                return NULL;
            }
            if (line_no == 0) {
                flag = 0;
//                printf("Thread %zu read line: %s", t_p->id, line);

                t_p->r_lines[index] = strdup(line);

                sem_post(&upper_sem);
                sem_post(&replace_sem);

                break;
            }
            line_no--;
        }
        fclose(file);
        sem_post(&t_p->mutex_arr[index]);

//        printf("give mutex %d\n", index);
        if (index > t_p->length - t_p->thread_number - 1) {
            break;
        }
//        if (flag == 1) {
////            sem_post(&t_p->mutex_arr[index]);
//            pthread_exit(NULL);
//        }
//        read_lines2[index] = strdup(line);
//        sem_post(&arr_mutex);
    }

}

int measure_text(char *filename) {
    FILE *file = fopen(filename, "r");
    char line[512];
    int line_number = 0;
    while (fgets(line, sizeof(line), file)) {
        line_number++;
    }
    fclose(file);
    return line_number;
}


void *to_upper(void *params) {
    t_params *t_p = (t_params *) params;
    int index;
    /*critical section */
    while (1) {
        sem_wait(&upper_sem);

        sem_wait(&upper_count_mutex);
        index = upper_count;
        sem_wait(&(t_p->mutex_arr[index])); // function critical section starts
        upper_count++;
        sem_post(&upper_count_mutex);

        /*critical section */

        char *line = t_p->r_lines[index];
        char upper[strlen(line) + 1];
        int i;
        for (i = 0; i < strlen(line); ++i) {
            upper[i] = toupper(line[i]);
        }
        upper[i] = '\0';
        t_p->r_lines[index] = strdup(upper);
//        printf("changed line: %s\n", upper);
//        printf("%s", upper);

        sem_wait(&write_ready_mutex);
        t_p->write_ready[index]++;
        if (t_p->write_ready[index] == 2) {
            sem_post(&write_sem);
        }
        sem_post(&write_ready_mutex);
        sem_post(&(t_p->mutex_arr[index]));// function critical section ends.

        if (index > t_p->length - t_p->thread_number - 1) { //bitiş şartı
//            printf("thread: %zu is done\n", t_p->id);
            break;
            pthread_exit(NULL);
        }
    }
    pthread_exit(NULL);
}

void *replace(void *params) {
    t_params *t_p = (t_params *) params;
    int index;
    while (1) {
        /*critical section*/
        sem_wait(&replace_sem);
        sem_wait(&replace_count_mutex);
        index = replace_count;
        sem_wait(&(t_p->mutex_arr[index])); // function critical section starts
        replace_count++;

        sem_post(&replace_count_mutex);

        /*critical section*/

        char *line = t_p->r_lines[index];
        char line_copy[256];
        strcpy(line_copy, line);
//        printf("line copy is: %s", line_copy);
        char replace[256];
        int i;
        for (i = 0; i < strlen(line); ++i) {
            if (isspace(line[i])) {
                replace[i] = '_';
                continue;
            }
            replace[i] = line[i];
        }
        replace[i - 1] = '\0';

        t_p->r_lines[index] = strdup(replace);
//        printf("changed line: %s\n", replace);

        sem_wait(&write_ready_mutex);
        t_p->write_ready[index]++;
        if (t_p->write_ready[index] == 2) {
            sem_post(&write_sem);
        }
        sem_post(&write_ready_mutex);

        sem_post(&(t_p->mutex_arr[index])); // function critical section ends

        if (index > t_p->length - t_p->thread_number - 1) { //bitiş şartı
//            printf("thread: %zu is done\n", t_p->id);
            break;
            pthread_exit(NULL);
        }
    }
}

void *write_file(void *params) {
    t_params *t_p = (t_params *) params;
    int index;
    while (1) {

//        sem_wait(&write_sem);

        sem_wait(&write_count_mutex);
        index = write_count;
        sem_wait(&(t_p->mutex_arr[index])); // function critical section starts
        write_count++;

        /*critical section */
        char *new_content = strdup(t_p->r_lines[index]);
        char *filename = output_file;
        int line_no = index;
        printf("content is: %s\n", t_p->r_lines[1]);
        printf("line no: %d\n", line_no);

        FILE *fptr1, *fptr2;
        int lno, linectr = 0;
        char str[MAX], fname[MAX];
        char newln[MAX], temp[] = "temp.txt";

//        sem_wait(&write_mutex);

        fptr1 = fopen(filename, "r");
        if (!fptr1) {
            printf("Unable to open the input file!!\n");
            return 0;
        }
        fptr2 = fopen(temp, "w");
        if (!fptr2) {
            printf("Unable to open a temporary file to write!!\n");
            fclose(fptr1);
            return 0;
        }

        while (!feof(fptr1)) {
            strcpy(str, "\0");
            fgets(str, MAX, fptr1);
            if (!feof(fptr1)) {
                if (linectr != line_no) {
                    fprintf(fptr2, "%s", str);
                } else {
                    fprintf(fptr2, "%s", new_content);
                    fprintf(fptr2, "%c", '\n');
                }
                linectr++;
            }
        }
        fclose(fptr1);
        fclose(fptr2);
        remove(filename);
        rename(temp, filename);
        printf(" Replacement did successfully..!! \n");
        sem_post(&(t_p->mutex_arr[index]));
//        sem_post(&write_mutex);
        sem_post(&write_count_mutex);

//        break;
        if (index > t_p->length - t_p->thread_number - 1) { //bitiş şartı
//            printf("thread: %zu is done\n", t_p->id);
            break;
            pthread_exit(NULL);
        }
    }
}

void duplicate_file(char *dest, char *src) {
    FILE *fptr1, *fptr2;
    char filename[100], c;

//    printf("Enter the filename to open for reading \n");
//    scanf("%s", filename);

    // Open one file for reading
    fptr1 = fopen(src, "r");
    if (fptr1 == NULL) {
        printf("Cannot open file %s \n", src);
        exit(0);
    }
//
//    printf("Enter the filename to open for writing \n");
//    scanf("%s", filename);

    // Open another file for writing
    fptr2 = fopen(dest, "w");
    if (fptr2 == NULL) {
        printf("Cannot open file %s \n", dest);
        exit(0);
    }

    // Read contents from file
    c = fgetc(fptr1);
    while (c != EOF) {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }

    printf("\nContents copied to %s", dest);

    fclose(fptr1);
    fclose(fptr2);
}