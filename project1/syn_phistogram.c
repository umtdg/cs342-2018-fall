#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

#include "helper.h"

#define SEM_NAME "/histsem"

#define SHM_NAME "/histshm"

int
main(int argc, char **argv) {
    if (argc < 6) {
        print_usage("syn_phistogram");
        return 0;
    }

    double min, max;
    size_t bin_count, file_count;
    size_t shm_size;

    sscanf(argv[1], "%lf", &min);
    sscanf(argv[2], "%lf", &max);
    sscanf(argv[3], "%lu", &bin_count);
    sscanf(argv[4], "%lu", &file_count);
    shm_size = sizeof(int) * bin_count;

    if ((size_t)argc < (6U + file_count)) {
        print_usage("syn_phistogram");
        return 0;
    }

    if (create_sem(SEM_NAME) == -1) exit(EXIT_FAILURE);

    if (create_shm(SHM_NAME, shm_size) == -1)
        exit(EXIT_FAILURE);

    pid_t pid;
    for (size_t i = 5; i < file_count + 5; i++) {
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            size_t lines = line_count(argv[i]);
            size_t *hist = hist_from_file(argv[i], lines, min, max, bin_count);
            if (hist == NULL) {
                _exit(EXIT_FAILURE);
            }

            sem_t *sem = open_wait_sem(SEM_NAME);
            if (sem == NULL) _exit(EXIT_FAILURE);

            int fd;
            size_t *shmp = (size_t *)get_shm(SHM_NAME, shm_size, &fd);
            if (shmp == NULL) _exit(EXIT_FAILURE);

            for (size_t i = 0; i < bin_count; i++) {
                shmp[i] += hist[i];
            }

            safe_free(hist, sizeof(size_t) * bin_count);

            if (cleanup_shm(shmp, SHM_NAME, shm_size, fd) == -1)
                _exit(EXIT_FAILURE);

            if (post_close_sem(sem, SEM_NAME) == -1)
                _exit(EXIT_FAILURE);

            _exit(EXIT_SUCCESS);
        }
    }

    int status;
    size_t pid_count = file_count;
    while (pid_count > 0) {
        wait(&status);

        if (status != EXIT_SUCCESS) {
            sem_unlink(SEM_NAME);
            shm_unlink(SHM_NAME);
            exit(EXIT_FAILURE);
        }

        --pid_count;
    }

    sem_unlink(SEM_NAME);

    int fd;
    size_t *shmp = (size_t *)get_shm(SHM_NAME, shm_size, &fd);
    if (shmp == NULL) exit(EXIT_FAILURE);

    save_hist_to_file(shmp, bin_count, argv[5U + file_count], 1);

    cleanup_shm(shmp, SHM_NAME, shm_size, fd);

    shm_unlink(SHM_NAME);

    exit(EXIT_SUCCESS);
}

