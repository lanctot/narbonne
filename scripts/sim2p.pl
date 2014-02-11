#!/usr/bin/perl
use IPC::Open2;

my $ofh = select STDOUT;
$| = 1;
select $ofh;

if (scalar(@ARGV) < 2) { 
  print "Usage: ./test2p.perl <p1 command> <p2 command> <optional runname>\n";
  exit;
}

my $p1cmd = $ARGV[0];
my $p2cmd = $ARGV[1];
my $runname = "default".$$;
if (scalar(@ARGV) >= 3) {
  $runname = $ARGV[2];
}

# check for scratch dir
if (-d "scratch") { } else { print "Please create a scratch subdirectory\n"; exit; }


$env_cmd = "./server -runname $runname 2>scratch/$runname.server.err";
$agent1_cmd = "$p1cmd 2>scratch/$runname.agent1.err";
$agent2_cmd = "$p2cmd 2>scratch/$runname.agent2.err";

print "run name = $runname\n";
print "agent 1 log = scratch/$runname.agent1.err\n";
print "agent 2 log = scratch/$runname.agent2.err\n";

#print "$env_cmd\n";
#print "$agent1_cmd\n";
#print "$agent2_cmd\n";

#$agent1_cmd = "./simple";
#$agent2_cmd = "./simple";
#$env_cmd = "valgrind --tool=memcheck ./server 2>server.err";
#$env_cmd = "./server 2>server.err";
#$env_cmd = "./server -ssfile game.dat 2>server.err";

#$agent1_cmd = "./simple 2>agent1.err";
#$agent1_cmd = "./expm_ai 1 2>agent1.err";
#$agent1_cmd = "./mcts_ai 1 2>agent1.err";
#$agent1_cmd = "valgrind --tool=memcheck --max-stackframe=4000000 ./simple 2>agent1.err";
#$agent1_cmd = "valgrind --tool=memcheck --max-stackframe=4000000 ./expm_ai 1 2>agent1.err";
#$agent1_cmd = "valgrind --tool=memcheck --max-stackframe=4000000 ./mcts_ai 1 2>agent1.err";

#$agent2_cmd = "./simple 2>agent2.err";
#$agent2_cmd = "./expm_ai 1 2>agent2.err";
#$agent2_cmd = "./mcts_ai 1 2>agent2.err";

#$env_cmd = "valgrind --tool=memcheck ./server 2>server.err";
#$agent1_cmd = "valgrind --tool=memcheck ./simple 2>agent1.err";
#$agent2_cmd = "valgrind --tool=memcheck ./simple 2>agent2.err";


# -----------------------------------
#            main loop
# -----------------------------------

local (*AGENT1_READ, *AGENT1_WRITE, *AGENT1_ERR);
local (*AGENT2_READ, *AGENT2_WRITE, *AGENT2_ERR);
local (*ENV_READ, *ENV_WRITE, *ENV_ERR);
$pid_env     = open2(\*ENV_READ, \*ENV_WRITE, $env_cmd);
$pid_agent1  = open2(\*AGENT1_READ, \*AGENT1_WRITE, $agent1_cmd);
$pid_agent2  = open2(\*AGENT2_READ, \*AGENT2_WRITE, $agent2_cmd);

$i = 0; 
$t = 1;

while (($l = <ENV_READ>) && !($l =~ /Score/)) {
    #$a = <AGENT_READ>;
    
    if ($i == 0)
    {
      print "#$t Player 1's turn... ";
      print AGENT1_WRITE $l;
      $a = <AGENT1_READ>; 
      $m = $a;
      chomp($m);
      print "read $m from agent 1\n";
      print ENV_WRITE $a;
    }
    elsif ($i == 1) 
    {
      print "#$t Player 2's turn... ";
      print AGENT2_WRITE $l;
      $a = <AGENT2_READ>; 
      $m = $a;
      chomp($m);
      print "read $m from agent 2\n";
      print ENV_WRITE $a;
    }

    $t++;
    $i++; if ($i >= 2) { $i = 0; }

    #sleep(1);
}

print "# Done, runname = $runname, game log = scratch/$runname.game.log\n";
    
# terminate agent/environment
print AGENT1_WRITE "\n";
print AGENT2_WRITE "\n";

waitpid($pid_env, 0); 
waitpid($pid_agent1, 0);
waitpid($pid_agent2, 0);

close ENV_READ; close ENV_WRITE; 
close AGENT1_READ; close AGENT1_WRITE;
close AGENT2_READ; close AGENT2_WRITE;

kill $pid_env, 9; 
kill $pid_agent1, 9;
kill $pid_agent2, 9;


