/* Floating-point iteration with testing  -*- C++ -*-
   Author: Robert Munafo
   Date: 2023 Jun 26

Define GEN_DATA to 1 to have it run iter1() which prints out the data
to initialise the array ref_r.

The iter2 function shows the original straightforward algorithm. It
performs 12 multiplies and 17 adds, 2 memory reads and 3 comparisons
(occasionally more) per iteration.

iter3 has been adapted to perform four simultaneous and independent
tasks each equialent to iter2. This may or may not be able to run faster
than simply caling iter2 four times, depending on the aggressiveness
of the compiler and the ILP abilities of the target processor.

 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define GEN_DATA 0
#define USE_ITER3 0

#define REF_ITER 28
#define MAX_ITER 252

float ref_r[REF_ITER] = {
 0.0,         -1.786429882050, 1.404901742935, 0.187319159508, -1.751341342926,
 1.280766844749, -0.146066218615, -1.765094518661, 1.329128742218, -0.019846491516,
 -1.786036014557, 1.403494596481, 0.183367356658, -1.752806305885, 1.285899996758,
 -0.132891148329, -1.768769741058, 1.342116713524, 0.014847493730, -1.786209464073,
 1.404114246368, 0.185106828809, -1.752165317535, 1.283653497696, -0.138663664460,
 -1.767202258110, 1.336573958397, 0.0
};
float ref_i[REF_ITER] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

void iter1(double cr, double ci, int nmax)
{
  double zr, zi, zr2, zi2;
  int n;
  zr = zi = 0;
  for (n=0; n<nmax; n++) {
    ref_r[n] = zr;
    ref_i[n] = zi;
    zr2 = zr*zr - zi*zi + cr;
    zi2 = zr*zi*2.0 + ci;
    zr = zr2; zi = zi2;
  }

  for (n=0; n<nmax; n++) { printf(", %.12f", ref_r[n]); } printf("\n\n");
  for (n=0; n<nmax; n++) { printf(", %.12f", ref_i[n]); } printf("\n\n");

} /* End of iter1 */

#if 0
/* Iteration by differential estimation with Zhuoran exit criterion */
int iter2(float cr, float ci, int ref_period, float *Zref_r, float *Zref_i, int nmax)
{
  float zr, zi, zr2, zi2, Zr, Zi, abs_z, abs_Zz, zr3, zi3;
  int i, m;
  m = 0; zr = zi = 0;
  for (i=0; i<nmax; ++i) {
    Zr = Zref_r[m]; Zi = Zref_i[m];
    abs_Zz = (Zr+zr)*(Zr+zr) + (Zi+zi)*(Zi+zi);
    abs_z = zr*zr + zi*zi;
    if (abs_Zz < abs_z) {
      zr = Zr + zr;
      m = 0;
      Zr = Zref_r[m]; Zi = Zref_i[m];
    }
    abs_Zz = (Zr+zr)*(Zr+zr) + (Zi+zi)*(Zi+zi);
    if (abs_Zz > 8.0) {
      return i;
    }
    zr2 = Zr*2.0 + zr; // (2 * Z + z)
    zi2 = Zi*2.0 + zi; // (2 * Z + z)
    zr3 = zr2*zr - zi2*zi + cr; // (2 * Z + z) * z + c
    zi3 = zr2*zi + zi2*zr + ci; // (2 * Z + z) * z + c
    zr = zr3; zi = zi3;
    if (++m >= ref_period) {
      m = 0;
    }
  }
  return i;
} // End of iter2
#endif

/* Macros perform the work of iter2() and the I3STEP3 macro
   performs the "stop iterating now" without any conditionals
   which will allow us to run multiple sets of these macros
   (on different data) side-by-side within the same loop. */
#define I3STEP1(Zr, Zi, m, zr, zi, aZz, az) \
    (Zr) = Zref_r[(m)]; (Zi) = Zref_i[(m)]; \
    (aZz) = ((Zr)+(zr))*((Zr)+(zr)) + ((Zi)+(zi))*((Zi)+(zi)); \
    (az) = (zr)*(zr) + (zi)*(zi);
#define I3STEP2(Zr, Zi, m, zr, zi, aZz, az) \
    if ((aZz) < (az)) { \
      (zr) = (Zr) + (zr); (zi) = (Zi) + (zi); \
      (m) = 0; \
      (Zr) = Zref_r[(m)]; (Zi) = Zref_i[(m)]; \
    } \
    (aZz) = ((Zr)+(zr))*((Zr)+(zr)) + ((Zi)+(zi))*((Zi)+(zi));
#define I3STEP3(going, aZz, nogg, ans, i) \
    (going) = (going) & (-((aZz) < 8.0)); \
    (nogg) = ~(going); \
    (ans) = ((going) & ((i)+1)) | ((nogg) & (ans));
#define I3STEP4(zr2, zi2, zr3, zi3, Zr, Zi, zr, zi, cr, ci, m) \
    (zr2) = (Zr)*2.0 + (zr); \
    (zi2) = (Zi)*2.0 + (zi); \
    (zr3) = (zr2)*(zr) - (zi2)*(zi) + (cr); \
    (zi3) = (zr2)*(zi) + (zi2)*(zr) + (ci); \
    (zr) = (zr3); (zi) = (zi3); \
    if (++(m) >= ref_period) { (m) = 0; }

#if USE_ITER3
/* This version of iter2 does the same work as iter2 but uses a fixed
   loop limit, might be easier to unroll */
