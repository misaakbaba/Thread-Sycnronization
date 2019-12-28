#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>


// Prototype Functions
void *get_nth_line(void *params);

int measure_text(char *filename);

void *read_list(void *param);

// Thread arguments
typedef struct t_params {
    pthread_t tid;
    char *filename;
    size_t id;
    char **r_lines;
} t_params;
typedef t_params *tParams;

sem_t counter_mutex;
sem_t arr_mutex;
int line_counter = 0;
char *read_lines2[20000];


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
    sem_init(&counter_mutex, 0, 1);//init binary semaphore
    sem_init(&arr_mutex, 0, 1);
    t_params param[NUM_READ_THREADS];
    pthread_t readers[NUM_READ_THREADS];
    int rc;
    for (int i = 0; i < NUM_READ_THREADS; ++i) {
        param[i].id = i;
        param[i].filename = strdup(argv[2]);
        param[i].r_lines = read_lines;
        rc = pthread_create(&readers[i], NULL, get_nth_line, (void *) &param);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (int k = 0; k < NUM_READ_THREADS; ++k) {
        pthread_join(readers[k], NULL);
    }
    int j = 0;
//    while (param[0].r_lines[j] != NULL) {
//        printf("%s", param[0].r_lines[j]);
//        j++;
//    }
    for (int l = 0; l < 500; ++l) {
        printf("index: %d line: %s\n", l, read_lines[l]);
    }
//    t_params *param = malloc(sizeof(t_params));
//    param->filename = strdup(argv[2]);
//    param->r = read_lines2;
//    param.r_lines = read_lines;
//    pthread_t pthread1;
//    pthread_t pthread2;
//    pthread_t pthread3;
//    pthread_t pthread4;
//    pthread_t pthread5;
//    pthread_t pthread6;
//    pthread_create(&pthread1, NULL, &get_nth_line, (void *) (&param));
//    pthread_create(&pthread2, NULL, &get_nth_line, (void *) (&param));
//    for (int i = 0; i < 5000; ++i) {
//
//    }
//    pthread_create(&pthread4, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread5, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread6, NULL, get_nth_line, (void *) (&param));
//
//    pthread_join(pthread1, NULL);
//    pthread_join(pthread2, NULL);
//    pthread_create(&pthread3, NULL, &read_list, (void *) (&param));
//    pthread_join(pthread3, NULL);

    sem_destroy(&counter_mutex); /* destroy semaphore */
    sem_destroy(&arr_mutex); /* destroy semaphore */

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
        line_counter++;
        /* END CRITICAL SECTION */
        sem_post(&counter_mutex); // up semaphore

        FILE *file = fopen(filename, "r");
        char line[256];
        int i = 0;
        while (fgets(line, sizeof(line), file)) {
            if (line == NULL || line_no < 0) {
                return NULL;
            }
            if (line_no == 0) {
                flag = 0;
                break;
            }
            line_no--;
        }
        fclose(file);

//        printf("Thread %lu read line: %s", pthread_self(), line);
//        sem_wait(&arr_mutex);
        t_p->r_lines[index] = strdup(line);
        if (flag == 1) {
            pthread_exit(NULL);
        }
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

//void *read_list(void *param) {
//    t_params *t_p = param;
//    int i = 0;
//    char **list = t_p->r;
//    while (list[i] != NULL) {
//        fprintf(stderr, "%s", list[i]);
//        i++;
//    }
//}

void *to_upper(void *param) {

}

void *add_underscore(void *param) {

}

