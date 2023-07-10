/* Successive Over Refinement  -*- C++ -*-
   Author: Robert Munafo  (with fixed-point maths from Andrew Canis)
   Date: 2023 Jun 10

To see the image of the SOR grid, define PRINT_IMG to 1
and then do:

  gcc sor.c -o s && ./s && (./s > sor.pbm) && convert sor.pbm sor.jpg && open sor.jpg

For LegUp use define PRINT_IMG to 0

 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void print_as_pbm(int width, int height, int max, unsigned char img[width][height])
{
  int i, j;
  printf("P2\n%d %d\n%d\n", width, height, max);
  for (j = 0; j < height; j++) {
    for (i = 0; i < width; i++) {
      /* assume grayscale image */
      printf("%d ", img[i][j]);
    }
    printf("\n");
  }
}

#define PRINT_IMG 0

#define FRACPREC 24
#define FRACMASK ((1<<FRACPREC)-1)
#define FM7 ((1<<(FRACPREC+3))-1)
#define int2fixed(num) ((num) << FRACPREC)
#define fixedmul(a, b) ((((long long)a) * ((long long)b)) >> FRACPREC)
#define fixed2int(num) ((num) >> FRACPREC)

#define WIDTH 32
#define HEIGHT 32
#define MAX_ITER 5
#define OMG 2

volatile int img[WIDTH][HEIGHT];

void sor()
{
  int i, j;
  int rnd1, rnd2, rnd3, rc;
  rnd1 = rnd2 = rnd3 = 1;

  // fill the array with quasi-randomness
  for (j = 0; j < HEIGHT; j++) {
    for (i = 0; i < WIDTH; i++) {
      if ((i==0) || (j==0)) {
        rc = 0;
      } else if ((i==(WIDTH-1)) || (j==(HEIGHT-1))) {
        rc = int2fixed(7);
      } else {
        rc = rnd1 ^ rnd2 ^ rnd3;
      }
      img[i][j] = rc & FM7;
      rnd1 = rnd1*5 + 3;
      rnd2 = rnd1*11 + 7;
      rnd3 = rnd1*17 + 13;
    }
  }

  for (int iter = 0; iter < MAX_ITER; iter++) {
    for (j = 1; j < (HEIGHT-1); j++) {
      int jm1 = j-1;
      int jp1 = j+1;
      for (i = 1; i < (WIDTH-1); i++) {
        int im1 = i-1;
        int ip1 = i+1;
        int nb_n = img[i][jm1];
        int nb_s = img[i][jp1];
        int nb_w = img[im1][j];
        int nb_e = img[ip1][j];
        int ctr = img[i][j];
//int dbg = ((i==1)&&(j==1) && 0);
//if (dbg) { printf("ctr = %d", ctr); }
//if (dbg) { printf("  nbrs %d\n", (nb_n+nb_s+nb_w+nb_e)); }
        int chg = ctr - ((nb_n+nb_s+nb_w+nb_e)>>2);
//if (dbg) { printf("  chg %d\n", chg); }
        ctr = ctr - chg;
//if (dbg) { printf("  -> %d\n", ctr); }
        img[i][j] = ctr;
      }
    }
  }
} /* End of sor */

int sorsum(void)
{
  int count = 0;
  int i, j;
  for (j = 0; j < HEIGHT; j++) {
    for (i = 0; i < WIDTH; i++) {
      count += (img[i][j] & 0xFF);
    }
  }
  return count;
}

int main()
{
  int count=0;

  sor(); count = sorsum();

#if PRINT_IMG
  int i, j;
  unsigned char final[WIDTH][HEIGHT];
  for (j = 0; j < HEIGHT; j++) {
    for (i = 0; i < WIDTH; i++) {
      final[i][j] = fixed2int(img[i][j]);
    }
  }
  int max = 0;
  for (j = 0; j < HEIGHT; j++) {
    for (i = 0; i < WIDTH; i++) {
      if (final[i][j] > max) { max = final[i][j]; }
    }
  }
  max = (max>15) ? 255 : ((max>3) ? 15 : ((max>1) ? 3 : 1) );
  print_as_pbm(WIDTH, HEIGHT, max, final);
#else
  printf("Count: %d\n", count);
  if (count == 118123) {
    printf("PASS\n");
  } else {
    printf("FAIL\n");
  }
#endif

  return 0;
}
