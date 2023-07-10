/* FFT, adapted from rosettacode.org/wiki/Fast_Fourier_transform by Robert Munafo
   for the 2023 HLS PPO training project. */

#include <stdio.h>

#define N 32

#define GENDATA 0
#define SHOW 0
#define USE_CPLX 0
#define USE_CPXFLT 0
#define USE_FLTFLT 1

#if USE_CPLX
#include <math.h>
#include <complex.h>
typedef double complex flt_cplx;
#endif

/* "twiddle" initialisers */
#define TRAW00R 1.000000
#define TRAW00I -0.000000
#define TRAW01R 0.995185
#define TRAW01I -0.098017
#define TRAW02R 0.980785
#define TRAW02I -0.195090
#define TRAW03R 0.956940
#define TRAW03I -0.290285
#define TRAW04R 0.923880
#define TRAW04I -0.382683
#define TRAW05R 0.881921
#define TRAW05I -0.471397
#define TRAW06R 0.831470
#define TRAW06I -0.555570
#define TRAW07R 0.773010
#define TRAW07I -0.634393
#define TRAW08R 0.707107
#define TRAW08I -0.707107
#define TRAW09R 0.634393
#define TRAW09I -0.773010
#define TRAW10R 0.555570
#define TRAW10I -0.831470
#define TRAW11R 0.471397
#define TRAW11I -0.881921
#define TRAW12R 0.382683
#define TRAW12I -0.923880
#define TRAW13R 0.290285
#define TRAW13I -0.956940
#define TRAW14R 0.195090
#define TRAW14I -0.980785
#define TRAW15R 0.098017
#define TRAW15I -0.995185
#define TRAW16R -0.000000
#define TRAW16I -1.000000
#define TRAW17R -0.098017
#define TRAW17I -0.995185
#define TRAW18R -0.195090
#define TRAW18I -0.980785
#define TRAW19R -0.290285
#define TRAW19I -0.956940
#define TRAW20R -0.382684
#define TRAW20I -0.923880
#define TRAW21R -0.471397
#define TRAW21I -0.881921
#define TRAW22R -0.555570
#define TRAW22I -0.831470
#define TRAW23R -0.634393
#define TRAW23I -0.773010
#define TRAW24R -0.707107
#define TRAW24I -0.707107
#define TRAW25R -0.773010
#define TRAW25I -0.634393
#define TRAW26R -0.831470
#define TRAW26I -0.555570
#define TRAW27R -0.881921
#define TRAW27I -0.471397
#define TRAW28R -0.923880
#define TRAW28I -0.382683
#define TRAW29R -0.956940
#define TRAW29I -0.290285
#define TRAW30R -0.980785
#define TRAW30I -0.195090
#define TRAW31R -0.995185
#define TRAW31I -0.098017

#if 0
/* N=16 initialisers */
#define TRAW00R 1.000000
#define TRAW00I -0.000000
#define TRAW01R 0.980785
#define TRAW01I -0.195090
#define TRAW02R 0.923880
#define TRAW02I -0.382683
#define TRAW03R 0.831470
#define TRAW03I -0.555570
#define TRAW04R 0.707107
#define TRAW04I -0.707107
#define TRAW05R 0.555570
#define TRAW05I -0.831470
#define TRAW06R 0.382683
#define TRAW06I -0.923880
#define TRAW07R 0.195090
#define TRAW07I -0.980785
#define TRAW08R -0.000000
#define TRAW08I -1.000000
#define TRAW09R -0.195090
#define TRAW09I -0.980785
#define TRAW10R -0.382684
#define TRAW10I -0.923880
#define TRAW11R -0.555570
#define TRAW11I -0.831470
#define TRAW12R -0.707107
#define TRAW12I -0.707107
#define TRAW13R -0.831470
#define TRAW13I -0.555570
#define TRAW14R -0.923880
#define TRAW14I -0.382683
#define TRAW15R -0.980785
#define TRAW15I -0.195090
#endif

