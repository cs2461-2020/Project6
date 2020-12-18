/*******************************************************************
 * 
 * driver.c - Driver program for Project 5
 * 
 * In kernels.c, students will generate an arbitrary number of rotate and
 * smooth test functions, which they then register with the driver
 * program using the add_rotate_function() and add_smooth_function()
 * functions.
 * For Project 5 you will generate my_rotate and my_smooth functions.
 * The driver program runs and measures the registered test functions
 * and reports their performance.
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 ********************************************************************/

#include <stdio.h>
#include "defs.h"

/* Team structure that identifies the students */
extern team_t team; 

#define DIM_CNT 4

/* Misc constants */
#define BSIZE 32     /* cache block size in bytes */     
#define MAX_DIM 4352 /* 4096 + 256 */
#define ODD_DIM 96   /* not a power of 2 */

/* fast versions of min and max */
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)


/* The range of image dimensions that we will be testing */
static int test_dim_rotate[] = {512, 1024, 2048,4096};
static int test_dim_smooth[] = {256, 512, 1024, 2048};



/* 
 * An image is a dimxdim matrix of pixels stored in a 1D array.  The
 * data array holds three images (the input original, a copy of the original, 
 * and the output result array. There is also an additional BSIZE bytes
 * of padding for alignment to cache block boundaries.
 */
static pixel data[(3*MAX_DIM*MAX_DIM) + (BSIZE/sizeof(pixel))];

/* Various image pointers */
static pixel *orig = NULL;         /* original image */
static pixel *copy_of_orig = NULL; /* copy of original for checking result */
static pixel *result = NULL;       /* result image */



/* 
 * random_in_interval - Returns random integer in interval [low, high) 
 */
static int random_in_interval(int low, int high) 
{
  int size = high - low;
  return (rand()% size) + low;
}

/*
 * create - creates a dimxdim image aligned to a BSIZE byte boundary
 */
static void create(int dim)
{
  int i, j;

  /* Align the images to BSIZE byte boundaries */
  orig = data;
  while ((unsigned)orig % BSIZE){
    //((char *)orig)++;
    orig += sizeof(char);
  }
  result = orig + dim*dim;
  copy_of_orig = result + dim*dim;

  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      /* Original image initialized to random colors */
      orig[RIDX(i,j,dim)].red = random_in_interval(0, 65536);
      orig[RIDX(i,j,dim)].green = random_in_interval(0, 65536);
      orig[RIDX(i,j,dim)].blue = random_in_interval(0, 65536);

      /* Copy of original image for checking result */
      copy_of_orig[RIDX(i,j,dim)].red = orig[RIDX(i,j,dim)].red;
      copy_of_orig[RIDX(i,j,dim)].green = orig[RIDX(i,j,dim)].green;
      copy_of_orig[RIDX(i,j,dim)].blue = orig[RIDX(i,j,dim)].blue;

      /* Result image initialized to all black */
      result[RIDX(i,j,dim)].red = 0;
      result[RIDX(i,j,dim)].green = 0;
      result[RIDX(i,j,dim)].blue = 0;
    }
  }

  return;
}


/* 
 * compare_pixels - Returns 1 if the two arguments don't have same RGB
 *    values, 0 o.w.  
 */
static int compare_pixels(pixel p1, pixel p2) 
{
  return 
    (p1.red != p2.red) || 
    (p1.green != p2.green) || 
    (p1.blue != p2.blue);
}


/* Make sure the orig array is unchanged */
static int check_orig(int dim) 
{
  int i, j;

  for (i = 0; i < dim; i++) 
    for (j = 0; j < dim; j++) 
      if (compare_pixels(orig[RIDX(i,j,dim)], copy_of_orig[RIDX(i,j,dim)])) {
	printf("\n");
	printf("Error: Original image has been changed!\n");
	return 1;
      }

  return 0;
}

/* 
 * check_rotate - Make sure the rotate actually works. 
 * The orig array should not  have been tampered with! 
 */
