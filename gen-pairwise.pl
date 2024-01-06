#!/usr/bin/env perl
#

$g_correct_fmax = 1;
$g_use_negatives = 0;

$help = qq`

This script generates pairwise ordering statistics, in a format that
is easy for Python to parse line-by-line

`;

$unused_header = qq`

REVISION HISTORY
 20230620 First version

`;


$unused_header = q`
 {In earlier logs, (without Fmax scaling by RLbase.py) typical output looks like this:

When it needs to do a new compilation (memoising cache miss):

  [...]
  (RolloutWorker pid=3774705) accelCyc 40430 cyc at Fmax 12
  (RolloutWorker pid=3774705) hwc 40430 
  (RolloutWorker pid=3774705) macc:   : 40430
  (RolloutWorker pid=3774705)  1    new cktocycle 2203954092 = 40430 pop 1
  (RolloutWorker pid=3774705)  saving .ll and .tex files with prefix O0
  (RolloutWorker pid=3774705)  baseline: set O0_cyc to 40430
  (RolloutWorker pid=3774705) make accelerationCycle
  (RolloutWorker pid=3774705) accelCyc 16244 cyc at Fmax 12
  (RolloutWorker pid=3774705) hwc 16244 
  (RolloutWorker pid=3774705) macc:  -O3 : 16244
  (RolloutWorker pid=3774705)  2    new cktocycle  744476403 = 16244 pop 1
  (RolloutWorker pid=3774705)  saving .ll and .tex files with prefix O3
  (RolloutWorker pid=3774705)  baseline: set O3_cyc to 16244
  [...]
  (RolloutWorker pid=3774705) myEnv::step(23)
  (RolloutWorker pid=3774705) make accelerationCycle
  (RolloutWorker pid=3774705) accelCyc 8501 cyc at Fmax 12
  (RolloutWorker pid=3774705) hwc 8501 
  (RolloutWorker pid=3774705) macc: -inline -prune-eh -inline -scalarrepl-ssa -scalarrepl -memcpyopt -early-cse -strip -functionattrs -inline -loop-rotate -loop-unroll -loop-unroll -sccp -prune-eh -simplifycfg -correlated-propagation -loop-rotate -loop-unroll -loop-rotate  : 8501
  (RolloutWorker pid=3774705) grew-returning 31929
  (RolloutWorker pid=3774705) myEnv::reset()
  (RolloutWorker pid=3774705) myEnv::step(28)
  [...]

When it gets a cache hit:
  [...]
  {same initial O0 and O3 section}
  [...]
  (RolloutWorker pid=3774705) myEnv::step(33)
  (RolloutWorker pid=3774705) hwc 8501 
  (RolloutWorker pid=3774705) macc: -adce -scalarrepl-ssa -inline -scalarrepl -loop-rotate -prune-eh -globalopt -ipsccp -strip -loop-unroll -inline -loop-rotate -simplifycfg -correlated-propagation -low(RolloutWorker pid=3774705) 20230413.14:33:55 gHC ret 8501
  (RolloutWorker pid=3774705) grew-returning 31929
  (RolloutWorker pid=3774705) myEnv::reset()
  (RolloutWorker pid=3774705) myEnv::step(1)
  [...]
 -0623}

After Fmax adjustment was implemented, it looks like this:
  [...]
  (RolloutWorker pid=2047406) accelCyc 48080 cyc at Fmax 310
  (RolloutWorker pid=2047406) hwc 48080 
  (RolloutWorker pid=2047406) macc:   : 7754
  (RolloutWorker pid=2047406)  1    new cktocycle 2897879774 = 7754 pop 1
  (RolloutWorker pid=2047406)  saving .ll and .tex files with prefix O0
  (RolloutWorker pid=2047406)  baseline: set O0_cyc to 7754
  (RolloutWorker pid=2047406) make accelerationCycle
  (RolloutWorker pid=2047406) accelCyc 40937 cyc at Fmax 310
  (RolloutWorker pid=2047406) hwc 40937 
  (RolloutWorker pid=2047406) macc:  -O3 : 6602
  (RolloutWorker pid=2047406)  2    new cktocycle 1852297508 = 6602 pop 1
  (RolloutWorker pid=2047406)  saving .ll and .tex files with prefix O3
  (RolloutWorker pid=2047406)  baseline: set O3_cyc to 6602
  [...]
  (RolloutWorker pid=2047406) myEnv::step(3)
  (RolloutWorker pid=2047406) make accelerationCycle
  (RolloutWorker pid=2047406) accelCyc 40973 cyc at Fmax 310
  (RolloutWorker pid=2047406) hwc 40973 
  (RolloutWorker pid=2047406) macc: -loop-rotate -ipsccp -tailcallelim -inline -jump-threading -strip-nondebug -deadargelim -strip-nondebug -globaldce -loop-deletion -globaldce -globaldce -early-cse -gvn -gvn -simplifycfg -loop-simplify -gvn -loop-deletion -functionattrs -inline -strip-nondebug -ipsccp -break-crit-edges -reassociate -jump-threading -inline -constmerge -licm -strip  : 6608
  (RolloutWorker pid=2047406) 20230616.04:40:08 gHC ret 6608
  (RolloutWorker pid=2047406) grew: new min_cycles = 6608
  (RolloutWorker pid=2047406) grew-returning 1146
  (RolloutWorker pid=2047406) myEnv::reset()
  (RolloutWorker pid=2047406) myEnv::step(21)
  [...]

"accelCyc" is the cycle count at the found fMax
  compare to "6608" which is normalised to a 50 MHz frequency:
  40973 * 50 / 310 = 6608.548

Thus we can get all pairs and the reward from the "macc:" line.
  {But if it's the older type of output we must also notice a prior
"hwc" line and do the Fmax scaling ourselves.
  If there is a cache "hit" the Fmax is not given, but we can figure
out if Fmax adjustment is being done right away at the beginning of
each experiment run. -0623}

rewards also need to be scaled relative to the program's baseline
performance. These are also output in the logs and look simply like
this:

  macc:   : 7754
  macc:  -O3 : 6602

`;

