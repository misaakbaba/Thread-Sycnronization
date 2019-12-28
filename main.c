#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <semaphore.h>

#define BUF_SIZE ( 256 )

// Prototype Functions
void *get_nth_line(void *params);

void *PrintHello(void *threadarg);


// Thread arguments
typedef struct t_params {
    pthread_t tid;
    char *filename;
    size_t id;
} t_params;

sem_t mutex;
int line_counter = 1;
char **read_lines;

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

    sem_init(&mutex, 0, 1);//init binary semaphore

    t_params param[NUM_READ_THREADS];
    pthread_t readers[NUM_READ_THREADS];
    int rc;
    for (int i = 0; i < NUM_READ_THREADS; ++i) {
        param[i].id = i;
        param[i].filename = strdup(argv[2]);
        rc = pthread_create(&readers[i], NULL, get_nth_line, (void *) &param);
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for (int k = 0; k < NUM_READ_THREADS; ++k) {
        pthread_join(readers[k], NULL);
    }
//    t_params param;
//    param.filename = strdup(argv[2]);
//    pthread_t pthread1;
//    pthread_t pthread2;
//    pthread_t pthread3;
//    pthread_t pthread4;
//    pthread_t pthread5;
//    pthread_t pthread6;
//    pthread_create(&pthread1, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread2, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread3, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread4, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread5, NULL, get_nth_line, (void *) (&param));
//    pthread_create(&pthread6, NULL, get_nth_line, (void *) (&param));
//
//    pthread_join(pthread1, NULL);
//    pthread_join(pthread2, NULL);

    sem_destroy(&mutex); /* destroy semaphore */

    return 0;
}


void *get_nth_line(void *params) {
    t_params *t_p = params;
    t_p->tid = pthread_self();
    char *filename = strdup(t_p->filename);
    int line_no, index;
    while (1) {
        sem_wait(&mutex); // down semaphore
        /* START CRITICAL SECTION */
        line_no = line_counter;
        index = line_counter;
        line_counter++;
        /* END CRITICAL SECTION */
        sem_post(&mutex); // up semaphore

        FILE *fp;
        fp = fopen(filename, "r");

        char buf[BUF_SIZE];
        size_t curr_alloc = BUF_SIZE, curr_ofs = 0;
        char *line = malloc(BUF_SIZE);
        int in_line = line_no == 1;
        size_t bytes_read;

        /* Illegal to ask for a line before the first one. */
        if (line_no < 1)
            return NULL;

        /* Handle out-of-memory by returning NULL */
        if (!line)
            return NULL;

        /* Scan the file looking for newlines */
        while (line_no &&
               (bytes_read = fread(buf, 1, BUF_SIZE, fp)) > 0) {
            int i;
            for (i = 0; i < bytes_read; i++) {
                if (in_line) {
                    if (curr_ofs >= curr_alloc) {
                        curr_alloc <<= 1;
                        line = realloc(line, curr_alloc);
                        if (!line)    /* out of memory? */
                            return NULL;
                    }
                    line[curr_ofs++] = buf[i];
                }

                if (buf[i] == '\n') {
                    line_no--;

                    if (line_no == 1)
                        in_line = 1;

                    if (line_no == 0)
                        break;
                }
            }
        }

        /* Didn't find the line? */
        if (line_no != 0) {
            free(line);
            break;
        }

        /* Resize allocated buffer to what's exactly needed by the string
           and the terminating NUL character.  Note that this code *keeps*
           the terminating newline as part of the string.
         */
        line = realloc(line, curr_ofs + 1);

        if (!line) /* out of memory? */
            break;

        /* Add the terminating NUL. */
        line[curr_ofs] = '\0';

        /* Return the line.  Caller is responsible for freeing it. */
//    return line;
        printf("Thread %lu read line:%s", pthread_self(), line);
    }
}

void *PrintHello(void *threadarg) {
    printf("hello world from Thread:,%lu\n", pthread_self());
    pthread_exit(NULL);
}