static int check_rotate(int dim) 
{
  int err = 0;
  int i, j;
  int badi = 0;
  int badj = 0;
  pixel orig_bad, res_bad;

  /* return 1 if the original image has been  changed */
  if (check_orig(dim)) 
    return 1; 

  for (i = 0; i < dim; i++) 
    for (j = 0; j < dim; j++) 
      if (compare_pixels(orig[RIDX(i,j,dim)], 
			 result[RIDX(dim-1-j,i,dim)])) {
	err++;
	badi = i;
	badj = j;
	orig_bad = orig[RIDX(i,j,dim)];
	res_bad = result[RIDX(dim-1-j,i,dim)];
      }

  if (err) {
    printf("\n");
    printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
    printf("E.g., The following two pixels should have equal value:\n");
    printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	   badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue);
    printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	   (dim-1-badj), badi, res_bad.red, res_bad.green, res_bad.blue);
  }

  return err;
}

static pixel check_average(int dim, int i, int j, pixel *src) {
  pixel result;
  int num = 0;
  int ii, jj;
  int sum0, sum1, sum2;
  int top_left_i, top_left_j;
  int bottom_right_i, bottom_right_j;

  top_left_i = max(i-1, 0);
  top_left_j = max(j-1, 0);
  bottom_right_i = min(i+1, dim-1); 
  bottom_right_j = min(j+1, dim-1);

  sum0 = sum1 = sum2 = 0;
  for(ii=top_left_i; ii <= bottom_right_i; ii++) {
    for(jj=top_left_j; jj <= bottom_right_j; jj++) {
      num++;
      sum0 += (int) src[RIDX(ii,jj,dim)].red;
      sum1 += (int) src[RIDX(ii,jj,dim)].green;
      sum2 += (int) src[RIDX(ii,jj,dim)].blue;
    }
  }
  result.red = (unsigned short) (sum0/num);
  result.green = (unsigned short) (sum1/num);
  result.blue = (unsigned short) (sum2/num);
 
  return result;
}


/* 
 * check_smooth - Make sure the smooth function actually works.  The
 * orig array should not have been tampered with!  
 */
static int check_smooth(int dim) {
  int err = 0;
  int i, j;
  int badi = 0;
  int badj = 0;
  pixel right, wrong;

  /* return 1 if original image has been changed */
  if (check_orig(dim)) 
    return 1; 

  for (i = 0; i < dim; i++) {
    for (j = 0; j < dim; j++) {
      pixel smoothed = check_average(dim, i, j, orig);
      if (compare_pixels(result[RIDX(i,j,dim)], smoothed)) {
	err++;
	badi = i;
	badj = j;
	wrong = result[RIDX(i,j,dim)];
	right = smoothed;
      }
    }
  }

  if (err) {
    printf("\n");
    printf("ERROR: Dimension=%d, %d errors\n", dim, err);    
    printf("E.g., \n");
    printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	   badi, badj, wrong.red, wrong.green, wrong.blue);
    printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n",
	   badi, badj, right.red, right.green, right.blue);
  }

  return err;
}




void run_rotate_benchmark(int dim) 
{
  int temp_int;
  unsigned long long temp_ull;
  my_rotate(dim, orig, result, &temp_int, &temp_ull);
}

void test_rotate() 
{
    
  int test_num;
  int i=0;
  int naive_time_rusage=0;
  int naive_ave_time_rusage=0;
  unsigned long long naive_time_rdtsc = 0;
  unsigned long long naive_ave_time_rdtsc = 0;
  int my_time_rusage=0; 
  int my_ave_time_rusage=0;
  unsigned long long my_time_rdtsc = 0;
  unsigned long long my_ave_time_rdtsc = 0;
	
  char dim_string[] = "Dimension";
  char naive_string[] = "naive_rotate";
  char my_string[] = "my_rotate";
  char rusage_header[] = "Time in milliseconds";
  char rdtsc_header[] = "Cycles used";

  printf("\nTesting Rotate:\n");
  printf("%-10s%-26s%-13s\n","",rusage_header,rdtsc_header);
  printf("==========================================================\n");
  printf("%-10s%-13s%-13s%-13s%-13s\n",dim_string,naive_string,my_string,naive_string,my_string);
  printf("==========================================================\n");
  for (test_num = 0; test_num < DIM_CNT; test_num++) 
    {

      int dim;
	
      /* Create a test image of the required dimension */
      dim = test_dim_rotate[test_num];
      create(dim);


      /* Check that the code works */
      run_rotate_benchmark(dim);
      if (check_rotate(dim)) 
	{
	  printf("Benchmark \"rotate()\" failed correctness check for dimension %d.\n", dim);
	  return;
	}	
      create(dim);
/* The code below tests performance of the rotate algorithms. It measures performance using three different system utilities and prints these out in the output */	
/* First measure performance of the naive rotate algorithm */

      for (i=0;i<10;i++){
	naive_rotate(dim,orig,result,&naive_time_rusage,&naive_time_rdtsc);

	naive_ave_time_rusage += naive_time_rusage;
	naive_ave_time_rdtsc += naive_time_rdtsc;
      }
      naive_ave_time_rusage = naive_ave_time_rusage / 10;
      naive_ave_time_rdtsc = naive_ave_time_rdtsc / 10;

/* measure performance of your implementation */

      for (i=0;i<10;i++){
	my_rotate(dim,orig,result,&my_time_rusage,&my_time_rdtsc);
	my_ave_time_rusage += my_time_rusage;
	my_ave_time_rdtsc += my_time_rdtsc;
      }
      my_ave_time_rusage = my_ave_time_rusage / 10;
      my_ave_time_rdtsc = my_ave_time_rdtsc / 10;
      printf("%-10d%-13d%-13d%-13llu%-13llu\n",dim,naive_ave_time_rusage,my_ave_time_rusage, naive_ave_time_rdtsc, my_ave_time_rdtsc);
    }
  return;  
}

