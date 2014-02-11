#include "rules.hpp"

#include <cstdlib>
#include <sstream>
#include <cassert>
#include <iostream>

#include "util.hpp"
#include "search.hpp"

using namespace std; 

// helper arrays for transitions
static std::vector<int> completedCities;
static bool marked[74][74][18];
static int cities[74][74][18];
static size_t tileMarkers[74*74];
static bool undoing = false;

static void clearMarked() { 
  for (size_t r = 0; r < 74; r++)
    for (size_t c = 0; c < 74; c++) 
      for (size_t i = 0; i < 18; i++)
        marked[r][c][i] = false;
}

void clearTilesMarkers() {
  for (size_t i = 0; i < (74*74); i++)
    tileMarkers[i] = 0;
}

size_t countTileMarkers(size_t m_rows, size_t m_cols) { 
  size_t sum = 0;
  for (size_t i = 0; i <= m_rows; i++) 
    for (size_t j = 0; j <= m_cols; j++) 
      sum += tileMarkers[i*m_rows + j]; 
  return sum;
}

static void clearCities() { 
  for (size_t r = 0; r < 74; r++)
    for (size_t c = 0; c < 74; c++) 
      for (size_t i = 0; i < 18; i++)
        cities[r][c][i] = 0;
}

// returns a string rep of this move
std::string Move::protocolText() {
  std::ostringstream oss; 
  oss << m_row << " " << m_col << " " << m_ori << " " << m_mpos; 
  return oss.str();
}

MoveIterator::MoveIterator(GameState & _gs) 
  : type(1), idx(0), legalMoveIndex(0), pos(0), gs(_gs)
{
  mpl.clear();
  gs.genPlacements(mpl); 
}

// get the next available move
bool MoveIterator::next(Move & mv) 
{
  for (; type < 6; type++) {
    for (; idx < mpl.size(); idx++) {

      size_t rp = mpl[idx].row();
      size_t cp = mpl[idx].col();
      size_t ori = mpl[idx].ori();
      size_t tileId = gs.firstTileId() + ori;
      assert(tileId >= 0 && tileId < 96); 
      Tile * tile = allTiles[tileId];

      if (SearchGbl::order[type] == 18)
      {
        Move m(rp, cp, ori, 18);
        mv = m;
        legalMoveIndex++;
        idx++;
        return true;
      }
      else 
      {
        for (; pos <= 16; pos++) {
          
          if (tile->getType(pos) != SearchGbl::order[type]) 
            continue;
          
          if (gs.legalMove(rp, cp, ori, pos)) 
          {
            Move m(rp, cp, ori, pos); 
            mv = m; 
            legalMoveIndex++; 
            pos++;             // make sure it points to the next one for next time
            return true;
          }
      
        }

        pos = 0; 
      }
    }

    idx = 0; 
  }

  return false;
}

MoveIterator GameState::moveIterator()
{
  return MoveIterator(*this);
}

unsigned long long GameState::hashVal() const {
  unsigned long long hval = 0; 

  cerr << "hashVal unimplemented" << endl;

  return hval;
}

GameState::GameState() { 
  createBag(m_bag);  
  m_meeplepos.clear(); 

  m_score1 = m_score2 = 0;
  m_meeples1 = m_meeples2 = 7;
  m_rows = m_cols = 1;
  m_turn = 1;

  for (size_t r = 0; r < 74; r++) 
    for (size_t c = 0; c < 74; c++) 
    {
      m_board[r][c] = -1; 

      for (size_t p = 0; p < 18; p++) 
        m_occupied[r][c][p] = false;
    }

  // starting tile
  m_board[1][1] = 12;

  m_bagIndex = 0;
  m_tilesRemaining = m_bag.size();

  m_deltas.clear();
}

bool GameState::legalChanceOutcome(int bagIdx) {
  Tile * save = m_bag[0];
  m_bag[0] = m_bag[bagIdx]; 

  bool ret = hasAtLeastOnePlacement();

  m_bag[0] = save;
  return ret;
}
 
bool GameState::hasAtLeastOnePlacement() {
  
  size_t tileId = m_bag[0]->getId(); 

  for (size_t r = 1; r <= m_rows; r++) 
    for (size_t c = 1; c <= m_cols; c++)
    {
      if (m_board[r][c] < 0) 
        continue;

      // north, east, south, west
      size_t rpset[4] = { r-1,   r, r+1, r   }; 
      size_t cpset[4] = {   c, c+1,   c, c-1 }; 

      for (size_t d = 0; d < 4; d++) {
        size_t rp = rpset[d];
        size_t cp = cpset[d];

        if (m_board[rp][cp] >= 0)
          continue;

        for (size_t rot = 0; rot < 4; rot++) {

          if (legalTilePlacement(tileId+rot, rp, cp)) 
            return true;
        }
      }
    }

  return false;
}

void GameState::getChanceDist(std::vector<size_t> & freq, size_t & total, std::vector<size_t> & order) {
  
  freq.clear(); 
  total = 0;
  freq.resize(TotalUniqueTileTypes); 
  
  for (size_t idx = 0; idx < TotalUniqueTileTypes; idx++) { 
    freq[idx] = 0;
  }

  for(size_t idx = 0; idx < m_bag.size(); idx++) { 
    if (legalChanceOutcome(idx)) {
      int tileId = m_bag[idx]->getId();
      int uniqId = tileId / 4; 
      assert(uniqId >= 0 && uniqId < TotalUniqueTileTypesInt); 
      freq[uniqId]++; 
      total++; 
    }
  }

  // order them by probability 
  order.clear();
  bool chosen[TotalUniqueTileTypes];
  for (size_t idx = 0; idx < TotalUniqueTileTypes; idx++) chosen[idx] = false;

  size_t max = 0;
  size_t maxidx = 0;
  do {
    max = maxidx = 0;

    for (size_t idx = 0; idx < TotalUniqueTileTypes; idx++) {
      if (!chosen[idx] && freq[idx] > max) { 
        max = freq[idx];
        maxidx = idx;
      }
    }

    if (max > 0) { 
      order.push_back(maxidx);
      chosen[maxidx] = true;
    }
  }
  while (max > 0);
}

size_t GameState::setNextTile(size_t tileId) {
  bool found = false; 
  size_t replacedIdx = 1000;

  for (size_t idx = 0; idx < m_bag.size(); idx++) { 
    if (m_bag[idx]->getId() == tileId) {
      found = true;
      Tile * tmp = m_bag[0]; 
      m_bag[0] = m_bag[idx]; 
      m_bag[idx] = tmp; 
      replacedIdx = idx;
      break;
    }
  }
  

  if (!found) 
    std::cerr << "setNextTile !found" << std::endl;
  assert(found);
  return replacedIdx;
}

void GameState::undoSetNextTile(size_t replaceIdx) {
  assert(replaceIdx < m_bag.size());

  Tile * tmp = m_bag[0]; 
  m_bag[0] = m_bag[replaceIdx]; 
  m_bag[replaceIdx] = tmp; 
}


    
void GameState::getScore(size_t & score1, size_t & score2) {
  score1 = m_score1;
  score2 = m_score2;
}
    
