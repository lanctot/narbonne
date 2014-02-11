#!/usr/bin/perl -w

use Tk;
use Tk::widgets qw/JPEG PNG/;
use strict;

my $TW = 62;
my $TH = 62;

my $mw = MainWindow->new;

my $scrollbar = $mw->Scrollbar( -orient => "horizontal" );
my $canvas = $mw->Canvas(
    -width  => 1200,
    -height => 820,
    -xscrollcommand => ['set' => $scrollbar],
);
$scrollbar->configure(-command => ['xview' => $canvas]);
$scrollbar->pack(-side => 'bottom', -fill => 'y');

# Create the vertical Scrollbar
#my $lb = $mw->Listbox(-xscrollcommand => ['set' => $scrollbar]);
#Configure the Scrollbar to talk to the Listbox widget
#$scrollbar->configure(-command => ['xview' => $canvas]);

#Pack the Scrollbar first so that it doesn't disappear when we resize
#$scrollbar->pack(-side => 'bottom', -fill => 'y');
#$lb->pack(-side => 'left', -fill => 'both');

my @parts = ();
my @move = ();

my @statelines = ();
my @movelines = ();
my $index = 0;

my $first = 1;
my $rows = 0;
my $cols = 0;
my $score1 = 0;
my $score2 = 0;
my $meeples1 = 0;
my $meeples2 = 0;
my $turn = 0;
my $tiles_remaining = 0;
my $player = 0;

my $infolabel;

my $file = "game.log";
if (defined($ARGV[0])) { 
  $file = $ARGV[0];
}
print "File is $file\n";

open(FILE, '<', "$file") or die "Cannot open $file"; 

sub getnextline {
 
  my $mline = ""; 
  my $sline = ""; 

  if ($index == 0 && scalar(@movelines) == 0) {
    $mline = "";
    push(@movelines, $mline);
  }
  elsif ($index >= scalar(@movelines)) 
  {
    $mline = <FILE>;
    chomp($mline);
    push(@movelines, $mline);
  }
  elsif ($index < scalar(@movelines))
  {
    $mline = $movelines[$index];
  }

  if ($index >= scalar(@statelines))
  {
    $sline = <FILE>;
    chomp($sline);
    push(@statelines, $sline);
  }
  elsif ($index < scalar(@statelines))
  {
    $sline = $statelines[$index];
  }

  return ($mline, $sline);
}

sub parsenextmove {
  
  my ($moveline, $stateline) = getnextline();

  if ($index > 0) {
    @move = split(' ', $moveline); 
    #print "parsed move: $moveline\n";
  }
  @parts = split(' ', $stateline); 

  $score1 = $parts[0];
  $score2 = $parts[1];
  $meeples1 = $parts[2];
  $meeples2 = $parts[3];
  $turn = $parts[4];
  $rows = $parts[5];
  $cols = $parts[6];
  $tiles_remaining = $parts[8];
  $player = $parts[4];
 
  #print "parsed state: $stateline\n";
   
  $index++;
}

sub parsenext10moves {
  for (my $i = 0; $i < 10; $i++) { 
    parsenextmove();
  }
}

sub undomove {

  $index--; 
  $index--; 
  print "Undoing, index = $index\n";

  my ($moveline, $stateline) = getnextline();

  if ($index > 0) {
    @move = split(' ', $moveline); 

    # enable to debug
    # print "parsed move: $moveline\n";
  }
  @parts = split(' ', $stateline); 

  $score1 = $parts[0];
  $score2 = $parts[1];
  $meeples1 = $parts[2];
  $meeples2 = $parts[3];
  $turn = $parts[4];
  $rows = $parts[5];
  $cols = $parts[6];
  $tiles_remaining = $parts[8];
  $player = $parts[4];
 
  # enable to debug
  #print "parsed state: $stateline\n";
   
  $index++;

}

sub drawmeta {

  $mw->title("Carcassonne Replay");
 
  #$mw->Checkbutton(-text => "I like it!")->pack;
  #$mw->Checkbutton(-text => "I hate it!")->pack;
  #$mw->Checkbutton(-text => "I don't care")->pack;
  my $b1 = $mw->Button(-text => "Next",
                      -command => sub { parsenextmove(); draw(); } )->pack(-side => 'left', -anchor => 'n');
  
  my $b2 = $mw->Button(-text => "Next 10",
                       -command => sub { parsenext10moves(); draw(); } )->pack(-side => 'left', -anchor => 'n');
  
  my $b3 = $mw->Button(-text => "Prev",
                       -command => sub { undomove(); draw(); } )->pack(-side => 'left', -anchor => 'n');

  my $b4 = $mw->Button(-text => "Exit",
                       -command => sub { exit })->pack(-side => 'left', -anchor => 'n');

  #$b1->grid(-row => 1, -column => 1); 
  #$b2->grid(-row => 1, -column => 2); 
  #$b3->grid(-row => 1, -column => 3); 
  #$b4->grid(-row => 1, -column => 4); 

  $infolabel = $mw->Label( -text => 
    "Whose turn: P$turn             " 
  . "P1 score: $score1           P2 score: $score2            tiles: $tiles_remaining            " 
  . "P1 meeples: $meeples1           P2 meeples: $meeples2") -> pack();              

}

sub draw {
    
  $canvas->delete('all');

  $infolabel->configure(   -text => 
    "Whose turn: P$turn            " 
  . "P1 score: $score1           P2 score: $score2            tiles: $tiles_remaining            " 
  . "P1 meeples: $meeples1           P2 meeples: $meeples2" );

  # draw the board first

  my $idx = 9;
  for (my $r = 1; $r <= $rows; $r++) {
    for (my $c = 1; $c <= $cols; $c++) {
      if ($parts[$idx] >= 0) { 
        my $n = $parts[$idx];
        
        my $xcoord = $r*$TH;
        my $ycoord = $c*$TW + 15;

        my $im = $canvas->Photo(-file => "images/small-tile$n.png");

        $canvas->create(
          'image',
          $ycoord,
          $xcoord, 
          -anchor => 'nw', 
          -image  => $im,
        );

      }

      $idx++;
    }
  }

  # now the meeples

  $idx = 9 + $rows*$cols + $tiles_remaining;
  for (my $m = 0; $m < 14-$meeples1-$meeples2; $m++) { 
    my $r = $parts[$idx]; $idx++;
    my $c = $parts[$idx]; $idx++;
    my $pos = $parts[$idx]; $idx++;
    my $player = $parts[$idx]; $idx++;

    my $xcoord = $r*$TH;
    my $ycoord = $c*$TW + 15;
    
    my $im = $canvas->Photo(-file => "images/mp$player" . "_" . "$pos.png");
        
    $canvas->create(
      'image',
      $ycoord,
      $xcoord, 
      -anchor => 'nw', 
      -image  => $im,
    );

  }

  # now the move

  if ($index > 1)
  {
    my $im2 = $canvas->Photo(-file => "images/small-highlight.png");

    my $mrow = $move[0];
    my $mcol = $move[1];
    my $mori = $move[2];
    my $mpos = $move[3];
       
    if ($mrow == 0) { $mrow++; }
    if ($mcol == 0) { $mcol++; }

    my $xcoord = $mrow*$TH;
    my $ycoord = $mcol*$TW + 15;
  
    $canvas->create(
        'image',
        $ycoord,
        $xcoord, 
        -anchor => 'nw', 
        -image  => $im2,
    );
  }

  $first = 0; 

  #$canvas->configure(-scrollregion => [ $canvas->bbox("all") ]);

  $canvas->pack;
}

drawmeta();
parsenextmove();
draw();

MainLoop;