# Scale a reward to a range of [0..1] == [O0..O3]
# for example:
#    cycles=3700   O3=2800  O0=4000
#    val=300
#    O3 is 1200 better than O0
#    val is 1/4 as much as that
#    scaled amount is (4000-3700)/(4000-2800) = 0.25
sub rscale
{
  my ($cycles, $O0, $O3) = @_;
  if ($O3 >= $O0) {
    return 0;
  }
  return (($O0-$cycles)/($O0-$O3));
};
#print ("rscale(3700, 4000, 2800) = " . &rscale(3700, 4000, 2800) , "\n"); exit(0);

sub addweights
{
  my($passes, $adjcyc) = @_;
  my($i, $j, $p1, $p2); my(@ps);
  my($np, $npairs, $k);
  @ps = ();
  foreach $p1 (split(/ /, $passes)) {
    if ($p1 ne '') {
      push(@ps, $p1);
    }
  }
  $np = $#ps+1;
  if ($np == 0) {
    return;
    # die "np==0  passes == '$passes'\n";
  }
  $npairs = $np*($np-1)/2;
  $g_awc++;
  if ($g_awc >= 10000) {
    $ggtot += $g_awc;
    print " $ggtot $np $npairs $gs4fname   \r"; #  $passes\n";
    $g_awc = 0;
  }

  # count freq. of each pair
  my (%tl_wn);
  undef %tl_wn; # This Line (of compiler opts) Weights N
  for($i=0; $i<$np; $i++) {
    for($j=$i+1; $j<$np; $j++) {
      $k = "$ps[$i] $ps[$j]";
      $tl_wn{$k} ++;
    }
  }

  # The amount of "little reward" to add to the global total should
  # be scaled by the number of opts. in this particular compiler invocation
  my($little);
  $little = $adjcyc/$npairs;  # each pair's share of the reward

  # The contribution to each pairwise global weight should be scaled
  # down to correct for duplication. This scaling is different for each
  # weight-pair.
  # We're counting it as if the "little" reward is earned sqrt(n) times
  # where the N is different for each weight-pair
  my($scaled_wn);
  foreach $k (keys %tl_wn) {
    $scaled_wn = sqrt($tl_wn{$k});
    $weights{$k} = $weights{$k} + $scaled_wn * $little;
    $wn{$k} += $scaled_wn;
  }
} # End of add.weights

if (0) {
  # Quick test of add.weights
  #&addweights(" -loop-rotate -ipsccp -tailcallelim -inline -jump-threading -strip-nondebug -deadargelim -strip-nondebug -globaldce -loop-deletion -globaldce -globaldce -early-cse -gvn -gvn -simplifycfg -loop-simplify -gvn -loop-deletion -functionattrs -inline -strip-nondebug -ipsccp -break-crit-edges -reassociate -jump-threading -inline -constmerge -licm -strip", 7754-6602);
  &addweights(" A B C D E F G F H I H H J K K L M K I N D F B O P E D Q R S", 7754-6602);
  &addweights(" A B C D E F G H I J K L M N O Q R S", 8000-6602);
  foreach $k (sort {$wn{$a} <=> $wn{$b}} (keys %wn)) {
    print sprintf("%3.1f $k %f\n", $wn{$k}, $weights{$k}/$wn{$k});
  }
  exit(0);
}