void GameState::removeMeeple(size_t row, size_t col, size_t mpos) {
  mposlist_t::iterator iter;
  for (iter = m_meeplepos.begin(); iter != m_meeplepos.end(); iter++) {
    if (iter->row == row && iter->col == col && iter->pos == mpos) 
    {
      if (!undoing) {
        MeeplePos mp; 
        mp.row = iter->row; 
        mp.col = iter->col;
        mp.pos = iter->pos; 
        mp.player = iter->player;
        m_curDelta.meeplePos.push_back(mp); 
        m_curDelta.meeplePosAdded.push_back(false); 
      }

      m_meeplepos.erase(iter);
      break;
    }
  }
}

int GameState::meepleOnTile(size_t row, size_t col, size_t & player) const {
  for (size_t idx = 0; idx < m_meeplepos.size(); idx++) {
    if (m_meeplepos[idx].row == row && m_meeplepos[idx].col == col) { 
      player = m_meeplepos[idx].player;
      return m_meeplepos[idx].pos;
    }
  }

  player = 0;
  return -1; 
}


void GameState::scoreMonk(size_t row, size_t col, bool endPartPoints) {
  
  // monk is always in the middle
  int tileId = m_board[row][col]; 
  if (tileId == -1) return;

  Tile * tile = allTiles[tileId]; 
  if (tile->getType(MID) == CLOISTER) { 
    int mplayer = meeplePlayer(row, col, MID); 

    if (mplayer > 0) { 

      // check all 9 squares 
      if (!endPartPoints
            && m_board[row-1][col-1] >= 0 && m_board[row-1][col] >= 0 && m_board[row-1][col+1] >= 0
            && m_board[row][col-1] >= 0   && m_board[row][col] >= 0   && m_board[row][col+1] >= 0
            && m_board[row+1][col-1] >= 0 && m_board[row+1][col] >= 0 && m_board[row+1][col+1] >= 0)
      {
        // score! 
        size_t & score = (mplayer == 1 ? m_score1 : m_score2); 
        size_t & meeples = (mplayer == 1 ? m_meeples1 : m_meeples2); 

        score += 9; 
        meeples += 1;

        removeMeeple(row, col, MID); 
      }
      else if (endPartPoints) 
      {
        size_t & score = (mplayer == 1 ? m_score1 : m_score2); 
        size_t & meeples = (mplayer == 1 ? m_meeples1 : m_meeples2); 

        if (m_board[row-1][col-1] >= 0) score++;
        if (m_board[row-1][col] >= 0)   score++;
        if (m_board[row-1][col+1] >= 0) score++;
        if (m_board[row][col-1] >= 0)   score++;
        if (m_board[row][col] >= 0)     score++;
        if (m_board[row][col+1] >= 0)   score++;
        if (m_board[row+1][col-1] >= 0) score++;
        if (m_board[row+1][col] >= 0)   score++;
        if (m_board[row+1][col+1] >= 0) score++;

        meeples += 1;
        removeMeeple(row, col, MID); 
      }
    }
    
  }

}

