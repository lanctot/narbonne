#ifndef __SEARCH_HPP__
#define __SEARCH_HPP__

#include "tiles.hpp"
#include "rules.hpp"
#include "statcounter.hpp"
#include "util.hpp"

// move order
// 18 == no meeple
// 20 == previous move from shallow search

extern bool TimeoutsEnabled;
extern unsigned int TimePerTurn;
extern int MaxDepth;
extern size_t MaxChanceOutcomes;
extern size_t SampleWidth;
extern bool PersistentTpTable;

class SearchGbl 
{
  public:
    /* Move order */
    static int order[6];

    static StatCounter sc; 
    static unsigned long long nodesExpanded;

    static StatCounter cbfCounter;
    static StatCounter nbfCounter;
};

// timer functions
void startTimer(); 
bool timeout(); 

// Globals used by the different search techniques
extern unsigned long long nodesExpanded;
extern StatCounter sc; 
void setMaxChanceOutcomes(size_t mco);

Move expectimax(GameState & gs, size_t myPlayerId, std::string runname);


#endif