# Scan a single file, glean rewards from all experiments
sub scan4
{
  my($inp) = @_;
  my($IN, $l, $collecting, $gg);
  my($g_prog, $g_var, $g_seed, $g_iter);

  print STDERR "scanning input: $inp\n";
  $gs4fname = $inp;
  if ($inp =~ m/\.gz$/) {
    open($IN, "cat $inp | gunzip |");
  } else {
    open($IN, $inp);
  }

  $collecting = 0;
  $x_ghc_2 = 0;
  while ($l = <$IN>) {
    if ($l =~ m/^RLBASE_PROG=(.+)$/) {
      $g_prog = $1; # &cn_prog($1);
    } elsif ($l =~ m/^RLBASE_VARIANT=(.+)$/) {
      $g_var = $1;
    } elsif ($l =~ m/^RLBASE_SEED=(\d+)$/) {
      $g_seed = $1;
      # print "% === seed $g_seed\n";
    } elsif ($l =~ m/^RLBASE_N_ITER=(\d+)$/) {
      $g_iter = $1;
    }

    if (($g_prog ne "") && ($g_var ne "") && ($g_seed ne "") && ($g_iter ne "")
    ) {
      $collecting = 1;
    }

    if ($collecting && ($l =~ m|mainloop: Finished|)) {
      # end of an experiment, reset per-experiment variables
      $collecting = 0;
      $O0 = $O3 = 0;
      $fmax_ignored_known = 0; $unadj_cyc = 0;
      $baseline_fmax = 0; $did_f50_adjust = 0;
    }

    if ($collecting) {
      if (0) {
      } elsif (($baseline_fmax == 0) && ($l =~ m|cyc at Fmax ([0-9]+)|)) {
         $baseline_fmax = $1;
         # print "got baseline Fmax = $baseline_fmax     \n";
      } elsif (($O0 == 0) && ($l =~ m|macc: +: ([0-9]+)|)) {
         $O0 = $1;
         # print "got O0 = $O0\n";
      } elsif (($O3 == 0) && ($l =~ m|macc: +-O3 +: ([0-9]+)|)) {
         $O3 = $1;
         # print "got O3 = $O3\n";
      } elsif ($l =~ m|hwc ([0-9]+)|) {
         $unadj_cyc = $1;
      } elsif ($l =~ m|macc: ([- a-z0-9]+)+ +: ([0-9]+)|) {
         $passes = $1; $f50cyc = $2;
         if ($f50cyc == $unadj_cyc) {
           if ($g_correct_fmax) {
             $fmax_ignored_known = 1;
           }
         }
         if ($fmax_ignored_known && ($baseline_fmax > 0) && ($O0*$O3 > 0)) {
           if ($did_f50_adjust == 0) {
             $O0 = int($O0 * 50 / $baseline_fmax);
             $O3 = int($O3 * 50 / $baseline_fmax);
             $did_f50_adjust = 1;
           }
         }
         if ($did_f50_adjust) {
           $f50cyc = int($f50cyc * 50 / $baseline_fmax);
         }
         if ($O0*$O3 > 0) {
           $adjrew = &rscale($f50cyc, $O0, $O3);  # This would be the full reward
           if (($adjrew > 0) || ($g_use_negatives)) {
             &addweights($passes, $adjrew);
           }
         }
      }
    }
  }
  close $IN;
} # End of scan.4

$| = 1;

while ($arg = shift) {
  if (0) {
  } elsif ($arg =~ m/^--?h(elp)?$/) {
    print $help;
    exit(0);
  } elsif ($arg eq '-v') {
    $verbose = 1;
  } elsif ($arg eq '-vv') {
    $verbose = 2;

  } else {
    die "unrecognised argument '$arg', or file '$arg.c' does not exist\n";
  }
}

$ts = time;

open($LIST, "l-survey-list.txt");
while($l = <$LIST>) {
  chomp $l;    #     1          2          3               4              5          6
  if ($l =~ m/^ +([-a-z]+):([a-z0-9]+):(mlp|lstm):([a-z0-9]+:s[0-9]+):i([0-9]+):(hgram|ahist) (202[^ ]+-log[^ ]+)$/) {
    $tup = "$1:$4"; $agent=$2; $lstm=$3; $iter=$5; $ahist=$6; $lfname = $7;
    $tup = "$1:$2:$3:$4:$6";
    # Keep the longest experiment of each tuple
    if ($iter >= $fiter{$tup}) {
      $fsrc{$tup} = $lfname;
      $fiter{$tup} = $iter;
    }
  }
}
close $LIST;

foreach $k (keys %fsrc) {
  $pn = "../logs/" . $fsrc{$k};
  if ($fsrc{$k} ne '') {
    $scanfiles{$pn} = 1;
  }
}

$gg = 1;
foreach $pn (sort (keys %scanfiles)) {
  if ($gg) {
    print STDERR "scan4($pn)\n";
    &scan4($pn);
#    $gg = 0;
  }
}

$tt = time - $ts;

print "\n";
undef %wavg;
foreach $k (keys %wn) {
  $wavg{$k} = $weights{$k}/$wn{$k};
}
$fn = "p-pairwise-weights-" . ($g_use_negatives ? "pn" : "pos") . ".txt";
print "Writing: $fn\n";
open($OUT, "> $fn");
foreach $k (sort {$wavg{$a} <=> $wavg{$b}} (keys %wn)) {
  $l = sprintf(" %3.1f %-30s %f\n", $wn{$k}, $k, $weights{$k}/$wn{$k});
  print $l;
  print $OUT $l;
}
close $OUT;

print " $ggtot data used; $tt sec\n";