#if USE_CPLX
flt_cplx k_cexp[N] = {
/*  1.000000 + -0.000000 * I, 0.923880 + -0.382683 * I,
  0.707107 + -0.707107 * I, 0.382683 + -0.923880 * I,
  0.000000 + -1.000000 * I, -0.382683 + -0.923880 * I,
 -0.707107 + -0.707107 * I, -0.923880 + -0.382683 * I */
  TRAW00R + TRAW00I * I,  TRAW01R + TRAW01I * I,  TRAW02R + TRAW02I * I,
  TRAW03R + TRAW03I * I,  TRAW04R + TRAW04I * I,  TRAW05R + TRAW05I * I,
  TRAW06R + TRAW06I * I,  TRAW07R + TRAW07I * I,  TRAW08R + TRAW08I * I,
  TRAW09R + TRAW09I * I,  TRAW10R + TRAW10I * I,  TRAW11R + TRAW11I * I,
  TRAW12R + TRAW12I * I,  TRAW13R + TRAW13I * I,  TRAW14R + TRAW14I * I,
  TRAW15R + TRAW15I * I,  TRAW16R + TRAW16I * I,  TRAW17R + TRAW17I * I,
  TRAW18R + TRAW18I * I,  TRAW19R + TRAW19I * I,  TRAW20R + TRAW20I * I,
  TRAW21R + TRAW21I * I,  TRAW22R + TRAW22I * I,  TRAW23R + TRAW23I * I,
  TRAW24R + TRAW24I * I,  TRAW25R + TRAW25I * I,  TRAW26R + TRAW26I * I,
  TRAW27R + TRAW27I * I,  TRAW28R + TRAW28I * I,  TRAW29R + TRAW29I * I,
  TRAW30R + TRAW30I * I,  TRAW31R + TRAW31I * I
};
flt_cplx in_time[N] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
flt_cplx out_freq[N];
#endif