void run_smooth_benchmark(int dim) 
{
  int temp_int;
  unsigned long long temp_ull;
  my_smooth(dim, orig, result, &temp_int, &temp_ull);
}

void test_smooth() 
{
    
  int test_num;
  int i=0;

  int naive_time_rusage=0;
  int naive_ave_time_rusage=0;
  unsigned long long naive_time_rdtsc = 0;
  unsigned long long naive_ave_time_rdtsc = 0;
  int my_time_rusage=0; 
  int my_ave_time_rusage=0;
  unsigned long long my_time_rdtsc = 0;
  unsigned long long my_ave_time_rdtsc = 0;
	
  char dim_string[] = "Dimension";
  char naive_string[] = "naive_smooth";
  char my_string[] = "my_smooth";
  char rusage_header[] = "Time in milliseconds";
  char rdtsc_header[] = "Cycles used";

  printf("\nTesting Smooth:\n");
  printf("%-10s%-26s%-13s\n","",rusage_header,rdtsc_header);
  printf("==========================================================\n");
  printf("%-10s%-13s%-13s%-13s%-13s\n",dim_string,naive_string,my_string,naive_string,my_string);
  printf("==========================================================\n");

  for(test_num=0; test_num < DIM_CNT; test_num++) 
    {
      int dim;

      /* Create a test image of the required dimension */
      dim = test_dim_smooth[test_num];
      create(dim);

      /* Check that the code works */
      run_smooth_benchmark(dim);
      if (check_smooth(dim)) {
	printf("Benchmark \"smooth()\" failed correctness check for dimension %d.\n", dim);
	return;
      }
        
      create(dim);
/* The code below tests performance of the rotate algorithms. It measures performance using three different system utilities and prints these out in the output */	
/* First measure performance of the naive smooth algorithm */

      for (i=0;i<10;i++){
	naive_smooth(dim,orig,result,&naive_time_rusage,&naive_time_rdtsc);

	naive_ave_time_rusage += naive_time_rusage;
	naive_ave_time_rdtsc += naive_time_rdtsc;
      }
      naive_ave_time_rusage = naive_ave_time_rusage / 10;
      naive_ave_time_rdtsc = naive_ave_time_rdtsc / 10;

/* measure performance of your implementtion for the smooth algorithm */

      for (i=0;i<10;i++){
	my_smooth(dim,orig,result,&my_time_rusage,&my_time_rdtsc);
	my_ave_time_rusage += my_time_rusage;
	my_ave_time_rdtsc += my_time_rdtsc;
      }
      my_ave_time_rusage = my_ave_time_rusage / 10;
      my_ave_time_rdtsc = my_ave_time_rdtsc / 10;
      printf("%-10d%-13d%-13d%-13llu%-13llu\n",dim,naive_ave_time_rusage,my_ave_time_rusage, naive_ave_time_rdtsc, my_ave_time_rdtsc);
		
    }

  return;  
}

void usage(char *progname) 
{
  fprintf(stderr, "Usage: %s \n", progname);    
  fprintf(stderr, "Options:\n");
  fprintf(stderr, "  -h         Print this message\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  int seed = 1729;
  srand(seed);

 
  test_rotate();
  test_smooth();
    
  return 0;
}
