
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "rules.hpp"
#include "util.hpp"
#include "search.hpp"

using namespace std; 

static unsigned int seed;
static bool seedSpecified = false;
static size_t playerid = 0;
static string runname = "default";

static void getOptions(int argc, char * argv[]) {

  if (argc < 3) {
    cerr << "Usage: ./expm_ai <player id> [-rn runname] [-seed seed] " 
              << "[-timeout milliseconds] [-maxdepth d]" << endl;
    exit(-1);
  }

  string str = argv[1];
  playerid = to_size_t(str); 
  assert(playerid >= 1 && playerid <= 2); 

  for (int i = 2; i < argc; i++) {
    string argstr = argv[i];
    if (argstr == "-seed") { seed = atoi(argv[++i]); seedSpecified = true; }
    else if (argstr == "-timeout") { TimeoutsEnabled = true; TimePerTurn = atoi(argv[++i]); }
    else if (argstr == "-maxdepth") { MaxDepth = atoi(argv[++i]); }
    else if (argstr == "-rn") { runname = argv[++i]; }
    else { cerr << "unrecognized cmdline arg: " << argstr << endl; exit(-1); }
  }

}

bool parseState(GameState &g) { 

  string buf;
  if (!getline(cin, buf)) return false;
  if (buf == "") return false;

  // build the state here; unneeded for the simple agent 
  return parseCarcState(buf, g); 
}

/* application entry point */
int main(int argc, char * argv[]) {

  getOptions(argc, argv);

  if (seedSpecified) 
    seed_rng(seed); 
  else
	  seed_rng(currentMillis()); 
  
  createTiles(); 

  GameState s; 
  bool val = parseState(s);
  
  while (val) {
    cerr << "Client received state from server." << endl;

    Move mv; 

    mv = expectimax(s, playerid, runname); 

    cerr << "Client sending move: " << mv.protocolText() << endl << endl;
    
    cout << mv.protocolText() << endl;
    
    GameState gs; 
    val = parseState(gs); 
    s = gs;
  }

  destroyTiles();
  return 0;
}