int iter3(float cr, float ci, int ref_period, float *Zref_r, float *Zref_i)
{
  float zr, zi, zr2, zi2, Zr, Zi, abs_z, abs_Zz, zr3, zi3; int m, going1, nogg1, ans1;
  int i;
  m = 0; zr = zi = 0; going1 = -1; nogg1 = ans1 = 0;
  for (i=0; i<MAX_ITER; ++i) {
    I3STEP1(Zr, Zi, m, zr, zi, abs_Zz, abs_z)
    I3STEP2(Zr, Zi, m, zr, zi, abs_Zz, abs_z)
    I3STEP3(going1, abs_Zz, nogg1, ans1, i);
    I3STEP4(zr2, zi2, zr3, zi3, Zr, Zi, zr, zi, cr, ci, m);
  }
  return ans1;
} // End of iter3
#else
/* This does four sets of iter3 in parallel */
int iter4(float cra, float cia, float crb, float cib,
  float crc, float cic,  float crd, float cid,
  int ref_period, float *Zref_r, float *Zref_i)
{
  float zra, zia, zr2a, zi2a, Zra, Zia, abs_za, abs_Zza, zr3a, zi3a;
  int ma, goinga, nogga, ansa;
  float zrb, zib, zr2b, zi2b, Zrb, Zib, abs_zb, abs_Zzb, zr3b, zi3b;
  int mb, goingb, noggb, ansb;
  float zrc, zic, zr2c, zi2c, Zrc, Zic, abs_zc, abs_Zzc, zr3c, zi3c;
  int mc, goingc, noggc, ansc;
  float zrd, zid, zr2d, zi2d, Zrd, Zid, abs_zd, abs_Zzd, zr3d, zi3d;
  int md, goingd, noggd, ansd;
  int i;
  ma = 0; zra = zia = 0; goinga = -1; nogga = ansa = 0;
  mb = 0; zrb = zib = 0; goingb = -1; noggb = ansb = 0;
  mc = 0; zrc = zic = 0; goingc = -1; noggc = ansc = 0;
  md = 0; zrd = zid = 0; goingd = -1; noggd = ansd = 0;
  for (i=0; i<MAX_ITER; ++i) {
    I3STEP1(Zra, Zia, ma, zra, zia, abs_Zza, abs_za)
    I3STEP2(Zra, Zia, ma, zra, zia, abs_Zza, abs_za)
    I3STEP3(goinga, abs_Zza, nogga, ansa, i)
    I3STEP4(zr2a, zi2a, zr3a, zi3a, Zra, Zia, zra, zia, cra, cia, ma)
    I3STEP1(Zrb, Zib, mb, zrb, zib, abs_Zzb, abs_zb)
    I3STEP2(Zrb, Zib, mb, zrb, zib, abs_Zzb, abs_zb)
    I3STEP3(goingb, abs_Zzb, noggb, ansb, i)
    I3STEP4(zr2b, zi2b, zr3b, zi3b, Zrb, Zib, zrb, zib, crb, cib, mb)
    I3STEP1(Zrc, Zic, mc, zrc, zic, abs_Zzc, abs_zc)
    I3STEP2(Zrc, Zic, mc, zrc, zic, abs_Zzc, abs_zc)
    I3STEP3(goingc, abs_Zzc, noggc, ansc, i)
    I3STEP4(zr2c, zi2c, zr3c, zi3c, Zrc, Zic, zrc, zic, crc, cic, mc)
    I3STEP1(Zrd, Zid, md, zrd, zid, abs_Zzd, abs_zd)
    I3STEP2(Zrd, Zid, md, zrd, zid, abs_Zzd, abs_zd)
    I3STEP3(goingd, abs_Zzd, noggd, ansd, i)
    I3STEP4(zr2d, zi2d, zr3d, zi3d, Zrd, Zid, zrd, zid, crd, cid, md)
  }
  return (ansa<<21) ^ (ansb<<14) ^ (ansc<<7) ^ ansd;
} // End of iter4
#endif

int main()
{
  int ans1, ans2, ans3, ans4, got;
#if GEN_DATA
  iter1(-1.786429858056, 0.0, REF_ITER);
  //                        Zr to iterate                reference Zr

  printf("%.12f\n", ((double)-1.78642674042) + ((double)1.786429858056));
  printf("%.12f\n", ((double)-1.78642700391) + ((double)1.786429858056));
  printf("%.12f\n", ((double)-1.78642712943) + ((double)1.786429858056));
  printf("%.12f\n", ((double)-1.78642716066) + ((double)1.786429858056));
#else
  // delta_cr = (-1.78642716066 - (-1.786429858056))
#if USE_ITER3
  ans1 = iter3(0.000003117636, -0.00000038117, 27, ref_r, ref_i);
  ans2 = iter3(0.000002854146, -0.00000023063, 27, ref_r, ref_i);
  ans3 = iter3(0.000002728626, -0.000000018644, 27, ref_r, ref_i);
  ans4 = iter3(0.000002697396, -0.00000019458, 27, ref_r, ref_i);
  got = (ans1<<21)^(ans2<<14)^(ans3<<7)^ans4;
#else
  got = iter4(0.000003117636, -0.00000038117, 0.000002854146, -0.00000023063,
              0.000002728626, -0.000000018644, 0.000002697396, -0.00000019458,
              27, ref_r, ref_i);
#endif
  int expect = (202<<21)^(223<<14)^(252<<7)^235;
  printf("got %08x\n", got);
  if (got == expect) {  printf("PASS\n");
  } else {    printf("FAIL\n");  }
#endif
  return 0;
}
