#!/usr/bin/env perl
#

$help = qq`

This script runs all 12 benchmarks on your CPU, using the CPU1000
switch to make it run many times, and computes a single-run estimate
for each that can be compared to HLS results.

It uses -O0, which is most useful as a baseline of comparison, since
-O3 is specifically CPU-biased and therefore a poor indicator of what
a spatial computing optimiser should attain. See Munafo 2024 thesis
section 3.8.1, "Why CPU-Oriented Meta-Optimization Doesn’t Matter".

`;

$unused_header = qq`

Typical output (Coffee Lake 2.30 GHz):

  raw   per run  program
 4.016 2.008e-05 adpcm-chs.c
 3.524 7.048e-06 aes-chs.c
 6.367 1.273e-04 blowfish-chs.c
 3.253 3.253e-06 dhry-legup.c
 2.608 1.304e-06 fft-caad.c
 4.127 4.127e-06 gsm-chs.c
 5.773 1.155e-05 iterfl-caad.c
 3.977 1.989e-05 mmult-legup.c
 3.541 3.541e-06 motion-chs.c
 3.622 3.622e-05 qsort-legup.c
 2.807 1.403e-04 sha-chs.c
 3.456 3.456e-05 sor-caad.c

REVISION HISTORY
 20240105 First version
 20240115 Measure -O3 and show ratio; use -std=c9x

`;

sub run1
{
  my($prog, $mult, $opts) = @_;
  my($cmd, $bin, $usec, $out);

  $mult = int($mult * $itscale);
  $bin = "x";
  if (-f $bin) {
    unlink($bin);
  }
  $cmd = "gcc $opts -std=c9x -DCPU1000=$mult $prog -o $bin";
  print "$cmd\n";
  system($cmd);
  if(!(-x $bin)) {
    die "Got no executable $bin from $cmd\n";
  }
  $cmd = "bash -c 'time ./$bin' 2>&1";
  print "$cmd\n";
  $out = `$cmd`;
  $out =~ s/\t/ /go;

  if ($out =~ m/ ([.0-9]+) user/) {
    $raw = $1;
  } elsif ($out =~ m/user 0m([.0-9]+)s/) {
    $raw = $1;
  } else {
    die "parse error:\n$out";
  }
  $each = $raw/$mult;
  $usec = 1.0e6 * $each;
  return (($raw, $each, $usec));
  $g_results[$grn++] = sprintf("%6.3f %9.3e %7.3f %s",
            $raw, $each, $usec, $prog);
  return (($raw, $each, $usec));
} # End of run.1

sub run2
{
  my($prog, $mult) = @_;
  my ($raw, $each, $usec, $t1, $t2, $us3, $rat);
  ($raw, $each, $usec) = &run1($prog, $mult, "-O0");
  ($t1, $t2, $us3) = &run1($prog, $mult, "-O3");
  $rat = $usec/$us3;
  $g_results[$grn++] = sprintf("%6.3f %9.3e %7.3f %7.3f %7.3f %s",
            $raw, $each, $usec, $us3, $rat, $prog);
} # End of run.2

$| = 1;

$itscale = 1;
while ($arg = shift) {
  if (0) {
  } elsif ($arg =~ m/^--?h(elp)?$/) {
    print $help;
    exit(0);
  } elsif ($arg =~ m/^[.0-9]+$/) {
    $itscale = $arg;
    print "scale iterations by $itscale\n";
  } else {
    die "unrecognised argument '$arg', or file '$arg.c' does not exist\n";
  }
}

&run2("adpcm-chs.c", 200000);
&run2("aes-chs.c", 500000);
&run2("blowfish-chs.c", 50000);
&run2("dhry-legup.c", 1000000);
&run2("fft-caad.c", 2000000);
&run2("gsm-chs.c", 1000000);
&run2("iterfl-caad.c", 500000);
&run2("mmult-legup.c", 200000);
&run2("motion-chs.c", 1000000);
&run2("qsort-legup.c", 100000);
&run2("sha-chs.c", 20000);
&run2("sor-caad.c", 100000);

print " time    t/run     usec     -O3  speedup program\n";
for($i=0; $i<$grn; $i++) {
  print "$g_results[$i]\n";
}
