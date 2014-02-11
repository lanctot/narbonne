#include "search.hpp"

#include "util.hpp"

/* move order */
int SearchGbl::order[6] = { 20, CITY, CLOISTER, ROAD, 18, FARM }; 
unsigned long long SearchGbl::nodesExpanded = 0;
StatCounter SearchGbl::sc; 
StatCounter SearchGbl::cbfCounter; 
StatCounter SearchGbl::nbfCounter; 

int MaxDepth = 2;
bool TimeoutsEnabled = true;
unsigned int start_time = 0;
unsigned int TimePerTurn = 1000;

// 0 means disable max chance outcomes
size_t MaxChanceOutcomes = 0; 

void startTimer()
{
  start_time = currentMillis();
}

bool timeout()
{
  if (TimeoutsEnabled)
    return (currentMillis() - start_time > TimePerTurn); 
  else 
    return false;
}

void setMaxChanceOutcomes(size_t mco)
{
  MaxChanceOutcomes = mco;
}

