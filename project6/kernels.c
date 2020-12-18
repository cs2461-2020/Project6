/********************************************************
 * Kernels to be optimized for the  Performance Project
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include <sys/time.h>
#include <sys/resource.h>

/* Below are statements to set up the performance measurement utilities */
/* we use rdtsc, clock, and getusage utilities to measure performance */

//#define rdtscll(val) __asm__ __volatile__("rdtsc" : "=A" (val))
#if defined(__i386__)

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}
#elif defined(__x86_64__)


static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#endif

/* end of definitions to set up measurement utilities */


/* 
 * Please fill in the following team struct 
 */
team_t team = {
    "joe",              /* Team name */

    "Harry Q. Bovik",     /* First member full name */
    "bovik@nowhere.edu",  /* First member email address */

    "",                   /* Second member full name (leave blank if none) */
    ""                    /* Second member email addr (leave blank if none) */
};

/***************
 * ROTATE KERNEL
 ***************/

/******************************************************
 * Your different versions of the rotate kernel go here
 ******************************************************/

/* the getUserTime function is used for measurement, you should not change the code for this function */

long int getUserTime()
{
	int who= RUSAGE_SELF;
	int ret;
	struct rusage usage;
	struct rusage *p=&usage;
	//long int current_time;

	ret=getrusage(who,p);
	if(ret == -1)
	{
		printf("Could not get GETRUSAGE to work in function %s at line %d in file %s\n",
				__PRETTY_FUNCTION__, __LINE__, __FILE__);
		exit(1);
	}
	return (p->ru_utime.tv_sec * 1000000 + p->ru_utime.tv_usec);
}

/* 
 * naive_rotate - The naive baseline version of rotate 
 */
 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char naive_rotate_descr[] = "naive_rotate: Naive baseline implementation";
void naive_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
	unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
	
	rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* below is the main computations for the rotate function */

	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];

/* the remaining lines in this function stop the measurement and set the values before returning. */

	rusage_end_time = getUserTime();
	rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}

 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
char my_rotate_descr[] = "my_rotate: Naive baseline implementation";
void my_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
		/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* ANY CHANGES ARE MADE HERE */
/* below are the main computations for your implementation of the rotate. Any changes in implementation will go here or the other functions it may call */
	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(dim-1-j, i, dim)] = src[RIDX(i, j, dim)];


/* end of computation for rotate function. any changes you make should be made above this line. */
/* END OF CHANGES in this function */

/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}





/***************
 * SMOOTH KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the smooth function
 * You may modify these any way you like.
 **************************************************************/

/* A struct used to compute averaged pixel value */
typedef struct {
	int red;
	int green;
	int blue;
	int num;
} pixel_sum;

/* Compute min and max of two integers, respectively */
static int minimum(int a, int b) 
{ return (a < b ? a : b); }
static int maximum(int a, int b) 
{ return (a > b ? a : b); }

/* 
 * initialize_pixel_sum - Initializes all fields of sum to 0 
 */
static void initialize_pixel_sum(pixel_sum *sum) 
{
	sum->red = sum->green = sum->blue = 0;
	sum->num = 0;
	return;
}

/* 
 * accumulate_sum - Accumulates field values of p in corresponding 
 * fields of sum 
 */
static void accumulate_sum(pixel_sum *sum, pixel p) 
{
	sum->red += (int) p.red;
	sum->green += (int) p.green;
	sum->blue += (int) p.blue;
	sum->num++;
	return;
}

/* 
 * assign_sum_to_pixel - Computes averaged pixel value in current_pixel 
 */
static void assign_sum_to_pixel(pixel *current_pixel, pixel_sum sum) 
{
	current_pixel->red = (unsigned short) (sum.red/sum.num);
	current_pixel->green = (unsigned short) (sum.green/sum.num);
	current_pixel->blue = (unsigned short) (sum.blue/sum.num);
	return;
}

/* 
 * avg - Returns averaged pixel value at (i,j) 
 */
static pixel avg(int dim, int i, int j, pixel *src) 
{
	int ii, jj;
	pixel_sum sum;
	pixel current_pixel;

	initialize_pixel_sum(&sum);
	for(ii = maximum(i-1, 0); ii <= minimum(i+1, dim-1); ii++) 
		for(jj = maximum(j-1, 0); jj <= minimum(j+1, dim-1); jj++) 
			accumulate_sum(&sum, src[RIDX(ii, jj, dim)]);

	assign_sum_to_pixel(&current_pixel, sum);
	return current_pixel;
}

/******************************************************
 * Your different versions of the smooth kernel go here
 ******************************************************/

/*
 * naive_smooth - The naive baseline version of smooth 
 */
  /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char naive_smooth_descr[] = "naive_smooth: Naive baseline implementation";
void naive_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;

	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();

/* below are the main computations for the smooth function */
	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(i, j, dim)] = avg(dim, i, j, src);

/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}

 /* The parameters, pointers, rusage_time, rdtsc_time, and cpu_time_used are used to measure performance and return values to caller. */
 /* You should not change the code that uses these parameters and variables. */
 
char my_smooth_descr[] = "my_smooth: Naive baseline implementation";
void my_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time) 
{
	int i, j;
	
	/* the variables below are used for performance measurement and not for computing the results of the algorithm */
	long int rusage_start_time, rusage_end_time = 0;
        unsigned long long rdtsc_start_time, rdtsc_end_time = 0;
	/* call system functions to start measuring performance. you should not bother to change these. */
        rusage_start_time = getUserTime();
	rdtsc_start_time = rdtsc();
	
/* ANY CHANGES TO BE MADE SHOULD BE BELOW HERE */
/* below are the main computations for your implementation of the smooth function. Any changes in implementation will go here or the other functiosn it calls */

	for (j = 0; j < dim; j++)
		for (i = 0; i < dim; i++)
			dst[RIDX(i, j, dim)] = avg(dim, i, j, src);

/* end of computation for smooth function. so don't change anything after this in this function. */
/* END OF CHANGES */

/* the remaining lines in this function stop the measurement and set the values before returning. */
	rusage_end_time = getUserTime();
        rdtsc_end_time = rdtsc();

	*rusage_time = rusage_end_time - rusage_start_time;
	*rdtsc_time = rdtsc_end_time - rdtsc_start_time;
}
