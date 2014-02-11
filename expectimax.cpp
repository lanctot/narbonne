#include "search.hpp"

#include "util.hpp"

// node types: 0 = max, 1 = chance, 2 = min, 3 = chance 

double expectimax(GameState & gs, int depth, bool chanceNode, Move & mv, int color, int level);
static size_t playerId;
static size_t topLevelMovesDone;

double chanceNodeExp(GameState & gs, int depth, int color, int level) { 
  
  // get the frequency of the tiles to compute the probabilities of each outcome
  std::vector<size_t> freq; 
  std::vector<size_t> order; 
  size_t total = 0;
  gs.getChanceDist(freq, total, order); 

  size_t numOutcomes = 0;
  double expValue = 0; 
  assert(total > 0); 

  for (size_t oi = 0; oi < order.size(); oi++) { 
    size_t idx = order[oi];
    if (freq[idx] > 0) { 
      size_t replaceIdx = gs.setNextTile(idx*4); 
      double prob = static_cast<double>(freq[idx]) / static_cast<double>(total); 
      Move mv;
      double childVal = expectimax(gs, depth, false, mv, color, level+1);
      gs.undoSetNextTile(replaceIdx);  
      expValue += prob*childVal; 

      numOutcomes++;
      if (MaxChanceOutcomes > 0 && numOutcomes >= MaxChanceOutcomes)
        break;
    }
  } 
  
  return expValue;
}

double expectimax(GameState & gs, int depth, bool chanceNode, Move & mv, int color, int level) {

  if (depth == 0 || gs.gameOver())  {
    return gs.utility(playerId)*color; 
  }

  SearchGbl::nodesExpanded++;

  if (chanceNode) 
    return chanceNodeExp(gs, depth, color, level); 

  double maxVal = -1000;
  Move maxMove;
  Move m; 

  MoveIterator mi = gs.moveIterator();

  while (mi.next(m)) {

    gs.makeMove(m);
    
    Move cmv;
    double childVal = -expectimax(gs, depth-1, true, cmv, -color, level+1); 
    
    gs.undoMove();

    if (level == 0)
      topLevelMovesDone++;
   
    if (childVal > maxVal) {
      maxVal = childVal;
      maxMove = m; 
    }
    
    if (timeout())  // at the top   
    {
      mv = maxMove;
      return maxVal;
    }
  }  

  //if (depth == 2) std::cerr << "max move = " << maxMove << std::endl;

  mv = maxMove;
  return maxVal;
}
 
Move expectimax(GameState & gs, size_t myPlayerId, std::string runname) {
  Move mv(1,1,1,1);

  std::cerr << "Running negamax.. player = " << gs.turn() << ", tiles left = " << gs.tilesRemaining() << std::endl;
  playerId = myPlayerId;

  unsigned int startms = currentMillis();

  double val = 0.0;
  Move maxMove;

  startTimer(); 
  int maxDepth = (TimeoutsEnabled ? 100 : MaxDepth);

  // do iterative deepening, even if nothing from previous searches is used. why?
  //   1. needed to compare nodesExpanded stats with star1, star2
  //   2. should be treated an "anytime algorithm" like the others
  for (int depth = 1; depth <= maxDepth; depth++)
  {
    topLevelMovesDone = 0;

    std::cerr << "Running negamax from root depth = " << depth << std::endl;
    val = expectimax(gs, depth, false, mv, 1, 0);

    if (!TimeoutsEnabled || (TimeoutsEnabled && !timeout()))
    {
      maxMove = mv;
    }
    else if (TimeoutsEnabled && timeout())
    {
      std::cerr << "client timed out.. "; 

      if (depth == 1) 
      { 
        // can timeout here, esp at the end, when there's a small timeout setting
        maxMove = mv;
      }
      
      if (depth >= 2 && topLevelMovesDone > 1) 
      {
        maxMove = mv;
        std::cerr << " using move from current depth.";
      }
      else
      {
        std::cerr << " using move from previous depth.";
      }
      
      std::cerr << std::endl;

      break;
    }
  }
  
  unsigned int endms = currentMillis();
  
  unsigned int delta = endms - startms;
  SearchGbl::sc.push(delta); 
  double nps = SearchGbl::nodesExpanded / (SearchGbl::sc.sum() / 1000.0);
  std::cerr << "infoline: node val = " << val << ", time taken = " << delta << " ms; average = " << SearchGbl::sc.mean() << " ";
  std::cerr << "total nodes expanded = " << SearchGbl::nodesExpanded << ", avg nps = " << nps << std::endl;

  return maxMove;
}

