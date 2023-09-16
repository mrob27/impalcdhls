#!/usr/bin/env perl
#
# survey the logfiles and make a list of experiments that are
# big enough to use for plotting etc.

$lfname = "l-survey-list.txt";
$lfnew = "l-svl2.txt";

$help = qq`

Logs should have names like ../logs/20230425.154745.txt.gz and contain
the output of a run of "PPOexperiment.py"

OPTIONS

Give an option in the form "20230425.1200" to preserve all items older
than the indicated day and time. This speeds up the process by only
scanning the newer logs.

`;

$unused_header = qq`

REVISION HISTORY
 20230505 First version
 20230518 Handle agent types; require $n_ghc >= 200
 20230605 Add the hgram/ahist field. This requires using "init:
horizon" to detect start of an experiment, to allow parsing options
(e.g. ACTION_HISTORY) that are not yet available from the initial
argument parsing messages
 20230615 Add lstm field.

`;

sub read_list
{
  my($IN, $l, $tup, $fil);
  undef %g_inp_seen;
  @g_ptupl = ();
  @g_pfile = ();
  $g_nlist = 0;
  open($IN, $lfname);
  while($l =<$IN>) {
    chomp $l;
    if ($l =~ m|^    ([^ ]+) ([^ ]+)$|) {
      $tup = $1; $fil = $2;
      if ($fil < $g_max_keep) {
        $g_inp_seen{$fil} = 1;
        $g_pfile[$g_nlist] = $fil;
        $g_ptupl[$g_nlist] = $tup;
        $g_nlist++;
      }
    } else {
      die "read_list parse error:\n  $l\n";
    }
  }
  close $IN;
  print "read_list: got $g_nlist items\n";
  if ($g_max_keep < 19770234) {
    print "  (probably because g_max_keep == $g_max_keep for a full rescan)\n";
  }
} # End of read.list

sub write_list
{
  my($i, $OUT);
  open($OUT, "> $lfnew");
  for($i=0; $i<$g_nlist; $i++) {
    print $OUT "    $g_ptupl[$i] $g_pfile[$i]\n";
  }
  close $OUT;
} # End of write.list

sub cn_prog  # canonical names of our test programs
{
  my($pr) = @_;
  if ($pr eq 'adpcm') { $pr = 'adpcm-chs'; }
  if ($pr eq 'mmult') { $pr = 'mmult-legup'; }
  if ($pr eq 'sor')   { $pr = 'sor-caad'; }
  return $pr;
} # End of cn.prog

sub report_ghc
{
  if ($prev_tuple ne '') {
    if (($prev_iter > 10) && ($n_ghc >= 200)) {
      $itm2 = "    $prev_tuple $prev_file";
      print $LIST "$itm2\n";
      print STDERR "    $prev_tuple $prev_file $n_ghc gHC\n";
    }
  }
  $prev_tuple = $cur_tuple; # For us to use next time
  $prev_file = $cur_file;
  $prev_iter = $iter;
  $n_ghc = 0; # reset this counter for next experiment
} # End of report.ghc

# Scan a single file, find experiments and remember their tuples
sub scan1
{
  my($d, $f) = @_;
  my($inp, $IN, $l, $prog, $var, $seed);

  $inp = "$d/$f";
  # print STDERR "scanning input: $inp\n";
  if ($inp =~ m/\.gz$/) {
    open($IN, "cat $inp | gunzip |");
  } else {
    open($IN, $inp);
  }

  while ($l = <$IN>) {
    if ($l =~ m/^RLBASE_PROG=(.+)$/) {
      $prog = &cn_prog($1);
      $agent = 'ppo'; # default for older logs
      $ahist = 'hgram'; # default for older logs
      $lstm = 'mlp'; # default for older logs
    } elsif ($l =~ m/^RLBASE_VARIANT=(.+)$/) {
      $var = $1;
    } elsif ($l =~ m/^RLBASE_SEED=(\d+)$/) {
      $seed = $1;
      # print "% === seed $g_seed\n";
    } elsif ($l =~ m/^RLBASE_N_ITER=(\d+)$/) {
      $iter = $1;
    } elsif ($l =~ m/^RLBASE_AGENT_TYPE=([a-z0-9]+)$/) {
      $agent = $1;
    } elsif ($l =~ m/init: act_hist True/) {
      $ahist = 'ahist';
    } elsif ($l =~ m/init: use_lstm True/) {
      $lstm = 'lstm';
    }

    if ($l =~ m|init: horizon|) {
      $cur_tuple = "$prog:$agent:$lstm:$var:s$seed:i$iter:$ahist"; # %%% prog:$agent:$var:...
      $cur_file = $f; # For use by report.ghc
      if ($iter > 10) {
        $item = "    $cur_tuple $f";
#        print STDERR "$item\n";
#        print $LIST "$item\n";
      }
      &report_ghc(); # Resets n_ghc
    }

    if ($l =~ m/make accelerationCycle/) {
      $n_ghc++;
    }
  }
  close $IN;
  &report_ghc();
  $prev_tuple = ''; # Prevents report.ghc from repeating itself
} # End of scan.1

sub scan2
{
  my($d, $DIR, $f);

  &read_list();
  &write_list();
  undef %newfiles;
  open($LIST, ">> $lfnew");
  $d = "../logs";
  opendir($DIR, $d);
  while ($f = readdir($DIR)) {
    if ($g_inp_seen{$f}) {
      # We got the entries for this file
    } elsif ($f < $g_max_keep) {
      # Ignore these old files, don't scan
    } elsif (&path_age("$d/$f") < 36) { # 3600
      # Possibly still running, skip it
    } elsif ($f =~ m|^202.+[a-z]+\.txt|) {
      $newfiles{$f} = 1;
    }
  }
  closedir($DIR);
  foreach $f (sort (keys %newfiles)) {
    # print STDERR "scan1($f)\n";
    &scan1($d, $f);
  }
  close $LIST;
} # End of scan.2


$| = 1;

$g_max_keep = "19700101.000000"; # 99999999.999999
while ($arg = shift) {
  if (0) {
  } elsif ($arg =~ m/^--?h(elp)?$/) {
    print $help;
    exit(0);
  } elsif ($arg =~ m/^([.0-9]+)$/) {
    $g_max_keep = $arg;

  } else {
    die "unregognised argument '$arg', or file '$arg.c' does not exist\n";
  }
}

&scan2();

print qq@

Now run the following command to see what got added to the list:

  diff $lfname $lfnew

If the changes are acceptable, run this command to keep the updated list:

  mv $lfnew $lfname

@;
