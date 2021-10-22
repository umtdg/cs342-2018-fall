#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "helper.h"


static double min, max;
static size_t bin_count;


struct thread_info {
    pthread_t   thread_id;
    size_t      thread_num;
    const char  *filename;
};


void *
thread_function(void *arg) {
    struct thread_info *tinfo = arg;

    char ofname[256];
    snprintf(ofname, 256, "hist%lu.txt", tinfo->thread_num);

    size_t lines = line_count(tinfo->filename);
    hist_from_file_to_file(tinfo->filename, lines,
            min, max, bin_count, ofname);

    return NULL;
}


void
print_usage() {
    puts("Usage: ");
    puts("\tthistogram [MINVALUE] [MAXVALUE] [BINCOUNT] "
            "[FILECOUNT] [FILE]... [OUTFILE]");
}

int
main(int argc, char **argv) {
    if (argc < 6) {
        print_usage();
        return 0;
    }

    size_t file_count = 0;

    sscanf(argv[1], "%lf", &min);
    sscanf(argv[2], "%lf", &max);
    sscanf(argv[3], "%lu", &bin_count);
    sscanf(argv[4], "%lu", &file_count);

    if ((size_t)argc < (6U + file_count)) {
        print_usage();
        return 0;
    }

    struct thread_info *tinfo = calloc(file_count, sizeof(*tinfo));
    if (tinfo == NULL) {
        perror("calloc");
        return 1;
    }

    for (size_t i = 0; i < file_count; i++) {
        tinfo[i].thread_num = i + 1;
        tinfo[i].filename = argv[i + 5];

        if (pthread_create(&tinfo[i].thread_id, NULL,
                    &thread_function, &tinfo[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (size_t i = 0; i < file_count; i++) {
        if (pthread_join(tinfo[i].thread_id, NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    safe_free(tinfo, sizeof(*tinfo));

    size_t result_hist[bin_count];
    memset(result_hist, 0, sizeof(size_t) * bin_count);

    merge_hist_files(result_hist, bin_count, "hist", file_count);

    save_hist_to_file(result_hist, bin_count, argv[5U + file_count], 1);

    return 0;
}