#if USE_CPXFLT
typedef struct cplxfloat { float r; float i; } complexfloat;
complexfloat cflt_exp[N] = {
/*{ 1.000000, -0.000000}, { 0.923880, -0.382683},
  { 0.707107, -0.707107}, { 0.382683, -0.923880},
  { 0.000000, -1.000000}, {-0.382683, -0.923880},
  {-0.707107, -0.707107}, {-0.923880, -0.382683} */
{TRAW00R, TRAW00I}, {TRAW01R, TRAW01I}, {TRAW02R, TRAW02I}, {TRAW03R, TRAW03I},
{TRAW04R, TRAW04I}, {TRAW05R, TRAW05I}, {TRAW06R, TRAW06I}, {TRAW07R, TRAW07I},
{TRAW08R, TRAW08I}, {TRAW09R, TRAW09I}, {TRAW10R, TRAW10I}, {TRAW11R, TRAW11I},
{TRAW12R, TRAW12I}, {TRAW13R, TRAW13I}, {TRAW14R, TRAW14I}, {TRAW15R, TRAW15I},
{TRAW16R, TRAW16I}, {TRAW17R, TRAW17I}, {TRAW18R, TRAW18I}, {TRAW19R, TRAW19I},
{TRAW20R, TRAW20I}, {TRAW21R, TRAW21I}, {TRAW22R, TRAW22I}, {TRAW23R, TRAW23I},
{TRAW24R, TRAW24I}, {TRAW25R, TRAW25I}, {TRAW26R, TRAW26I}, {TRAW27R, TRAW27I},
{TRAW28R, TRAW28I}, {TRAW29R, TRAW29I}, {TRAW30R, TRAW30I}, {TRAW31R, TRAW31I}
};
complexfloat cflt_time[N] = {
  {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0},
  {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0}, {1,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
  {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};
complexfloat cflt_freq[N];
# define CADD(dest, a, b) \
    (dest).r = (a).r + (b).r; \
    (dest).i = (a).i + (b).i;
# define CSUB(dest, a, b) \
    (dest).r = (a).r - (b).r; \
    (dest).i = (a).i - (b).i;
# define CMULT(dest, a, b) \
    (dest).r = (a).r*(b).r - (a).i*(b).i; \
    (dest).i = (a).r*(b).i + (a).i*(b).r;
#endif

#if USE_FLTFLT
float fltflt_exp[2*N] = {
 TRAW00R, TRAW00I, TRAW01R, TRAW01I, TRAW02R, TRAW02I, TRAW03R, TRAW03I,
 TRAW04R, TRAW04I, TRAW05R, TRAW05I, TRAW06R, TRAW06I, TRAW07R, TRAW07I,
 TRAW08R, TRAW08I, TRAW09R, TRAW09I, TRAW10R, TRAW10I, TRAW11R, TRAW11I,
 TRAW12R, TRAW12I, TRAW13R, TRAW13I, TRAW14R, TRAW14I, TRAW15R, TRAW15I,
 TRAW16R, TRAW16I, TRAW17R, TRAW17I, TRAW18R, TRAW18I, TRAW19R, TRAW19I,
 TRAW20R, TRAW20I, TRAW21R, TRAW21I, TRAW22R, TRAW22I, TRAW23R, TRAW23I,
 TRAW24R, TRAW24I, TRAW25R, TRAW25I, TRAW26R, TRAW26I, TRAW27R, TRAW27I,
 TRAW28R, TRAW28I, TRAW29R, TRAW29I, TRAW30R, TRAW30I, TRAW31R, TRAW31I
};
float fltflt_time[2*N] = {1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0,
                          1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0, 1,0,
                          0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0,
                          0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0};
float fltflt_freq[2*N];
# define FFADD(dr,di, ar,ai, br,bi) \
    (dr) = (ar) + (br); \
    (di) = (ai) + (bi);
# define FFSUB(dr,di, ar,ai, br,bi) \
    (dr) = (ar) - (br); \
    (di) = (ai) - (bi);
# define FFMULT(dr,di, ar,ai, br,bi) \
    (dr) = (ar)*(br) - (ai)*(bi); \
    (di) = (ar)*(bi) + (ai)*(br);
#endif

#if USE_CPLX
void fft_recursive(flt_cplx out[], flt_cplx inp[], int step)
{
  if (step < N) {
    fft_recursive(inp, out, step * 2);
    fft_recursive(inp + step, out + step, step * 2);

    for (int i = 0; i < N; i += 2 * step) {
      /* cplx t = cexp(-I * PI * i / N) * inp[i + step]; */
      flt_cplx t = k_cexp[i] * inp[i + step];
      out[i / 2]   = inp[i] + t;
      out[(i + N)/2] = inp[i] - t;
    }
  }
}
#endif

#if USE_CPXFLT
void cflt_fft_recur(complexfloat out[], complexfloat inp[], int step)
{
  if (step < N) {
    cflt_fft_recur(inp, out, step * 2);
    cflt_fft_recur(inp + step, out + step, step * 2);

    for (int i = 0; i < N; i += 2 * step) {
      /* cplx t = cexp(-I * PI * i / N) * inp[i + step]; */
      complexfloat t;
      CMULT(t, cflt_exp[i], inp[i + step]);
      CADD(out[i / 2], inp[i], t);
      CSUB(out[(i + N)/2], inp[i], t);
    }
  }
}
#endif

#if USE_FLTFLT
void fltflt_fft_rec16(float out[], float inp[])
{
#define STEP16 16
/*fltflt_fft_rec32(inp,            out,);
  fltflt_fft_rec32(inp + 2*STEP16, out + 2*STEP16); */
  for (int i = 0; i < N; i += 2 * STEP16) {
    float tr, ti;
    FFMULT(tr,ti, fltflt_exp[2*i],fltflt_exp[2*i+1], inp[2*(i+STEP16)],inp[2*(i+STEP16)+1]);
    FFADD(out[2*i/2],out[2*i/2+1], inp[2*i],inp[2*i+1], tr,ti);
    FFSUB(out[2*(i+N)/2],out[2*(i+N)/2+1], inp[2*i],inp[2*i+1], tr,ti);
  }
}

void fltflt_fft_rec8(float out[], float inp[])
{
#define STEP8 8
  fltflt_fft_rec16(inp,           out);
  fltflt_fft_rec16(inp + 2*STEP8, out + 2*STEP8);
  for (int i = 0; i < N; i += 2 * STEP8) {
    float tr, ti;
    FFMULT(tr,ti, fltflt_exp[2*i],fltflt_exp[2*i+1], inp[2*(i+STEP8)],inp[2*(i+STEP8)+1]);
    FFADD(out[2*i/2],out[2*i/2+1], inp[2*i],inp[2*i+1], tr,ti);
    FFSUB(out[2*(i+N)/2],out[2*(i+N)/2+1], inp[2*i],inp[2*i+1], tr,ti);
  }
}

void fltflt_fft_rec4(float out[], float inp[])
{
#define STEP4 4
  fltflt_fft_rec8(inp,           out);
  fltflt_fft_rec8(inp + 2*STEP4, out + 2*STEP4);
  for (int i = 0; i < N; i += 2 * STEP4) {
    float tr, ti;
    FFMULT(tr,ti, fltflt_exp[2*i],fltflt_exp[2*i+1], inp[2*(i+STEP4)],inp[2*(i+STEP4)+1]);
    FFADD(out[2*i/2],out[2*i/2+1], inp[2*i],inp[2*i+1], tr,ti);
    FFSUB(out[2*(i+N)/2],out[2*(i+N)/2+1], inp[2*i],inp[2*i+1], tr,ti);
  }
}

void fltflt_fft_rec2(float out[], float inp[])
{
#define STEP2 2
  fltflt_fft_rec4(inp,           out);
  fltflt_fft_rec4(inp + 2*STEP2, out + 2*STEP2);
  for (int i = 0; i < N; i += 2 * STEP2) {
    float tr, ti;
    FFMULT(tr,ti, fltflt_exp[2*i],fltflt_exp[2*i+1], inp[2*(i+STEP2)],inp[2*(i+STEP2)+1]);
    FFADD(out[2*i/2],out[2*i/2+1], inp[2*i],inp[2*i+1], tr,ti);
    FFSUB(out[2*(i+N)/2],out[2*(i+N)/2+1], inp[2*i],inp[2*i+1], tr,ti);
  }
}

void fltflt_fft_rec1(float out[], float inp[])
{
#define STEP1 1
  fltflt_fft_rec2(inp,           out);
  fltflt_fft_rec2(inp + 2*STEP1, out + 2*STEP1);
  for (int i = 0; i < N; i += 2 * STEP1) {
    float tr, ti;
    FFMULT(tr,ti, fltflt_exp[2*i],fltflt_exp[2*i+1], inp[2*(i+STEP1)],inp[2*(i+STEP1)+1]);
    FFADD(out[2*i/2],out[2*i/2+1], inp[2*i],inp[2*i+1], tr,ti);
    FFSUB(out[2*(i+N)/2],out[2*(i+N)/2+1], inp[2*i],inp[2*i+1], tr,ti);
  }
}
#endif

#if SHOW
# if USE_CPLX
void show(const char * s, flt_cplx buf[])
{
  printf("%s", s);
  for (int i = 0; i < N; i++) {
    double t = cimag(buf[i]);
    if (i>0) { printf(", "); }
    printf("%g", creal(buf[i]));
    if (t >= 0) {
      printf("+%g*I", t);
    } else {
      printf("-%g*I", -t);
    }
  }
  printf("\n");
}
#endif

# if USE_CPXFLT
void cflt_shw(const char * s, complexfloat buf[])
{
  printf("%s", s);
  for (int i = 0; i < N; i++) {
    float t = buf[i].i;
    if (i>0) { printf(", "); }
    printf("%g", buf[i].r);
    if (t >= 0) {
      printf("+%g*I", t);
    } else {
      printf("-%g*I", -t);
    }
  }
  printf("\n");
}
# endif

# if USE_FLTFLT
void fltflt_shw(const char * s, float buf[])
{
  printf("%s", s);
  for (int i = 0; i < N; i++) {
    float ti = buf[2*i+1];
    if (i>0) { printf(", "); }
    printf("%g", buf[2*i]);
    if (ti >= 0) {
      printf("+%g*I", ti);
    } else {
      printf("-%g*I", -ti);
    }
  }
  printf("\n");
}
# endif
#endif

int main()
{
  int n = N; int i; int csum = 0;
  /* printf("sizeof(creal) = %d\n", sizeof(creal(buf[0]))); */

#if GENDATA
  /* To run this, use -lm in the compile line */
  float PI = atan2f(1, 1) * 4;
  for(i=0; i<N; i++) {
    flt_cplx t = cexpf(-I * PI * i / n);
    printf("#define TRAW%02dR %f\n", i, creal(t));
    printf("#define TRAW%02dI %f\n", i, cimag(t));
  }
  printf("flt_cplx k_cexp[N] = {\n");
  for(i=0; i<N; i++) {
    if (i>0) { printf(", "); }
    printf(" TRAW%02dR", i);
    printf(" + TRAW%02dI * I", i);
  }
  printf("\n};\n");
  printf("complexfloat cflt_exp[N] = {\n");
  for(i=0; i<N; i++) {
    if (i>0) { printf(", "); }
    printf("{TRAW%02dR, TRAW%02dI}", i, i);
  }
  printf("\n};\n");
  printf("float fltflt_exp[2*N] = {\n");
  for(i=0; i<N; i++) {
    if (i>0) { printf(", "); }
    printf("TRAW%02dR, TRAW%02dI", i, i);
  }
  printf("\n};\n");
#endif

#if SHOW
# if USE_CPLX
    show("Data: ", in_time);
# else
#   if USE_CPXFLT
      cflt_shw("Data: ", cflt_time);
#   else
      fltflt_shw("Data: ", fltflt_time);
#   endif
# endif
#endif

#if SHOW
/* printf("expect: 4+0*I, 1-2.41421*I, 0+0*I, 1-0.414214*I"
                      ", 0+0*I, 1+0.414214*I, 0+0*I, 1+2.41421*I\n"); */
/* printf("expect: 8+0*I, 1-5.02734*I, 0+0*I, 0.999999-1.49661*I, 0+0*I, 1-0.668179*I, 0+0*I, 0.999999-0.198913*I, 0+0*I, 0.999999+0.198913*I, 0+0*I, 1+0.668179*I, 0+0*I, 1+1.49661*I, 0+0*I, 1+5.02734*I\n"); */
printf("expect: 16+0*I, 1-10.1532*I, 0+0*I, 0.999998-3.29656*I, 0+0*I, 0.999999-1.87087*I, 0+0*I, 0.999997-1.2185*I, 0+0*I, 1-0.820677*I, 0+0*I, 1-0.534512*I, 0+0*I, 1-0.303347*I, 0+0*I, 0.999999-0.0984899*I, 0+0*I, 0.999999+0.0984899*I, 0+0*I, 1+0.303346*I, 0+0*I, 1+0.534511*I, 0+0*I, 1+0.820677*I, 0+0*I, 0.999997+1.2185*I, 0+0*I, 1+1.87087*I, 0+0*I, 1+3.29656*I, 0+0*I, 1+10.1532*I\n");
#endif

#if USE_CPLX
    for (i = 0; i < N; i++) out_freq[i] = in_time[i];
    fft_recursive(in_time, out_freq, 1);
# if SHOW
    show(  " cflt : ", in_time);
#  endif
  csum = 0;
  for(i=0; i<N; i++) {
    float f = 0; int t = 0;
    f = creal(out_freq[i]); t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<2) | ((csum >> 30) & 3);
    f = cimag(out_freq[i]); t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<1) | ((csum >> 31) & 1);
  }
#endif

#if USE_CPXFLT
  for (i = 0; i < N; i++) cflt_freq[i] = cflt_time[i];
  cflt_fft_recur(cflt_time, cflt_freq, 1);
# if SHOW
    cflt_shw( "cfFFT : ", cflt_time);
# endif
  csum = 0;
  for(i=0; i<N; i++) {
    float f = 0; int t = 0;
    f = cflt_freq[i].r; t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<2) | ((csum >> 30) & 3);
    f = cflt_freq[i].i; t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<1) | ((csum >> 31) & 1);
  }
