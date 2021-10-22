#ifndef PROJECT1_HELPER_H
#define PROJECT1_HELPER_H

#include <stdio.h>
#include <stdlib.h>

#define STR(x) #x

#define ERROR(f, msg) fputs(f ": " msg "\n", stderr)

#define EINVALID_ARGS(f) ERROR(f, "invalid arguments supplied")


/// Read file for floating point numbers
/// \param filename Name of the file to read
/// \param n Maximum number of floating-point numbers to read
/// \param read Number of floating-point numbers read
/// \return A new malloc-ed array containing floating-point numbers read
double *
numbers_from_file(const char *filename, size_t n, size_t *read);

/// Create a histogram
/// \param src Source data
/// \param n  Number of items in src
/// \param min Minimum number
/// \param max Maximum number
/// \param bin_count Number of bins
/// \return A new malloc-ed array containing number of items in a bin
size_t *
hist(const double *src, size_t n,
        double min, double max, size_t bin_count);

/// Get number of lines in a file
/// \param filename Name of the file
/// \return Number of lines in the file
size_t
line_count(const char *filename);

/// Safely free a heap block
/// \param block Block to free
/// \param size Size of the block
void
safe_free(void *block, size_t size);

/// Create histogram using data in file
/// \param filename Name of the file to read
/// \param n Maximum number of numbers to read
/// \param min Minimum value for histogram
/// \param max Maximum value for histogram
/// \param bin_count Number of bins
/// \return A new malloc-ed array containing histogram data
size_t *
hist_from_file(const char *filename, size_t n,
        double min, double max, size_t bin_count);

/// Write histogram to file overriding existing file
/// \param h Histogram to write
/// \param n Length of the histogram
/// \param filename Name of the file to write
/// \param write_bin_numbers Indicator to whether to write bin numbers or not
int
save_hist_to_file(const size_t *h, size_t n,
        const char *filename, int write_bin_numbers);

/// Generate histogram using numbers in file ifname and
/// write histogram to file ofname without bin numbers
/// \param ifname Name of the file to read numbers from
/// \param n Maximum number of numbers to read
/// \param min Minimum value for histogram
/// \param max Maximum value for histogram
/// \param bin_count Number of bins
/// \param ofname Name of the file to write histogram data
int
hist_from_file_to_file(const char *ifname, size_t n,
        double min, double max, size_t bin_count,
        const char *ofname);

/// Merge multiple histogram files, <filename_prefix>N.txt, into dest
/// \param dest Destination histogram, where histograms in files will be merged
/// \param bin_count Number of bins
/// \param filename_prefix Prefix for name of the files containing histogram data
/// \param hist_count Number of files containing histogram data
int
merge_hist_files(size_t dest[], size_t bin_count, 
        const char *filename_prefix, size_t hist_count);

#endif //PROJECT1_HELPER_H
