
To build the code and run simulations, you will need: g++, cmake, make, and perl. 
To watch the replays, you will also perl-tk and the images in an images/ subdir.

These instructions assume a Linux shell, but should work in any OS with some
slight modifications. The code itself has not been tested on Windows or MacOS.

If you're building the code for the first time, run: 

  cmake .

To build the code any time afterward:

  make

To set build options or add new executables, see CMakeFiles.txt. To completely 
rebuild from scratch: rm -rf CMakeCache.txt CMakeFiles

Before running anything, you will need a subdirectory called scratch: 

  mkdir scratch

There are two sample players: 
  - a random player (simple.cpp) 
  - an expectimax player (mostly expm_ai.cpp & expectimax.cpp; some search.cpp)

To run a game, use scripts/sim2p.perl:

  Run it by itself to see the parameters. Here are some examples:

  Random vs. Random, runname = test1
    scripts/sim2p.pl "./simple" "./simple" test1

  Expectimax (1 second search time) vs. Random, runname = test2
    scripts/sim2p.pl "./expm_ai 1 -rn test2 -timeout 1000" "./simple" test2

sim2p.pl will output the following files: 
  scratch/<runname>.agent1.err      (player 1's debug output)
  scratch/<runname>.agent2.err      (player 2's debug output)
  scratch/<runname>.server.err      (server's debug output)
  scratch/<runname>.game.log        (a log of the game to be used for replay)

The result of the game is at the end of the server output. 

To step through a game via a Tk GUI once it is finished, run:
  scripts/replay.pl scratch/<runname>.game.log


