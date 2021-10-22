#include <sys/time.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

/* TYPEDEFS */
typedef long long llong;

typedef double (*probability_function)(double, size_t);
/* TYPEDEFS */

/* PREPROCESSOR DEFINITIONS */
#define _gettimeofday(tv) do { if (gettimeofday(&(tv), NULL) == -1) { fputs("failed to get time of day\n", stderr); return 1; } } while(0)

#define timediff(s, e) (llong)((e).tv_usec - (s).tv_usec)

#define return_on_error(result, msg, ret) do { if ((result) == -1) { fputs((msg), stderr); return (ret); } } while(0)

#define measure(f, f_current, f_times, index) do { \
    f_current = f(); \
    if (result == -1) return 1; \
    if (f_current < 0) f_current += 1000000; \
    (f_times)[(index)] = f_current; \
} while(0)

#define NSAMPLES 10000

#define DNSAMPLES (double)NSAMPLES

#define SREAD 1024
/* PREPROCESSOR DEFINITIONS */

/* GLOBALS */
static struct timeval start;
static struct timeval end;
static int fd = -1;
static ssize_t result = -1;
static char buf[SREAD];
/* GLOBALS */

/* FUNCTION DECLS */
llong
measure_open(void);

llong
measure_read(void);

llong
measure_close(void);

double
mean(double *arr, size_t length, probability_function p);

double
variance(double *arr, size_t length, double m, probability_function p);

double
standard_deviation(double *arr, size_t length, double var, probability_function p);

double
sum(double *arr, size_t length);
/* FUNCTION DECLS */

/* FUNCTION DEFS */
llong
measure_open(void) {
    _gettimeofday(start);

    result = open("/dev/random", O_RDONLY);

    _gettimeofday(end);

    return_on_error(result, "failed to open /dev/random\n", -1);

    return timediff(start, end);
}

llong
measure_read(void) {
    _gettimeofday(start);

    result = read(fd, buf, SREAD);

    _gettimeofday(end);

    return_on_error(result, "failed to read /dev/random\n", -1);

    return timediff(start, end);
}

llong
measure_close(void) {
    _gettimeofday(start);

    result = close(fd);

    _gettimeofday(end);

    return_on_error(result, "failed to close /dev/random\n", -1);

    return timediff(start, end);
}

double
equivalance_probability(double x, size_t length) {
    (void) x;
    return 1.0 / length;
}

double
mean(double *arr, size_t length, probability_function p) {
    if (!arr) return 0;

    double result = 0;

    for (size_t i = 0; i < length; i++)
        result += arr[i] * p(arr[i], length);

    return result;
}

double
variance(double *arr, size_t length, double m, probability_function p) {
    if (!arr) return 0;

    double x[length];
    for (size_t i = 0; i < length; i++)
        x[i] = (arr[i] - m) * (arr[i] - m);

    return mean(x, length, p);
}

double
standard_deviation(double *arr, size_t length, double var, probability_function p) {
    if (!arr) return 0;

    if (var < 0)
        var = variance(arr, length, mean(arr, length, p), p);

    return sqrt(var);
}

double
sum(double *arr, size_t length) {
    if (!arr) return 0;

    double result = 0;
    for (size_t i = 0; i < length; i++) {
        result += arr[i];
    }

    return result;
}

void
print_mean_var_and_sd(const char *name, double time_array[NSAMPLES]) {
    if (!name) return;

    double m = mean(time_array, NSAMPLES, equivalance_probability);
    double var = variance(time_array, NSAMPLES, m, equivalance_probability);
    double sd = standard_deviation(time_array, NSAMPLES, var, equivalance_probability);

    const char *fmt = "%s\n\tmean: %.2f\n\tvariance: %.2f\n\tstandard deviation: %.2f\n";
    printf(fmt, name, m, var, sd);
}
/* FUNCTION DEFS */

int
main() {
    printf("Open, read %d bytes and close /dev/random %d times and run time of each\n\n",
            SREAD, NSAMPLES);

    llong time_taken;
    double open_times[NSAMPLES];
    double close_times[NSAMPLES];
    double read_times[NSAMPLES];
    
    for (int i = 0; i < NSAMPLES; i++) {
        measure(measure_open, time_taken, open_times, i);
        fd = result;

        measure(measure_read, time_taken, read_times, i);
        measure(measure_close, time_taken, close_times, i);
    }
    
    print_mean_var_and_sd("open\0", open_times);
    print_mean_var_and_sd("read\0", read_times);
    print_mean_var_and_sd("close\0", close_times);

    return 0;
}

