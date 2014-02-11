#!/usr/bin/perl
use Carp;
use strict;
use IPC::Open3;
use vars qw( $debug );
$debug = 1;

# The first parameter is how many jobs to run at once, the remaining are
# the jobs.  Jobs may be a string, or an anonymous array of the cmd and
# args.
#
# All output from children go to your STDERR and STDOUT.  They get no
# input.  It prints fairly uninformative errors for commands with
# problems, and returns a hash of problems.
#
# The jobs SHOULD NOT depend on each other!
sub run_parallel {
  my $job_count = shift;
  unless (0 < $job_count) {
    confess("run_parallel called without a positive parallel job count!");
  }
  my @to_start = @_;
  my %running;
  my %errors;
  my $is_running = 0;
  while (@to_start or %running) {
    if (@to_start and ($is_running < $job_count)) {
      # Launch a job
      my $job = shift @to_start;
      unless (ref($job)) {
        $job = [$job];
      }
      print "Launching '$job->[0]'\n" if $debug;
      local *NULL;
      my $null_file = ($^O =~ /Win/) ? 'nul': '/dev/null';   
      open (NULL, $null_file) or confess("Cannot read from $null_file:$!");
      my $proc_id = open3("<&NULL", ">&STDOUT", ">&STDERR", @$job);
      $running{$proc_id} = $job;
      ++$is_running;
    }
    else {
      # collect a job
      my $proc_id = wait();
      if (! exists $running{$proc_id}) {
        confess("Reaped unknown process $proc_id!");
      }
      elsif ($?) {
        # Oops
        my $job = $running{$proc_id};
        my ($cmd, @args) = @$job;
        my $err = "Running '$cmd' gave return code '$?'";
        if (@args) {
          $err .= join "\n\t", "\nAdditional args:", @args;
        }
        print STDERR $err, "\n";
        $errors{$proc_id} = $err;
      }
      print "Reaped '$running{$proc_id}->[0]'\n" if $debug;
      delete $running{$proc_id};
      --$is_running;
    }
  }
  return %errors;
}

sub get_cmd
{
  my $alg = shift;
  my $player = shift;
  my $seed = shift;

  my $algnum = 0;

  if ($alg eq "exp") { $algnum = 1; }
  elsif ($alg eq "star1") { $algnum = 2; }
  elsif ($alg eq "star2") { $algnum = 3; }
  else { print "Error: unrecognized algorithm: $alg."; return ""; }

  my $cmd = "";
  #my $cmd = "./expm_ai $player $algnum -seed 18309876 -timeout 30000"; 
  if ($seed > 0) {
    $cmd = "./expm_ai $player $algnum -seed $seed -timeout 10000"; 
  }
  else {
    $cmd = "./expm_ai $player $algnum -timeout 10000"; 
  }

  
  return $cmd;
}

my @jobs = (); 

#my @searches = ("mc", "mcts"); 
#my @sims = (16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536); 
#my @methods = ("vanilla", "crn", "cv", "cvcrn"); 

# 1 = exp, 2 = star1, 3 = star2
#my @algorithms = ("star2", "star1", "exp"); 
my @algorithms = ("star2", "star1"); 
my $gamespermatch = 1;  # actually it's double this number since swap seats

for (my $i = 0; $i < scalar(@algorithms); $i++)
{
  for (my $j = $i+1; $j < scalar(@algorithms); $j++)
  {
    my @seeds = (); 
    for (my $k = 0; $k < $gamespermatch; $k++) {
      my $seed = `rand 1 100000`;
      chomp($seed); 
      #print "seed = $seed\n";
      push(@seeds, $seed);
    }

    # a1 as player1 a2 as player 2
    for (my $run = 1; $run <= $gamespermatch; $run++)
    {
      my $seed = $seeds[$run-1];

      my $alg1 = $algorithms[$i];
      my $alg2 = $algorithms[$j];

      my $runname = "$alg1-$alg2-$run";

      my $cmd1 = get_cmd($alg1, 1, 0); 
      my $cmd2 = get_cmd($alg2, 2, 0); 

      #my $errfile = "/scratch/dom/$search-$method-$sim.err.txt";
      #my $outfile = "/scratch/dom/$search-$method-$sim.out.txt"; 

      my $fullcmd = "scripts/sim2p.pl \"$cmd1\" \"$cmd2\" $runname $seed >$runname.trace 2>$runname.trace";
      
      print "queueing $fullcmd\n";
      push(@jobs, $fullcmd); 

      $cmd1 = get_cmd($alg2, 1, 0); 
      $cmd2 = get_cmd($alg1, 2, 0); 

      # now its swapped counterpart
      $runname = "$alg2-$alg1-$run"; 
      $fullcmd = "scripts/sim2p.pl \"$cmd1\" \"$cmd2\" $runname $seed >$runname.trace 2>$runname.trace";
      
      print "queueing $fullcmd\n";
      push(@jobs, $fullcmd); 
    }
  }
}

# first parm is the number of concurrent processes
print "\n";
&run_parallel(8, @jobs);




