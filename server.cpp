#include "server.hpp"

#include "rules.hpp"
#include "util.hpp"
#include "tiles.hpp"

#include <ctime>
#include <cstdlib>
#include <sstream>
#include <cassert>
#include <fstream>

ServerParameters gbl_srvopt;

static bool loggame = true;
static std::ofstream logfile;

static bool processTurn(GameState &s, const std::string &input) {

  std::vector<std::string> parts;
  split(parts, input, ' '); 

  assert(parts.size() == 4);
  size_t row, col, ori, mpos;
  row = to_size_t(parts[0]); 
  col = to_size_t(parts[1]); 
  ori = to_size_t(parts[2]); 
  mpos = to_size_t(parts[3]);
  
  if (!s.legalMove(row, col, ori, mpos)) 
    return false;

  Move mv(row, col, ori, mpos);

  if (loggame)
    logfile << mv << std::endl;

  bool ret = s.makeMove(mv);

	return ret;
}


/* main game loop */
static void mainLoop() {

  createTiles(); 

  if (gbl_srvopt.seedSpecified) {
    unsigned int seed = gbl_srvopt.seed;
	  seed_rng(seed); 
  }
  else {
    unsigned int seed = currentMillis();
	  seed_rng(seed); 
  }

  std::string gamelogname = "scratch/" + gbl_srvopt.runname + ".game.log"; 

  if (loggame) { 
    logfile.open(gamelogname.c_str());
  }

	GameState s;

  s.reshuffleBag();

	std::string input;

  if (gbl_srvopt.ssfile != "") { 
    std::cerr << "Opening state from " << gbl_srvopt.ssfile << " ... ";
    std::ifstream in(gbl_srvopt.ssfile.c_str()); 
    if (!in.is_open()) { 
      std::cerr << "Error opening " << gbl_srvopt.ssfile << std::endl;
      exit(-1); 
    }

    if (!std::getline(in, input)) { 
      std::cerr << "Error reading line from: " << gbl_srvopt.ssfile << std::endl;
    }

    if (!parseCarcState(input, s)) {
      std::cerr << "Error parsing Carcassonne state from " << gbl_srvopt.ssfile << std::endl;
    }

    std::cerr << "success" << std::endl;

    in.close();
  }
  

	//if (!std::getline(std::cin, input)) return;

  if (loggame)
    logfile << s.agentProtocolStr() << std::endl;

	// get action
  int turns = 0; 

	while (!s.gameOver()) {
		
		std::cout << s.agentProtocolStr() << std::endl; 
		std::cerr << s;
    std::cerr << "Player to move: " << s.turn() << std::endl;
		std::cerr << "Action> " << std::endl;

		if (!std::getline(std::cin, input)) return;

    std::cerr << "Received: " << input << std::endl;
    turns++; 

		if (!processTurn(s, input)) {
			std::cerr << "Error in process turn!" << std::endl;
			return;
		}
  
    //'std::cerr << "Sending " << s.agentProtocolStr() << " back to client. " << std::endl;
    std::cerr << "Sending state back to client. " << std::endl;
    if (loggame)
      logfile << s.agentProtocolStr() << std::endl;

    std::cerr << std::endl;
	}

  if (loggame)
    logfile.close();

  size_t score1, score2;
  s.getScore(score1, score2); 
  
  int winner = 0; 
  if (score1 > score2) winner = 1;
  else if (score2 > score1) winner = 2;
  
  std::cerr << "infoline: " << score1 << " " << score2 << " " << s.utility() << " " << winner << std::endl;

	// display final score
	std::cout << "Score: " << s.utility() << std::endl;
	std::cerr << "Score: " << s.utility() << std::endl;

  // results file
  /*/std::string resultsfile = gbl_srvopt.runname + ".srvres.dat";
  std::ofstream results(resultsfile.c_str());
  results << score1 << " " << score2 << " " << s.utility() << " " << winner << std::endl;
  results.close();*/
}


/* application entry point */
int main(int argc, const char *argv[]) {

	bool rval = gbl_srvopt.init(argc, argv);
	if (!rval) {
		std::cerr << "invalid cmdline args..." << std::endl;
		std::exit(1);
	}
	
	// initialise random number generator
	if (gbl_srvopt.seed == 0) gbl_srvopt.seed = currentMillis();
	seed_rng(gbl_srvopt.seed); 

	mainLoop();

  destroyTiles();
	return 0;
}

