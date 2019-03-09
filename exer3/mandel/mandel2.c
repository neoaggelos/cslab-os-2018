/*
 * mandel.c
 *
 * A program to draw the Mandelbrot Set on a 256-color xterm.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#include "mandel-lib.h"

#define MANDEL_MAX_ITERATION 100000

/***************************
 * Compile-time parameters *
 ***************************/

/*
 * Output at the terminal is is x_chars wide by y_chars long
*/
const int y_chars = 50;
const int x_chars = 90;

/*
 * The part of the complex plane to be drawn:
 * upper left corner is (xmin, ymax), lower right corner is (xmax, ymin)
*/
const double xmin = -1.8, xmax = 1.0;
const double ymin = -1.0, ymax = 1.0;

/*
 * Every character in the final output is
 * xstep x ystep units wide on the complex plane.
 */
double xstep;
double ystep;

/*
 * Number of threads, and semaphores used for syncing
 */
int NTHREADS;
sem_t *sems;

/*
 * This function computes a line of output
 * as an array of x_char color values.
 */
void compute_mandel_line(int line, int color_val[])
{
	/*
	 * x and y traverse the complex plane.
	 */
	double x, y;

	int n;
	int val;

	/* Find out the y value corresponding to this line */
	y = ymax - ystep * line;

	/* and iterate for all points on this line */
	for (x = xmin, n = 0; n < x_chars; x+= xstep, n++) {

		/* Compute the point's color value */
		val = mandel_iterations_at_point(x, y, MANDEL_MAX_ITERATION);
		if (val > 255)
			val = 255;

		/* And store it in the color_val[] array */
		val = xterm_color(val);
		color_val[n] = val;
	}
}

/*
 * This function outputs an array of x_char color values
 * to a 256-color xterm.
 */
void output_mandel_line(int fd, int color_val[])
{
	int i;

	char point ='@';
	char newline='\n';

	for (i = 0; i < x_chars; i++) {
		/* Set the current color, then output the point */
		set_xterm_color(fd, color_val[i]);
		if (write(fd, &point, 1) != 1) {
			perror("compute_and_output_mandel_line: write point");
			exit(1);
		}
	}

	/* Now that the line is done, output a newline character */
	if (write(fd, &newline, 1) != 1) {
		perror("compute_and_output_mandel_line: write newline");
		exit(1);
	}
}

void compute_and_output_mandel_line(int fd, int line)
{
	/*
	 * A temporary array, used to hold color values for the line being drawn
	 */
	int color_val[x_chars];

	compute_mandel_line(line, color_val);
	output_mandel_line(fd, color_val);
}

void *thread_func(void *_idx)
{
  int i, next;
	// responsible for lines idx, idx + N, idx + 2N, ...
	// waits sems[idx], signals sems[idx+1 % NTHREADS]
	int idx = (int)_idx;
	int **color_val;
	int sz = ceil((float)y_chars/NTHREADS);
	color_val = malloc(sizeof(int*) * sz);

	for (i = 0, next=idx; i < sz; i++, next+=NTHREADS) {
		color_val[i] = malloc(sizeof(int) * x_chars);
		compute_mandel_line(next, color_val[i]);
	}

	for (i = 0; i < sz; i ++) {
		sem_wait(&sems[idx % NTHREADS]);
		//printf("Got in: %d\n", idx);
		//int color_val[x_chars];
		//compute_mandel_line(i, color_val);

		output_mandel_line(1, color_val[i]);
		//printf("Waking: %d\n", idx+1 %)
		sem_post(&sems[(idx+1) % NTHREADS]);
	}

	return NULL;
}

void cleanup()
{
  reset_xterm_color(1);
}

int main(int argc, char** argv)
{
	pthread_t *threads;
  int i;

	NTHREADS = argc == 1 ? 3 : atoi(argv[1]);

	xstep = (xmax - xmin) / x_chars;
	ystep = (ymax - ymin) / y_chars;

  atexit(cleanup);

	threads = malloc(sizeof(pthread_t) * NTHREADS);
	sems = malloc(sizeof(sem_t) * NTHREADS);
	if (threads == NULL || sems == NULL) {
		perror("malloc");
		exit(-1);
	}

	for (i = 0; i < NTHREADS; i++) {
		int ret = pthread_create(threads + i, NULL, thread_func, (void*)i);
		if (ret < 0) {
			errno = ret;
			perror("pthread_create");
			exit(-1);
		}

		ret = sem_init(sems + i, 0, 0);
		if (ret < 0) {
			perror("sem_init");
			exit(-1);
		}
	}

	/* begin output */
	sem_post(&sems[0]);
	//puts("Unlocked 0");

	/* wait for threads to finish */
	for (i = 0; i < NTHREADS; i++) {
		int ret = pthread_join(threads[i], NULL);
		if (ret < 0) {
			errno = ret;
			perror("pthread_join");
		}
	}

	/* reset and exit */
	reset_xterm_color(1);
	return 0;
}
