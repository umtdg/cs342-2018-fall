#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helper.h"


int
main(int argc, char **argv) {
    if (argc < 6) {
        print_usage("phistogram");
        return 0;
    }

    double min = 0.0, max = 0.0;
    size_t bin_count = 0;
    size_t file_count = 0;

    sscanf(argv[1], "%lf", &min);
    sscanf(argv[2], "%lf", &max);
    sscanf(argv[3], "%lu", &bin_count);
    sscanf(argv[4], "%lu", &file_count);

    if ((size_t)argc < (6U + file_count)) {
        print_usage("phistogram");
        return 0;
    }

    pid_t pid;
    for (size_t i = 5; i < file_count + 5; i++) {
        size_t relative_index = i - 5;
        
        // Create a child process
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {
            // Create histogram from file
            char ofname[256];
            snprintf(ofname, 256, "hist%lu.txt", relative_index + 1);

            size_t lines = line_count(argv[i]);
            if (hist_from_file_to_file(argv[i], lines,
                        min, max, bin_count, ofname) != 0) {
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // Wait for all childs to finish
    size_t pid_count = file_count;
    int status;
    while (pid_count > 0) {
        wait(&status);
        --pid_count;
    }
    
    // Merge histograms from intermediate files
    size_t result_hist[bin_count];
    memset(result_hist, 0, sizeof(size_t) * bin_count);

    merge_hist_files(result_hist, bin_count, "hist", file_count);

    save_hist_to_file(result_hist, bin_count, argv[5U + file_count], 1);
    
    return 0;
}