bool GameState::immediateScoring_rec(size_t row, size_t col, size_t mpos, int type, 
  std::vector<MeeplePos> & meeplepos1, std::vector<MeeplePos> & meeplepos2, 
  bool crossingBoundary, bool endPartPoints) {

  if (mpos == END) return true;
  if (!endPartPoints && !m_occupied[row][col][mpos]) return false;

  int tileId = m_board[row][col]; 
  
  if (endPartPoints && tileId < 0) 
    return true; 
  
  marked[row][col][mpos] = true;

  Tile * tile = allTiles[tileId];

  if (crossingBoundary) {
    int idx = row*m_rows + col; 

    if (type == CITY && tile->hasShield())
      tileMarkers[idx] = 2; 
    else
      tileMarkers[idx] = 1; 
  }
    
  // check for a meeple at this location
  size_t mplayer = meeplePlayer(row, col, mpos);
  if (mplayer > 0) {
    MeeplePos mp; 
    mp.row = row;
    mp.col = col; 
    mp.pos = mpos; 
    mp.player = mplayer;

    if (mplayer == 1) 
      meeplepos1.push_back(mp);
    else if (mplayer == 2) 
      meeplepos2.push_back(mp);
  }

  // try connections on this tile first
  
  for (size_t pos = 0; pos < 18; pos++) {

    // no self-loops
    if (pos == mpos)
      continue;

    if ((pos <= 16 && !marked[row][col][pos]) || pos == 17)
    {
      int desttype = tile->getType(mpos, pos);
    
      // is it a connection of the same type?
      if (desttype == type) { 
      
        // now check the internal connections

        if (pos == END) 
        {
          // done recursion
        }
        else 
        {
          // keep extending the snake
          marked[row][col][pos] = true;
          bool closed = immediateScoring_rec(row, col, pos, type, meeplepos1, meeplepos2, false, endPartPoints);
          if (!closed)
            return false;
        } 
      }
    }
  }

  // now try possible neighbors
  
  // north
  if (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row-1][col][newmpos]) {
      if (m_board[row-1][col] < 0 && !endPartPoints) return false;
      marked[row-1][col][newmpos] = true;
      bool closed = immediateScoring_rec(row-1, col, newmpos, type, meeplepos1, meeplepos2, true, endPartPoints);
      if (!closed) return false;
    }
  }
    
  // east
  if (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row][col+1][newmpos]) {
      if (m_board[row][col+1] < 0 && !endPartPoints) return false; 
      marked[row][col+1][newmpos] = true;
      bool closed = immediateScoring_rec(row, col+1, newmpos, type, meeplepos1, meeplepos2, true, endPartPoints);
      if (!closed) return false;
    }
  }

  // south
  if (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
  {
    size_t newmpos = flipped(mpos);
    
    if (!marked[row+1][col][newmpos]) {
      if (m_board[row+1][col] < 0 && !endPartPoints) return false; 
      marked[row+1][col][newmpos] = true;
      bool closed = immediateScoring_rec(row+1, col, newmpos, type, meeplepos1, meeplepos2, true, endPartPoints);
      if (!closed) return false;
    }
  }
   
  // west 
  if (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
  {
    size_t newmpos = flipped(mpos);
    if (!marked[row][col-1][newmpos]) {
      if (m_board[row][col-1] < 0 && !endPartPoints) return false; 
      marked[row][col-1][newmpos] = true;
      bool closed = immediateScoring_rec(row, col-1, newmpos, type, meeplepos1, meeplepos2, true, endPartPoints);
      if (!closed) return false;
    }
  }

  // if we've exhausted everything, we have a self loop. considered closed!
  return true;
}

void GameState::getNeighborCoords(size_t row, size_t col, size_t mpos, size_t & rowp, size_t & colp) {
  
  // north
  if (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
  { rowp = row-1; colp = col; return; }
    
  // east
  if (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
  { rowp = row; colp = col+1; return; }

  // south
  if (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
  { rowp = row+1; colp = col; return; }
   
  // west 
  if (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
  { rowp = row; colp = col-1; return; }
}

// row, col is the newly placed tile. called after reposition
void GameState::immediateScoring(size_t row, size_t col, bool endPartPoints) {

  int tileId = m_board[row][col]; 
  Tile * tile = allTiles[tileId];

  // check: if there is only 1 neighbor and no end-points, we can skip this
  size_t neighbors = 0; 
  if (m_board[row-1][col] >= 0) neighbors++;
  if (m_board[row+1][col] >= 0) neighbors++;
  if (m_board[row][col-1] >= 0) neighbors++;
  if (m_board[row][col+1] >= 0) neighbors++;
  if (neighbors == 1) { 
    bool hasEnding = false;
    for (size_t pos = 0; pos <= 15; pos++) {
      int contype = tile->getType(pos, END);
      if (contype == ROAD || contype == CITY) {
        hasEnding = true; break;
      }
    }
    // if we get here, no immediate scoring needed.
    if (!hasEnding) return;
  }

  if (m_board[row+1][col+1] >= 0) neighbors++; 
  if (m_board[row+1][col-1] >= 0) neighbors++; 
  if (m_board[row-1][col-1] >= 0) neighbors++; 
  if (m_board[row-1][col+1] >= 0) neighbors++; 
  if (neighbors >= 3)
  {
    // score monks if possible
    scoreMonk(row-1, col-1, endPartPoints); 
    scoreMonk(row-1, col, endPartPoints); 
    scoreMonk(row-1, col+1, endPartPoints); 
    scoreMonk(row, col-1, endPartPoints);   
    scoreMonk(row, col, endPartPoints);   
    scoreMonk(row, col+1, endPartPoints); 
    scoreMonk(row+1, col-1, endPartPoints); 
    scoreMonk(row+1, col, endPartPoints); 
    scoreMonk(row+1, col+1, endPartPoints); 
  }

  clearMarked(); 


  for (size_t pos = 0; pos <= 16; pos++) {
     
    if (!m_occupied[row][col][pos]) continue;

    int type = tile->getType(pos); 
   
    if ((type == ROAD || type == CITY) && !marked[row][col][pos]) 
    {
      for (size_t pos2 = pos+1; pos2 < 18; pos2++) 
      {
        int contype = tile->getType(pos, pos2); 

        if ((contype == ROAD || contype == CITY) && ((pos2 <= 16 && !marked[row][col][pos2]) || pos2 == 17)) 
        {
          // check if tiles on both sides before recusive search

          int possibleEndpoints = 0;
          
          if (!endPartPoints) {
            {
              size_t colp, rowp; 
              getNeighborCoords(row, col, pos, rowp, colp); 
              int flipside = flipped(pos); 
              if (m_board[rowp][colp] >= 0 && !marked[rowp][colp][flipside]) possibleEndpoints++;
            }
            
            if (pos2 == END || pos2 == MID) possibleEndpoints++;
            else {
              size_t colp, rowp; 
              getNeighborCoords(row, col, pos2, rowp, colp); 
              int flipside = flipped(pos2); 
              if (m_board[rowp][colp] >= 0 && !marked[rowp][colp][flipside]) possibleEndpoints++;
            }
          }

          // only do the recursive search if both directions have possible endpoints
          if (endPartPoints || (!endPartPoints && possibleEndpoints == 2)) 
          {
            marked[row][col][pos] = true;
            std::vector<MeeplePos> meeplepos1;  // meeples of player1
            std::vector<MeeplePos> meeplepos2;  // meeples of player2
            clearTilesMarkers();
            bool closed = immediateScoring_rec(row, col, pos, type, meeplepos1, meeplepos2, true, endPartPoints); 
            size_t totalTiles = countTileMarkers(m_rows, m_cols); 
            
            if (closed) {

              // compare meeples, adjust score, remove meeples
              int points = 0; 
              if (!endPartPoints)
                points = (type == ROAD ? totalTiles : totalTiles*2); 
              else
                points = totalTiles;
             
              if (meeplepos1.size() > 0 || meeplepos2.size() > 0) {
                if (meeplepos1.size() > meeplepos2.size()) {
                  m_score1 += points; 
                }
                else if (meeplepos2.size() > meeplepos1.size()) {
                  m_score2 += points; 
                }
                else
                {
                  m_score1 += points;
                  m_score2 += points;
                }

                for (size_t idx = 0; idx < meeplepos1.size(); idx++) {
                  removeMeeple(meeplepos1[idx].row, meeplepos1[idx].col, meeplepos1[idx].pos);
                  m_meeples1++;
                }
                
                for (size_t idx = 0; idx < meeplepos2.size(); idx++) {
                  removeMeeple(meeplepos2[idx].row, meeplepos2[idx].col, meeplepos2[idx].pos);
                  m_meeples2++;
                }
                
                clearMarked();
                meeplePlacedRemoved(row, col, pos, type, true); // unflood m_occupy
              }
            }
          }
        }
      }
    }
  }
}

void GameState::partPoints()
{
  clearMarked();
  for (size_t r = 1; r <= m_rows; r++) {
    for (size_t c = 1; c <= m_cols; c++) { 
      int tileId = m_board[r][c]; 
      if (tileId >= 0) { 
        immediateScoring(r, c, true); 
      }
    }
  }
}

bool GameState::discoverCities_rec(size_t row, size_t col, size_t mpos, int cityId)
{
  int tileId = m_board[row][col]; 
  Tile * tile = allTiles[tileId];

  bool ret = true;

  // try connections on this tile first
  
  for (size_t pos = 0; pos < 18; pos++) {

    // no self-loops
    if (pos == mpos)
      continue;

    if (cities[row][col][pos] == 0)
    {
      int desttype = tile->getType(mpos, pos);
    
      // is it a connection of the same type?
      if (desttype == CITY) { 
      
        // now check the internal connections

        if (pos == END) 
        {
          //cities[row][col][pos] = cityId;
          // can't return true here: must check outside connections
        }
        else 
        {
          // keep extending the snake
          cities[row][col][pos] = cityId;
          bool complete = discoverCities_rec(row, col, pos, cityId);
          if (!complete)
            ret = false;
        } 
      }
    }
  }

  // now try possible neighbors
  
  // north
  if (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
  {
    size_t newmpos = flipped(mpos);

    if (cities[row-1][col][newmpos] == 0) {
      if (m_board[row-1][col] < 0) 
        ret = false;
      else {
        cities[row-1][col][newmpos] = cityId;
        bool complete = discoverCities_rec(row-1, col, newmpos, cityId);
        if (!complete) ret = false;
      }
    }
  }
    
  // east
  if (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
  {
    size_t newmpos = flipped(mpos);

    if (cities[row][col+1][newmpos] == 0) {
      if (m_board[row][col+1] < 0) 
        ret = false; 
      else {
        cities[row][col+1][newmpos] = cityId;
        bool complete = discoverCities_rec(row, col+1, newmpos, cityId);
        if (!complete) ret = false;
      }
    }
  }

  // south
  if (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
  {
    size_t newmpos = flipped(mpos);
    
    if (cities[row+1][col][newmpos] == 0) {
      if (m_board[row+1][col] < 0) 
        ret = false; 
      else {
        cities[row+1][col][newmpos] = cityId;
        bool complete = discoverCities_rec(row+1, col, newmpos, cityId);
        if (!complete) ret = false;
      }
    }
  }
   
  // west 
  if (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
  {
    size_t newmpos = flipped(mpos);
    if (cities[row][col-1][newmpos] == 0) {
      if (m_board[row][col-1] < 0) 
        ret = false; 
      else {
        cities[row][col-1][newmpos] = cityId;
        bool complete = discoverCities_rec(row, col-1, newmpos, cityId);
        if (!complete) ret = false;
      }
    }
  }

  // if we've exhausted everything, we have a self loop. considered complete!
  return ret;
}

void GameState::discoverCities(size_t row, size_t col, int & cityId)
{
  int tileId = m_board[row][col]; 
  Tile * tile = allTiles[tileId];
  
  //std::cerr << "discovering cities: " << row << " " << col << " " << tileId << std::endl;

  for (size_t pos = 0; pos < 18; pos++) {
      
    int type = tile->getType(pos); 
    
    if (type == CITY && cities[row][col][pos] == 0) 
    {
      cityId++; 

      // TODO: recursive search to check if closed
      cities[row][col][pos] = cityId;
      bool complete = discoverCities_rec(row, col, pos, cityId);
      if (complete) {
        completedCities.push_back(cityId);
      }
    }
  }
}
    
void GameState::scoreFarms_rec(size_t row, size_t col, size_t mpos, 
  std::vector<MeeplePos> & meeplepos1, std::vector<MeeplePos> & meeplepos2, 
  std::vector<int> & citiesTouched)
{
  int tileId = m_board[row][col]; 
  Tile * tile = allTiles[tileId];

  // check for a meeple at this location
  size_t mplayer = meeplePlayer(row, col, mpos);
  if (mplayer > 0) {
    MeeplePos mp; 
    mp.row = row;
    mp.col = col; 
    mp.pos = mpos; 
    mp.player = mplayer;

    if (mplayer == 1) 
      meeplepos1.push_back(mp);
    else if (mplayer == 2) 
      meeplepos2.push_back(mp);
  }
  
  // try connections to cities on this tile first

  std::vector<FCon> fcons;
  tile->getFCons(fcons);
  for (size_t i = 0; i < fcons.size(); i++) {
    if (fcons[i].fromPos == mpos) {
      int cityId = cities[row][col][fcons[i].toPos];
      if (cityId > 0 && contains(completedCities, cityId) && !contains(citiesTouched, cityId)) 
        citiesTouched.push_back(cityId);     
    }
  }

  // try connections on this tile first
  
  for (size_t pos = 0; pos < 18; pos++) {

    // no self-loops
    if (pos == mpos)
      continue;

    if (!marked[row][col][pos])
    {
      int desttype = tile->getType(mpos, pos);
    
      // is it a connection of the same type?
      if (desttype == FARM) { 
      
        // now check the internal connections

        if (pos == END) 
        {
          marked[row][col][pos] = true;
          // can't return true here: must check outside connections
        }
        else 
        {
          // keep extending the snake
          marked[row][col][pos] = true;
          scoreFarms_rec(row, col, pos, meeplepos1, meeplepos2, citiesTouched);
        } 
      }
    }
  }

  // now try possible neighbors
  
  // north
  if (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row-1][col][newmpos] && m_board[row-1][col] >= 0) {
      marked[row-1][col][newmpos] = true;
      scoreFarms_rec(row-1, col, newmpos, meeplepos1, meeplepos2, citiesTouched);
    }
  }
    
  // east
  if (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row][col+1][newmpos] && m_board[row][col+1] >= 0) {
      marked[row][col+1][newmpos] = true;
      scoreFarms_rec(row, col+1, newmpos, meeplepos1, meeplepos2, citiesTouched);
    }
  }

  // south
  if (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
  {
    size_t newmpos = flipped(mpos);
    
    if (!marked[row+1][col][newmpos] && m_board[row+1][col] >= 0) {
      marked[row+1][col][newmpos] = true;
      scoreFarms_rec(row+1, col, newmpos, meeplepos1, meeplepos2, citiesTouched);
    }
  }
   
  // west 
  if (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
  {
    size_t newmpos = flipped(mpos);
    if (!marked[row][col-1][newmpos] && m_board[row][col-1] >= 0) {
      marked[row][col-1][newmpos] = true;
      scoreFarms_rec(row, col-1, newmpos, meeplepos1, meeplepos2, citiesTouched);
    }
  }
}

void GameState::scoreFarms(size_t row, size_t col) {
  
  int tileId = m_board[row][col]; 
  Tile * tile = allTiles[tileId];
  
  //std::cerr << "scoring farms: " << row << " " << col << " " << tileId << std::endl;

  for (size_t pos = 0; pos < 18; pos++) {
      
    int type = tile->getType(pos); 
    
    if (type == FARM && !marked[row][col][pos]) 
    {
      marked[row][col][pos] = true;

      std::vector<MeeplePos> meeplepos1;
      std::vector<MeeplePos> meeplepos2;
      std::vector<int> citiesTouched;

      scoreFarms_rec(row, col, pos, meeplepos1, meeplepos2, citiesTouched);
     
      if (citiesTouched.size() > 0 && (meeplepos1.size() > 0 || meeplepos2.size() > 0))
      {
        // compare meeples, adjust score, remove meeples
        int points = citiesTouched.size()*3; 
       
        if (meeplepos1.size() > meeplepos2.size()) {
          m_score1 += points; 
        }
        else if (meeplepos2.size() > meeplepos1.size()) {
          m_score2 += points; 
        }
        else
        {
          m_score1 += points;
          m_score2 += points;
        }

        for (size_t idx = 0; idx < meeplepos1.size(); idx++) {
          removeMeeple(meeplepos1[idx].row, meeplepos1[idx].col, meeplepos1[idx].pos);
          m_meeples1++;
        }
        
        for (size_t idx = 0; idx < meeplepos2.size(); idx++) {
          removeMeeple(meeplepos2[idx].row, meeplepos2[idx].col, meeplepos2[idx].pos);
          m_meeples2++;
        }
      }
    }
  }

}

void GameState::farmPoints() 
{
  // step 1: discover cities, and check complete
  clearCities();
  completedCities.clear();

  int cityId = 0; 
  for (size_t r = 1; r <= m_rows; r++)
    for (size_t c = 1; c <= m_cols; c++) 
    {
      if (m_board[r][c] >= 0)
        discoverCities(r, c, cityId);
    }
  
  // step 2: discover farms, for each check how many completed cities they're touching
  //         and how many meeples are 

  clearMarked();
  for (size_t r = 1; r <= m_rows; r++)
    for (size_t c = 1; c <= m_cols; c++) 
    {
      if (m_board[r][c] >= 0)
        scoreFarms(r, c);
    }
}

void GameState::reshuffleBag() {

  std::vector<Tile*> bag1;
  std::vector<Tile*> bag2;

  for (size_t idx = m_bagIndex; idx < m_bag.size(); idx++) 
    bag1.push_back(m_bag[idx]); 

  while(bag1.size() > 0) {
    size_t idx = rand() % bag1.size(); 
    Tile * tile = bag1[idx]; 
    bag1.erase(bag1.begin()+idx);
    bag2.push_back(tile); 
  }

  for (size_t idx = 0; idx < bag2.size(); idx++) 
    m_bag[m_bagIndex+idx] = bag2[idx];  
}

/* apply the specific move */
bool GameState::makeMove(Move mv) {

  m_curDelta.reset(); 

  // first, get all the info
  size_t row = mv.row();
  size_t col = mv.col();
  size_t ori = mv.ori();
  size_t mpos = mv.mpos(); 

  // tile ID is the one in the bag plus orientation value when placing it
  size_t tileId = m_bag[0]->getId() + ori; 
  Tile * tile = allTiles[tileId]; 
  assert(tile != NULL);

  m_curDelta.tileId = m_bag[0]->getId(); // need to put bad in the bag unrotated
  m_curDelta.ori = ori;
  m_curDelta.row = row;
  m_curDelta.col = col;
  m_curDelta.pos = mpos;
  m_curDelta.score1before = m_score1;
  m_curDelta.score2before = m_score2;
  m_curDelta.meeples1before = m_meeples1;
  m_curDelta.meeples2before = m_meeples2;
  m_curDelta.tilesRemBefore = m_tilesRemaining;
  m_curDelta.rowsBefore = m_rows;
  m_curDelta.colsBefore = m_cols;
  m_curDelta.turnBefore = m_turn;

  // remove it from the bag
  m_bag.erase(m_bag.begin());
  m_tilesRemaining = m_bag.size();
  
  // place the tile onto the board
  m_board[row][col] = tileId;
  if (row > m_rows)
    m_rows = row; 
  if (col > m_cols)
    m_cols = col; 

  // reposition board, if needed
  int repos = repositionBoard(row, col);
  if (repos == 1) // shift down
  {
    row++; 
    m_rows++;
    m_curDelta.reposition = 1;
  }
  else if (repos == 2) // shift right
  {
    col++; 
    m_cols++;
    m_curDelta.reposition = 2;
  }
  else
  {
    m_curDelta.reposition = 0;
  }

  // place meeple if needed (only after reposition)
  if (mpos != 18) {
    MeeplePos mp;
    mp.row = row;
    mp.col = col;
    mp.pos = mpos; 
    mp.player = m_turn;
    m_meeplepos.push_back(mp);

    m_curDelta.meeplePos.push_back(mp); 
    m_curDelta.meeplePosAdded.push_back(true);
    
    size_t & m_meeples = (m_turn == 1 ? m_meeples1 : m_meeples2); 
    m_meeples--;

    // flood meeple occupies
    clearMarked();
    meeplePlacedRemoved(row, col, mpos, tile->getType(mpos), false);
  }

  // spread the occupied markers
  for (size_t p = 0; p <= 16; p++) 
    marked[row][col][p] = false;

  for (size_t p = 0; p <= 16; p++)
    spreadOccupied(row, col, p, tile->getType(p));

  // do scoring
  immediateScoring(row, col, false); 

  // next player's turn
  m_turn = 3-m_turn;

  if (m_tilesRemaining > 0) 
  {
    // check: if there are no moves, reshuffle bag
    while (!hasAtLeastOnePlacement()) {
      if (m_curDelta.bagBefore.size() == 0) {
        m_curDelta.bagBefore = m_bag;
      }

      //std::cerr << "reshuffling bag" << std::endl;
      reshuffleBag(); 
      //genPlacements(ml);
    }
  }
  else 
  {
    // game is over. count part points and farm points
    partPoints();
    farmPoints();
  }

  m_deltas.push_back(m_curDelta); 

	return true;
}

// everything above, done in reverse order
void GameState::undoMove() {

  undoing = true; 

  assert(m_deltas.size() > 0);   
  MoveDelta & delta = m_deltas[m_deltas.size()-1]; 

  if (delta.bagBefore.size() > 0) { 
    m_bag = delta.bagBefore;
  }

  m_turn = delta.turnBefore;

  // undo meeple placements / removals
  assert(delta.meeplePos.size() == delta.meeplePosAdded.size());

  for (int idx = delta.meeplePos.size()-1; idx >= 0; idx--) {
    if (delta.meeplePosAdded[idx]) 
    {
      // it was added, so remove it. 
      removeMeeple(delta.meeplePos[idx].row, delta.meeplePos[idx].col, delta.meeplePos[idx].pos);  
    }
    else
    {
      // it was removed, so add it
      m_meeplepos.push_back(delta.meeplePos[idx]); 
    }
  }
 
  // now occupy deltas
  
  for (int idx = delta.occupyDeltas.size()-1; idx >= 0; idx--) {
    OccupyDelta & od = delta.occupyDeltas[idx];
    if (od.changedTo == true) {
      assert(m_occupied[od.row][od.col][od.pos] == true);
      m_occupied[od.row][od.col][od.pos] = false;
    }
    else if (od.changedTo == false) {
      assert(m_occupied[od.row][od.col][od.pos] == false);
      m_occupied[od.row][od.col][od.pos] = true;
    }
  }
  
  // undo reposition if necessary
  if (delta.reposition > 0) {
    undoReposition(delta.reposition);
  }

  m_rows = delta.rowsBefore;
  m_cols = delta.colsBefore;

  // take tile from board, put back at top of bag
  assert(m_board[delta.row][delta.col] >= 0);
  m_board[delta.row][delta.col] = -1;

  int tileId = delta.tileId; // put it back in the bag unrotated
  Tile * tile = allTiles[tileId];

  tilelist_t::iterator iter = m_bag.begin();
  m_bag.insert(iter, tile);

  // reset variables
  m_score1 = delta.score1before;
  m_score2 = delta.score2before;
  m_meeples1 = delta.meeples1before;
  m_meeples2 = delta.meeples2before;
  m_tilesRemaining = delta.tilesRemBefore;
  m_turn = delta.turnBefore;

  // make sure everything adds up
  assert(m_meeples1 + m_meeples2 + m_meeplepos.size() == 14); 
  assert(m_tilesRemaining == m_bag.size()); 

  //std::cerr << "successful undo" << std::endl;

  undoing = false; 
  m_deltas.pop_back(); 
}

bool GameState::inBounds(size_t row, size_t col) const {
  return (row >= 0 && row < 74 && col >= 0 && col < 74);
}

int GameState::repositionBoard(size_t row, size_t col) {

  int repos = 0;
  int rows = static_cast<int>(m_rows);
  int cols = static_cast<int>(m_cols);

  if (row == 0)
  {
    for (int r = rows+1; r >= 0; r--)
      for (int c = 1; c <= cols; c++)
      {
        if (r > 0) { 
          m_board[r][c] = m_board[r-1][c];

          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = m_occupied[r-1][c][p];
        }
        else {
          m_board[r][c] = -1; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = false;
        }
      }

    repos = 1; // shift down

    // move the meeples
    for (size_t idx = 0; idx < m_meeplepos.size(); idx++)
      m_meeplepos[idx].row++; 

  }
  else if (col == 0) 
  {
    for (int c = cols+1; c >= 0; c--) 
      for (int r = 1; r <= rows; r++) 
      {
        if (c > 0) {
          m_board[r][c] = m_board[r][c-1]; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = m_occupied[r][c-1][p];
        }
        else {
          m_board[r][c] = -1; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = false; 
        }
      }
    
    repos = 2; // shift right
    
    // move the meeples
    for (size_t idx = 0; idx < m_meeplepos.size(); idx++)
      m_meeplepos[idx].col++; 
  }

  return repos;
}

void GameState::undoReposition(size_t repos) {
  
  int rows = static_cast<int>(m_rows);
  int cols = static_cast<int>(m_cols);

  if (repos == 1)
  {
    // was shifted down. so shift up
    for (int r = 0; r <= rows; r++)
      for (int c = 0; c < 74; c++)
      {
        if (r < rows) { 
          m_board[r][c] = m_board[r+1][c];

          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = m_occupied[r+1][c][p];
        }
        else {
          m_board[r][c] = -1; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = false;
        }
      }

    // move the meeples
    for (size_t idx = 0; idx < m_meeplepos.size(); idx++)
      m_meeplepos[idx].row--; 

  }
  else if (repos == 2) 
  {
    // was shifted right. so shift left

    for (int c = 0; c <= cols; c++) 
      for (int r = 0; r < 74; r++) 
      {
        if (c < cols) {
          m_board[r][c] = m_board[r][c+1]; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = m_occupied[r][c+1][p];
        }
        else {
          m_board[r][c] = -1; 
          
          for (size_t p = 0; p < 18; p++) 
            m_occupied[r][c][p] = false; 
        }
      }
    
    // move the meeples
    for (size_t idx = 0; idx < m_meeplepos.size(); idx++)
      m_meeplepos[idx].col--; 
  }
  else { 
    assert(false);
  }
}

bool GameState::legalTilePlacement(size_t tileId, size_t row, size_t col) const {

  // for each side, check compatibilities
  Tile * placingTile = allTiles[tileId]; 
  assert(placingTile != NULL);

  // north
  if (inBounds(row-1, col) && m_board[row-1][col] >= 0) { 
    size_t neighborId = m_board[row-1][col];
    Tile * neighborTile = allTiles[neighborId];
    assert(neighborTile != NULL);

    if (   placingTile->getType(TOPLEFT) == neighborTile->getType(BOTLEFT)
        && placingTile->getType(TOPMID) == neighborTile->getType(BOTMID)
        && placingTile->getType(TOPRIGHT) == neighborTile->getType(BOTRIGHT)
        && placingTile->getType(TOP) == neighborTile->getType(BOT))
    {
      // pass
    }
    else
    {      
      return false;
    }
  }
  
  // east
  if (inBounds(row, col+1) && m_board[row][col+1] >= 0) { 
    size_t neighborId = m_board[row][col+1];
    Tile * neighborTile = allTiles[neighborId];
    assert(neighborTile != NULL);

    if (   placingTile->getType(RIGHTTOP) == neighborTile->getType(LEFTTOP)
        && placingTile->getType(RIGHTMID) == neighborTile->getType(LEFTMID)
        && placingTile->getType(RIGHTBOT) == neighborTile->getType(LEFTBOT)
        && placingTile->getType(RIGHT) == neighborTile->getType(LEFT))
    {
      // pass
    }
    else
    {
      return false;
    }
  }

  // south
  if (inBounds(row+1, col) && m_board[row+1][col] >= 0) { 
    size_t neighborId = m_board[row+1][col];
    Tile * neighborTile = allTiles[neighborId];
    assert(neighborTile != NULL);

    if (   placingTile->getType(BOTLEFT) == neighborTile->getType(TOPLEFT)
        && placingTile->getType(BOTMID) == neighborTile->getType(TOPMID)
        && placingTile->getType(BOTRIGHT) == neighborTile->getType(TOPRIGHT)
        && placingTile->getType(BOT) == neighborTile->getType(TOP))
    {
      // pass
    }
    else
    {
      return false;
    }
  }

  
  // west
  if (inBounds(row, col-1) && m_board[row][col-1] >= 0) { 
    size_t neighborId = m_board[row][col-1];
    Tile * neighborTile = allTiles[neighborId];
    assert(neighborTile != NULL);

    if (   placingTile->getType(LEFTTOP) == neighborTile->getType(RIGHTTOP)
        && placingTile->getType(LEFTMID) == neighborTile->getType(RIGHTMID)
        && placingTile->getType(LEFTBOT) == neighborTile->getType(RIGHTBOT)
        && placingTile->getType(LEFT) == neighborTile->getType(RIGHT))
    {
      // pass
    }
    else
    {
      return false;
    }
  }

  // passed everything
  return true;
}
    
size_t GameState::meeplePlayer(size_t row, size_t col, size_t mpos) const {
  for (size_t idx = 0; idx < m_meeplepos.size(); idx++) {
    if (m_meeplepos[idx].row == row && m_meeplepos[idx].col == col && m_meeplepos[idx].pos == mpos) 
      return m_meeplepos[idx].player;
  }

  return 0;
}

bool GameState::isMeepleHere(size_t row, size_t col, size_t mpos) const {
  return (meeplePlayer(row, col, mpos) > 0); 
}

size_t GameState::flipped(size_t mpos) const {
  switch(mpos) {
    case TOP:       return BOT;
    case RIGHT:     return LEFT;
    case LEFT:      return RIGHT;
    case BOT:       return TOP;
    case TOPLEFT:   return BOTLEFT;
    case TOPMID:    return BOTMID;
    case TOPRIGHT:  return BOTRIGHT;
    case RIGHTTOP:  return LEFTTOP;
    case RIGHTMID:  return LEFTMID;
    case RIGHTBOT:  return LEFTBOT;
    case BOTRIGHT:  return TOPRIGHT;
    case BOTMID:    return TOPMID;
    case BOTLEFT:   return TOPLEFT;
    case LEFTTOP:   return RIGHTTOP;
    case LEFTMID:   return RIGHTMID;
    case LEFTBOT:   return RIGHTBOT;
    default: 
    {
      assert(false);
      break;
    }
  }

  return 100;
}

bool GameState::spreadOccupied(size_t row, size_t col, size_t mpos, size_t type) {

  // only use markings on own tile

  size_t tileId = m_board[row][col]; 
  assert(tileId >= 0 && tileId < 96);
  Tile * tile = allTiles[tileId]; 
  assert(tile != NULL);
  
  bool retVal = false;

  // find all connections on this tile
  for (size_t e2 = 0; e2 < 18; e2++) {

    int contype = tile->getType(mpos, e2); 
    
    if (contype >= 0) {
      assert(contype == static_cast<int>(type)); 

      if (m_occupied[row][col][e2]) {
        setOccupied(row, col, mpos, false); 
        continue;
      }
     
      if (!marked[row][col][e2]) {
        marked[row][col][e2] = true;    
        
        if (spreadOccupied(row, col, e2, type)) {
          setOccupied(row, col, mpos, false); 
          retVal = true;
        }
      }
    }
  }

  // look at neighbor tiles, but don't search them recursively
  // since info maintained incrementally
  
  // north
  if (   (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
      && inBounds(row-1, col) && m_board[row-1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row-1][col][newmpos]) 
    { setOccupied(row, col, mpos, false); retVal = true; }
  }
    
  // east
  if (   (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
      && inBounds(row, col+1) && m_board[row][col+1] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row][col+1][newmpos]) 
    { setOccupied(row, col, mpos, false); retVal = true; }
  }

  // south
  if (   (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
      && inBounds(row+1, col) && m_board[row+1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row+1][col][newmpos]) 
    { setOccupied(row, col, mpos, false); retVal = true; }
  }
   
  // west 
  if (   (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
      && inBounds(row, col-1) && m_board[row][col-1] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row][col-1][newmpos]) 
    { setOccupied(row, col, mpos, false); retVal = true; }
  }
  
  return retVal;
}

bool GameState::doesMeepleConnectTo(size_t row, size_t col, size_t mpos, size_t type) const {

  // only use markings on own tile

  size_t tileId = m_board[row][col]; 
  assert(tileId >= 0 && tileId < 96);
  Tile * tile = allTiles[tileId]; 
  assert(tile != NULL);

  // find all connections on this tile
  for (size_t e2 = 0; e2 < 18; e2++) {

    int contype = tile->getType(mpos, e2); 
    
    if (contype >= 0) {
      //std::cerr << contype << " " << type << std::endl;
      assert(contype == static_cast<int>(type)); 
     
      if (!marked[row][col][e2]) {
        marked[row][col][e2] = true;    
        
        // only on the tile being placed
        //if (isMeepleHere(row, col, e2)) return true;
        
        if (doesMeepleConnectTo(row, col, e2, type)) return true;
      }
    }
  }

  // look at neighbor tiles, but don't search them recursively
  // since info maintained incrementally
  
  // north
  if (   (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
      && inBounds(row-1, col) && m_board[row-1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row-1][col][newmpos]) return true;
  }
    
  // east
  if (   (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
      && inBounds(row, col+1) && m_board[row][col+1] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row][col+1][newmpos]) return true;
  }

  // south
  if (   (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
      && inBounds(row+1, col) && m_board[row+1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row+1][col][newmpos]) return true;
  }
   
  // west 
  if (   (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
      && inBounds(row, col-1) && m_board[row][col-1] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    if (m_occupied[row][col-1][newmpos]) return true;
  }
  
  return false;
}

void GameState::setOccupied(size_t row, size_t col, size_t pos, bool removing) {
  if (removing) {
    if (m_occupied[row][col][pos]) {
      OccupyDelta od; 
      od.row = row; 
      od.col = col; 
      od.pos = pos;
      od.changedTo = false;
      m_occupied[row][col][pos] = false;
      m_curDelta.occupyDeltas.push_back(od);
    }
  }
  else {
    if (!m_occupied[row][col][pos]) { 
      OccupyDelta od; 
      od.row = row; 
      od.col = col; 
      od.pos = pos;
      od.changedTo = true;
      m_occupied[row][col][pos] = true;
      m_curDelta.occupyDeltas.push_back(od);
    }
  }
}

void GameState::meeplePlacedRemoved(size_t row, size_t col, size_t mpos, size_t type, bool removed) {

  setOccupied(row, col, mpos, removed); 

  size_t tileId = m_board[row][col]; 
  assert(tileId >= 0 && tileId < 96);
  Tile * tile = allTiles[tileId]; 
  assert(tile != NULL);

  // find all connections on this tile
  for (size_t e2 = 0; e2 < 18; e2++) {

    int contype = tile->getType(mpos, e2); 
    
    if (contype >= 0) {
      assert(contype == static_cast<int>(type)); 
     
      if (!marked[row][col][e2]) {
        marked[row][col][e2] = true;    
        
        meeplePlacedRemoved(row, col, e2, type, removed);
      }
    }
  }

  // look at neighbor tiles
  
  // north
  if (   (mpos == TOP || mpos == TOPLEFT || mpos == TOPMID || mpos == TOPRIGHT)
      && inBounds(row-1, col) && m_board[row-1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row-1][col][newmpos]) {
      marked[row-1][col][newmpos] = true;
      meeplePlacedRemoved(row-1, col, newmpos, type, removed);
    }
  }
    
  // east
  if (   (mpos == RIGHT || mpos == RIGHTTOP || mpos == RIGHTMID || mpos == RIGHTBOT)
      && inBounds(row, col+1) && m_board[row][col+1] >= 0) 
  {
    size_t newmpos = flipped(mpos);

    if (!marked[row][col+1][newmpos]) {
      marked[row][col+1][newmpos] = true;
      meeplePlacedRemoved(row, col+1, newmpos, type, removed);
    }
  }

  // south
  if (   (mpos == BOT || mpos == BOTLEFT || mpos == BOTMID || mpos == BOTRIGHT)
      && inBounds(row+1, col) && m_board[row+1][col] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    
    if (!marked[row+1][col][newmpos]) {
      marked[row+1][col][newmpos] = true;
      meeplePlacedRemoved(row+1, col, newmpos, type, removed);
    }
  }
   
  // west 
  if (   (mpos == LEFT || mpos == LEFTTOP || mpos == LEFTMID || mpos == LEFTBOT)
      && inBounds(row, col-1) && m_board[row][col-1] >= 0) 
  {
    size_t newmpos = flipped(mpos);
    
    if (!marked[row][col-1][newmpos]) {
      marked[row][col-1][newmpos] = true;
      meeplePlacedRemoved(row, col-1, newmpos, type, removed);
    }
  }
}


bool GameState::legalMeeplePlacement(size_t tileId, size_t row, size_t col, size_t mpos) const {
 
  // first check if it's even a valid placement on the tile
  Tile * tile = allTiles[tileId]; 
  assert(tile != NULL);
  if (!tile->isValidPlacement(mpos)) return false;

  // now do a depth-first neighborhood search of connections for meeples
  int placingType = tile->getType(mpos); 
  assert(placingType != -1);

  marked[row][col][mpos] = true;
  if (doesMeepleConnectTo(row, col, mpos, static_cast<size_t>(placingType))) return false;
  
  // everything checks out: we can place here
  return true;
}



// generate all valid moves. 
void GameState::genMoves(movelist_t &ml) {

  movelist_t pml;
  genPlacements(pml); 

  for (size_t idx = 0; idx < pml.size(); idx++) {
    
    size_t rp = pml[idx].row();
    size_t cp = pml[idx].col();
    size_t ori = pml[idx].ori(); 

    Move mv0(rp, cp, ori, 18); 
    ml.push_back(mv0); 

    for (size_t pos = 0; pos <= 16; pos++) {
      if (legalMove(rp, cp, ori, pos)) {
        Move mv(rp, cp, ori, pos); 
        ml.push_back(mv); 
      }
    }
  }
}

    
void GameState::genPlacements(movelist_t &ml) {

  size_t tileId = m_bag[0]->getId(); 

  for (size_t r = 1; r <= m_rows; r++) 
    for (size_t c = 1; c <= m_cols; c++)
    {
      if (m_board[r][c] < 0) 
        continue;

      // north, east, south, west
      size_t rpset[4] = { r-1,   r, r+1, r   }; 
      size_t cpset[4] = {   c, c+1,   c, c-1 }; 

      for (size_t d = 0; d < 4; d++) {
        size_t rp = rpset[d];
        size_t cp = cpset[d];

        if (m_board[rp][cp] >= 0)
          continue;

        for (size_t rot = 0; rot < 4; rot++) {

          if (legalTilePlacement(tileId+rot, rp, cp)) {

            Move mv(rp, cp, rot, 20); 
            ml.push_back(mv);

          }
        }
      }
    }
}
    
bool GameState::legalMove(size_t rp, size_t cp, size_t ori, size_t mpos) {

  size_t tileId = m_bag[0]->getId();

  assert(legalTilePlacement(tileId+ori, rp, cp)); 
  
  // always OK to not place a meeple
  if (mpos == 18)  
    return true;

  size_t mymeeples = m_turn == 1 ? m_meeples1 : m_meeples2;
  if (mymeeples == 0) return false;

  // temporarily place the tile there (temporarily)
  m_board[rp][cp] = tileId+ori;

  // clear marked on the placing tile
  for (size_t p = 0; p < 18; p++) 
    marked[rp][cp][p] = false;

  if (legalMeeplePlacement(tileId+ori, rp, cp, mpos)) {
    m_board[rp][cp] = -1; // remove the tile
    return true;
  }

  m_board[rp][cp] = -1; // remove the tile
  return false;
}


/* is the game over? */
bool GameState::gameOver() const {
  return (m_bag.size() == 0);
}


/* Print out the string state (used by server) */
std::ostream &operator<<(std::ostream &o, const GameState &s) {

	const std::string line_sep(100, '-');
	return o;
}

/* Print out the string rep of the move */
std::ostream &operator<<(std::ostream &o, const Move &s) {

  o << s.m_row << " " << s.m_col << " " << s.m_ori << " " << s.m_mpos;
	return o;
}


/* The string to send to the client describing the game state */
std::string GameState::agentProtocolStr() const {

	std::ostringstream oss;

  oss << m_score1 << " " << m_score2 << " " 
      << m_meeples1 << " " << m_meeples2 << " "
      << m_turn << " "
      << m_rows << " " << m_cols << " "
      << m_bagIndex << " " << m_tilesRemaining; 

  // what follows is m_rows * m_cols tile IDs (starts at index 9)
  for (size_t r = 1; r <= m_rows; r++)
    for (size_t c = 1; c <= m_cols; c++) 
      oss << " " << m_board[r][c];

  // tiles left in the bag (starts at index 9 + rows*cols)
  for (size_t i = m_bagIndex; i < m_bag.size(); i++)
    oss << " " << m_bag[i]->getId(); 

  // meeple positions (starts at index 9 + rows*cols + tiles_remaining)
  for (size_t i = 0; i < m_meeplepos.size(); i++) 
    oss << " " << m_meeplepos[i].row << " " << m_meeplepos[i].col << " " << m_meeplepos[i].pos << " " << m_meeplepos[i].player;

  // what follows is, for each non-empty type in m_rows * m_cols 
  // 18 0 or 1's for true or false for m_occupy
  for (size_t r = 1; r <= m_rows; r++)
    for (size_t c = 1; c <= m_cols; c++) 
    {
      if (m_board[r][c] >= 0) {
        for (size_t p = 0; p < 18; p++) 
          oss << " " << (m_occupied[r][c][p] ? "1" : "0"); 
      }
    }

	return oss.str();
}


/* Parse the state from the string */
bool parseCarcState(const std::string &str, GameState &g) {

	std::vector<std::string> tokens;
	split(tokens, str, ' ');

  for (size_t r = 0; r < 74; r++)
    for (size_t c = 0; c < 74; c++)
    {
      g.m_board[r][c] = -1; 

      for (size_t p = 0; p < 18; p++) 
        g.m_occupied[r][c][p] = false;
    }
  
  g.m_score1 = to_size_t(tokens[0]);
  g.m_score2 = to_size_t(tokens[1]);
  g.m_meeples1 = to_size_t(tokens[2]);
  g.m_meeples2 = to_size_t(tokens[3]);
  g.m_turn = to_size_t(tokens[4]);
  g.m_rows = to_size_t(tokens[5]);
  g.m_cols = to_size_t(tokens[6]);
  g.m_bagIndex = to_size_t(tokens[7]);
  g.m_tilesRemaining = to_size_t(tokens[8]);

  // board
  size_t index = 9; 
  for (size_t r = 1; r <= g.m_rows; r++)
    for (size_t c = 1; c <= g.m_cols; c++) {
      g.m_board[r][c] = to_int(tokens[index]);
      index++;
    }

  // bag tiles
  g.m_bag.clear();
  for (size_t i = g.m_bagIndex; i < g.m_tilesRemaining; i++)
  {
    size_t tileId = to_size_t(tokens[index]);
    index++;
    g.m_bag.push_back(allTiles[tileId]);
  }

  // meeple coordinates
  g.m_meeplepos.clear(); 
  for (size_t m = 0; m < (14-g.m_meeples1-g.m_meeples2); m++) 
  {
    size_t row = to_size_t(tokens[index]); 
    index++; assert(index < tokens.size());
    size_t col = to_size_t(tokens[index]); 
    index++; assert(index < tokens.size());
    size_t pos = to_size_t(tokens[index]); 
    index++; assert(index < tokens.size());
    size_t player = to_size_t(tokens[index]);
    index++;

    MeeplePos mp;
    mp.row = row; mp.col = col; mp.pos = pos; mp.player = player;

    g.m_meeplepos.push_back(mp); 
  }

  // m_occupied
  for (size_t r = 1; r <= g.m_rows; r++)
    for (size_t c = 1; c <= g.m_cols; c++) {
      if (g.m_board[r][c] >= 0) {
        for (size_t p = 0; p < 18; p++, index++) { 
          g.m_occupied[r][c][p] = (tokens[index] == "1" ? true : false);
        }
      }
    }

  assert(index == tokens.size()); 

	return true;
}


/* a unique representation of the game state */
hash_t GameState::hash() const {

  // unimplemented!
	return 1;
}
