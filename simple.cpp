
/**
 * A simple bot that plays a legal move (uniformly) at random. 
 */

#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

#include "rules.hpp"
#include "util.hpp"

bool parseState(GameState &g) { 

	std::string buf;
	if (!std::getline(std::cin, buf)) return false;
	if (buf == "") return false;

  std::cerr << "Client received: " << buf << std::endl;

	// build the state here; unneeded for the simple agent 
  return parseCarcState(buf, g); 
}

/* application entry point */
int main(int argc, const char *argv[]) {

  createTiles(); 
	GameState s; 
	std::srand(currentMillis()); 

	while (parseState(s)) {

    std::cerr << "Client's state is: " << s.agentProtocolStr() << std::endl;

    movelist_t pml;
		s.genPlacements(pml); 

    movelist_t ml;

    for (size_t idx = 0; idx < pml.size(); idx++) {
      
      size_t rp = pml[idx].row();
      size_t cp = pml[idx].col();
      size_t ori = pml[idx].ori(); 

      Move mv0(rp, cp, ori, 18); 
      ml.push_back(mv0); 

      for (size_t pos = 0; pos <= 16; pos++) {
        if (s.legalMove(rp, cp, ori, pos)) {
          Move mv(rp, cp, ori, pos); 
          ml.push_back(mv); 
        }
      }
    }

    assert(ml.size() > 0); 

    std::cerr << "Client's available moves:" << std::endl;
    for (size_t i = 0; i < ml.size(); i++) 
      std::cerr << ml[i].protocolText() << std::endl;
   
    size_t idx = rand() % ml.size(); 
    std::cerr << "Client sending " <<  ml[idx].protocolText() << std::endl;
    std::cout << ml[idx].protocolText() << std::endl;
	}

	return 0;
}

