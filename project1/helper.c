#include "helper.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <unistd.h>

double *
numbers_from_file(const char *filename, size_t n, size_t *read) {
    if (!filename || !read || n == 0) {
        EINVALID_ARGS("numbers_from_file");
        return NULL;
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    size_t size = sizeof(double) * n;

    double *result = (double *)malloc(size);
    if (!result) {
        perror("malloc");
        fclose(f);
        exit(1);
    }

    double x = 0.0;
    *read = 0;
    while (!feof(f) && *read < n) {
        if (fscanf(f, "%lf", &x) != 1) {
            perror("fscanf");
            safe_free(result, size);
            fclose(f);
            return NULL;
        }
        result[*read] = x;
        (*read)++;
    }

    fclose(f);
    return result;
}

void
safe_free(void *block, size_t size) {
    if (!block) return;

    memset(block, 0, size);
    free(block);
    block = NULL;
}

size_t
line_count(const char *filename) {
    if (!filename) {
        EINVALID_ARGS("line_count");
    }

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 0;
    }

    char c;
    size_t result = 0;
    while (!feof(f)) {
        c = fgetc(f);
        if (c == '\n') result++;
    }

    fclose(f);

    return result;
}

size_t *
hist(const double *src, size_t n,
        double min, double max, size_t bin_count) {
    if (!src || bin_count == 0) {
        EINVALID_ARGS("hist");
        return NULL;
    }

    if (min == max) bin_count = 1;
    if (min > max) {
        max = max + min;
        min = max - min;
        max = max - min;
    }

    size_t hist_size = sizeof(size_t) * bin_count;
    size_t *result = (size_t *)malloc(hist_size);
    memset(result, 0, hist_size);

    double bin_width = (max - min) / (double)bin_count;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < bin_count; j++) {
            double lower = min + bin_width * (double)j;
            if (lower > src[i]) continue;

            double upper = min + bin_width * (double)(j + 1);
            if (src[i] > upper) continue;
            if (src[i] == upper && j != bin_count - 1) continue;

            result[j]++;
        }
    }

    return result;
}

size_t *
hist_from_file(const char *filename, size_t n,
        double min, double max, size_t bin_count) {
    size_t number_count = 0;
    double *numbers = numbers_from_file(filename, n, &number_count);
    if (!numbers) return NULL;

    size_t *h = hist(numbers, number_count, min, max, bin_count);
    safe_free(numbers, sizeof(double) * number_count);

    return h;
}

int
save_hist_to_file(const size_t *h, size_t n,
        const char *filename, int write_bin_numbers) {
    if (!h || !filename) return 1;

    FILE *fhist = fopen(filename, "w");
    if (!fhist) {
        perror("fopen");
        return 1;
    }

    for (size_t i = 0; i < n; i++) {
        if (write_bin_numbers)
            fprintf(fhist, "%lu: %lu\n", i + 1, h[i]);
        else
            fprintf(fhist, "%lu\n", h[i]);
    }

    fclose(fhist);

    return 0;
}

int
hist_from_file_to_file(const char *ifname, size_t n,
        double min, double max, size_t bin_count,
        const char *ofname) {
    int result = 0;

    size_t *h = hist_from_file(ifname, n, min, max, bin_count);
    if (!h) return 1;

    result = save_hist_to_file(h, bin_count, ofname, 0);
    safe_free(h, sizeof(size_t) * bin_count);

    return result;
}

int
merge_hist_files(size_t dest[], size_t bin_count,
        const char *filename_prefix, size_t hist_count) {
    if (!dest) return 1;
    if (!filename_prefix) return 1;

    char fname[256];
    double *h = NULL;
    size_t hist_length = 0;
    size_t lines;

    for (size_t i = 0; i < hist_count; i++) {
        snprintf(fname, 256, "%s%lu.txt", filename_prefix, i + 1);
        lines = line_count(fname);
        h = numbers_from_file(fname, lines, &hist_length);
        if (!h) goto next;

        if (hist_length > bin_count) {
            ERROR("merge_hist_files", "number of bins in histogram file"
                                        "exceeds given bin count");
            goto next;
        }

        for (size_t j = 0; j < bin_count; j++) {
            if (j < hist_length)
                dest[j] += (size_t)h[j];
            else
                dest[j] += 0;
        }

next:
        safe_free(h, sizeof(double) * hist_length);
    }

    return 0;
}

int
create_shm(const char *shm_name, size_t shm_size) {
    int fd;
    void *shmp;

    if (!shm_name || shm_size == 0) return -1;

    // Open or create shared memory
    fd = shm_open(shm_name, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("shm_open");
        return -1;
    }

    // Allocate memory
    if (ftruncate(fd, shm_size) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(shm_name);
        return -1;
    }

    // Fill allocated memory with 0's
    shmp = mmap(NULL, shm_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(shm_name);
        return -1;
    }

    memset(shmp, 0, shm_size);

    // Cleanup
    if (munmap(shmp, shm_size) == -1) {
        perror("munmap");
        close(fd);
        shm_unlink(shm_name);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close");
        shm_unlink(shm_name);
        return -1;
    }

    return 0;
}

void *
get_shm(const char *shm_name, size_t shm_size, int *fd) {
    if (!shm_name || shm_size == 0 || !fd) return NULL;

    if ((*fd = shm_open(shm_name, O_RDWR, 0)) == -1) {
        perror("shm_open");
        shm_unlink(shm_name);
        return NULL;
    }

    void *shmp = mmap(NULL, shm_size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED, *fd, 0);
    if (shmp == MAP_FAILED) {
        perror("mmap");
        close(*fd);
        shm_unlink(shm_name);
        return NULL;
    }

    return shmp;
}

int
cleanup_shm(void *shmp, const char *shm_name, size_t shm_size, int fd) {
    if (!shmp || shm_size == 0 || fd < 0) return -1;

    if (munmap(shmp, shm_size) == -1) {
        perror("munmap");
        close(fd);
        shm_unlink(shm_name);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close");
        shm_unlink(shm_name);
        return -1;
    }

    return 0;
}

int
create_sem(const char *sem_name) {
    if (!sem_name) return -1;

    sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        return -1;
    }

    if (sem_close(sem) == -1) {
        perror("sem_close");
        sem_unlink(sem_name);
        return -1;
    }

    return 0;
}

sem_t *
open_wait_sem(const char *sem_name) {
    if (!sem_name) return NULL;
    
    sem_t *sem = sem_open(sem_name, O_RDWR);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(sem_name);
        return NULL;
    }

    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        sem_close(sem);
        sem_unlink(sem_name);
        return NULL;
    }

    return sem;
}

int
post_close_sem(sem_t *sem, const char *sem_name) {
    if (!sem || !sem_name) return -1;

    if (sem_post(sem) == -1) {
        perror("sem_post");
        sem_close(sem);
        sem_unlink(sem_name);
        return -1;
    }

    if (sem_close(sem) == -1) {
        perror("sem_close");
        sem_unlink(sem_name);
        return -1;
    }

    return 0;
}

void
print_usage(const char *prog) {
    if (!prog) return;
    printf("Usage:\n");
    printf("\t%s [MINVAL] [MAXVAL] [BINCOUNT]"
           " [FILECOUNT] [IFILE]... [OFILE]", prog);
}
