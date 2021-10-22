#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SEM_NAME "/histsem\0"

#define SHM_NAME "/histshm\0"

int
main(void) {
    sem_t *histsem;
    int shm_hist_fd;
    int *shmp;

    histsem = sem_open(SEM_NAME, O_CREAT, 0644, 1);
    if (histsem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    shm_hist_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (shm_hist_fd == -1) {
        perror("shm_hist_fd");
        return 1;
    }

    if (ftruncate(shm_hist_fd, sizeof(int)) == -1) {
        perror("ftruncate");
        return 1;
    }

    shmp = (int *)mmap(NULL, sizeof(int),
            PROT_READ | PROT_WRITE,
            MAP_SHARED, shm_hist_fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    *shmp = 0;

    if (munmap(shmp, sizeof(int)) == -1) {
        perror("munmap");
        return 1;
    }

    if (close(shm_hist_fd) == -1) {
        perror("close");
        return 1;
    }

    int proc_count = 5;
    for (int i = 0; i < proc_count; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            sem_t *sem = sem_open(SEM_NAME, O_RDWR);
            if (sem == SEM_FAILED) {
                perror("sem_open");
                _exit(EXIT_FAILURE);
            }

            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                _exit(EXIT_FAILURE);
            }

            int fd = shm_open(SHM_NAME, O_RDWR, 0);
            if (fd == -1) {
                perror("shm_open");
                _exit(EXIT_FAILURE);
            }

            int *shmp = (int *)mmap(NULL, sizeof(int),
                    PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, 0);
            if (shmp == MAP_FAILED) {
                perror("mmap");
                _exit(EXIT_FAILURE);
            }

            *shmp += 5;

            if (munmap(shmp, sizeof(int)) == -1) {
                perror("munmap");
                _exit(EXIT_FAILURE);
            }

            if (close(fd) == -1) {
                perror("close");
                _exit(EXIT_FAILURE);
            }
            
            if (sem_post(sem) == -1) {
                perror("sem_post");
                _exit(EXIT_FAILURE);
            }

            if (sem_close(sem) == -1) {
                perror("sem_close");
                _exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }
    }

    int status;
    while (proc_count > 0) {
        wait(&status);
        --proc_count;
    }

    if (sem_wait(histsem) == -1) {
        perror("sem_wait");
        return 1;
    }

    if (shm_open(SHM_NAME, O_RDWR, 0) == -1) {
        perror("shm_open");
        return 1;
    }

    shmp = (int *)mmap(NULL, sizeof(int),
            PROT_READ | PROT_WRITE,
            MAP_SHARED, shm_hist_fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("Total: %d\n", *shmp);

    if (munmap(shmp, sizeof(int)) == -1) {
        perror("munmap");
        return 1;
    }

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

