/*
* driver.h - Various definitions for the Performance Lab.
*
* DO NOT MODIFY ANYTHING IN THIS FILE
*/
#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdlib.h>

#define RIDX(i,j,n) ((i)*(n)+(j))

typedef struct {
    char *team;
    char *name1, *email1;
    char *name2, *email2;
} team_t;

extern team_t team;

typedef struct {
    unsigned short red;
    unsigned short green;
    unsigned short blue;
} pixel;

/* prototypes for the rotate and smooth functions */
void naive_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time);
void my_rotate(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time);
void naive_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time);
void my_smooth(int dim, pixel *src, pixel *dst, int *rusage_time, unsigned long long *rdtsc_time);

#endif /* _DEFS_H_ */
