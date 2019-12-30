#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>
#include <ctype.h>

// Prototype Functions
void *get_nth_line(void *params);

int measure_text(char *filename);

void *read_list(void *param);

void *to_upper(void *params);

void *replace(void *params);

// Thread arguments
typedef struct t_params {
    pthread_t tid;
    char *filename;
    size_t id;
    char **r_lines;
    sem_t *mutex_arr;
    int length;
    int thread_number;
} t_params;
//typedef t_params *tParams;

sem_t counter_mutex;
sem_t upper_count_mutex;
sem_t replace_count_mutex;
sem_t upper_sem;
sem_t replace_sem;
int line_counter = 0;
int upper_count = 0;
int replace_count = 0;
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

    sem_t mutex_arr[line_number];
    for (int l = 0; l < line_number; ++l) {
        sem_init(&mutex_arr[l], 0, 1);
    }
    sem_init(&upper_sem, 0, 0);
    sem_init(&replace_sem, 0, 0);
    sem_init(&upper_count_mutex, 0, 1);
    sem_init(&replace_count_mutex, 0, 1);
    sem_init(&counter_mutex, 0, 1); //init binary semaphore

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
        rc2 = pthread_create(&replacer[a], NULL, replace, (void *) &replace_param[a]);
        if (rc2) {
            printf("ERROR; return code from pthread_create() is %d\n", rc2);
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
//    for (int l = 0; l < 500; ++l) {
//        printf("index: %d line: %s\n", l, read_lines[l]);
//    }


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
                printf("Thread %zu read line: %s", t_p->id, line);

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
        printf("changed line: %s", upper);
//        printf("%s", upper);
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
        if (replace_count == 0) {
            sem_wait(&replace_sem);
        }
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
        printf("changed line: %s\n", replace);

        sem_post(&(t_p->mutex_arr[index])); // function critical section ends

        if (index > t_p->length - t_p->thread_number - 1) { //bitiş şartı
//            printf("thread: %zu is done\n", t_p->id);
            break;
            pthread_exit(NULL);
        }
    }
}