#endif

#if USE_FLTFLT
  for (i = 0; i < 2*N; i++) fltflt_freq[i] = fltflt_time[i];
  fltflt_fft_rec1(fltflt_time, fltflt_freq);
# if SHOW
    fltflt_shw( "ffFFT : ", fltflt_time);
# endif
  csum = 0;
  for(i=0; i<N; i++) {
    float f = 0; int t = 0;
    f = fltflt_freq[2*i];   t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<2) | ((csum >> 30) & 3);
    f = fltflt_freq[2*i+1]; t = *((int *)(&f));
    csum = csum ^ t; csum = (csum<<1) | ((csum >> 31) & 1);
  }
#endif

#define CSUM8 0x9b4849bd
#define CSUM16a 0xd7630e51
#define CSUM16b 0xa964e9cf
#define CSUM32a 0xbb39b809
#define CSUM32b 0xd5cc0eb2

  printf("csum: %08x\n", csum);
  if ((USE_CPLX || USE_CPXFLT || USE_FLTFLT)
     && (csum != CSUM32a) && (csum != CSUM32b)
     && (csum != CSUM16a) && (csum != CSUM16b) && (csum != CSUM8))
  {
    printf("FAIL\n");
  } else {
    printf("PASS\n");
  }

  return 0;
}
