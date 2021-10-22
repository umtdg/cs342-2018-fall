#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"

#define SEM_NAME "/histsem\0"

#define SHM_NAME "/histshm\0"

int
main(void) {
    if (create_sem(SEM_NAME) == -1) exit(EXIT_FAILURE);

    if (create_shm(SHM_NAME, sizeof(int)) == -1) {
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    int proc_count = 5;
    for (int i = 0; i < proc_count; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            sem_t *sem = open_wait_sem(SEM_NAME);
            if (sem == NULL) _exit(EXIT_FAILURE);

            int fd;
            int *shmp = (int *)get_shm(SHM_NAME, sizeof(int), &fd);
            if (shmp == NULL) _exit(EXIT_FAILURE);

            *shmp += 5;

            if (cleanup_shm(shmp, SHM_NAME, sizeof(int), fd) == -1)
                _exit(EXIT_FAILURE);

            if (post_close_sem(sem, SEM_NAME) == -1)
                _exit(EXIT_FAILURE);
           
            exit(EXIT_SUCCESS);
        }
    }

    int status;
    while (proc_count > 0) {
        wait(&status);
        if (status != EXIT_SUCCESS) {
           sem_unlink(SEM_NAME);
           shm_unlink(SHM_NAME);
           exit(EXIT_FAILURE);
        }
        --proc_count;
    }

    sem_t *histsem;
    if ((histsem = open_wait_sem(SEM_NAME)) == NULL) {
        shm_unlink(SHM_NAME);
        exit(EXIT_FAILURE);
    }

    int shm_hist_fd;
    int *shmp = (int *)get_shm(SHM_NAME, sizeof(int), &shm_hist_fd);
    if (shmp == NULL) {
        post_close_sem(histsem, SEM_NAME);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    printf("Total: %d\n", *shmp);

    if (cleanup_shm(shmp, SHM_NAME, sizeof(int), shm_hist_fd) == -1) {
        post_close_sem(histsem, SEM_NAME);
        sem_unlink(SEM_NAME);
        exit(EXIT_FAILURE);
    }

    post_close_sem(histsem, SEM_NAME);

    if (sem_unlink(SEM_NAME) == -1) {
        perror("sem_unlink");
        return 1;
    }

    if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlink");
        return 1;
    }

    return 0;
}

