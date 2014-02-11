#ifndef __RULES_HPP__
#define __RULES_HPP__

#include "tiles.hpp"

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>

typedef unsigned long long hash_t; 

struct MeeplePos
{
  size_t row;
  size_t col;
  size_t pos;
  size_t player;
};
typedef std::vector<MeeplePos> mposlist_t;

struct OccupyDelta
{
  size_t row;
  size_t col;
  size_t pos;
  bool changedTo;
};

// tracks all changes made to a state for the undo
struct MoveDelta 
{
  size_t tileId;
  size_t row;
  size_t col;
  size_t pos;
  size_t ori; 
    
  size_t score1before;
  size_t score2before;
  size_t meeples1before;
  size_t meeples2before;
  size_t tilesRemBefore; 
  size_t rowsBefore;
  size_t colsBefore;
  size_t turnBefore; 

  size_t reposition; // 0 = not repositioned, 1 = moved down, 2 = moved right

  // occupy / unoccupy
  std::vector<OccupyDelta> occupyDeltas;

  // meeplePosDeltas
  std::vector<MeeplePos> meeplePos; 
  std::vector<bool> meeplePosAdded; // added? (if false, was removed)
    
  // before and after, if reshuffle was done
  tilelist_t bagBefore;

  void reset() { 
    occupyDeltas.clear();
    meeplePos.clear();
    meeplePosAdded.clear(); 
    bagBefore.clear(); 
  }
};

class Move { 

  friend std::ostream &operator<<(std::ostream &o, const Move &s);

	public:
		// default constructor
		Move() { }

		// constructor
		Move(size_t row, size_t col, size_t ori, size_t mpos) 
      : m_row(row), m_col(col), m_ori(ori), m_mpos(mpos) { } 
		
		// copy constructor
		Move(const Move &m) 
      : m_row(m.m_row), m_col(m.m_col), m_ori(m.m_ori), m_mpos(m.m_mpos) { } 

		// returns a string rep of this move
		std::string protocolText(); 

    size_t row() const { return m_row; }
    size_t col() const { return m_col; }
    size_t ori() const { return m_ori; }
    size_t mpos() const { return m_mpos; }

	private: 
		size_t m_row, m_col, m_ori, m_mpos;
};
extern std::ostream &operator<<(std::ostream &o, const Move &s);
typedef std::vector<Move> movelist_t;

class GameState;

class MoveIterator 
{
  public:
    MoveIterator(GameState & _gs);
    bool next(Move & mv); 
    int index() { return legalMoveIndex; }
  
  private: 
    size_t type;           // index into order[]
    size_t idx;
    int legalMoveIndex; 
    size_t pos; 
    movelist_t mpl;        // placements
    GameState & gs;
};



// current game state
class GameState {

	friend std::ostream &operator<<(std::ostream &o, const GameState &s);
	friend bool parseCarcState(const std::string &str, GameState &g);

	public:

		GameState();

		// execute the actions specified by this move
		bool makeMove(Move mv); 
		
    // execute the actions specified by this move
		void undoMove(); 

		// is the game over?
		bool gameOver() const; 

		// return a string representation of this move for the client
		std::string agentProtocolStr() const; 

		// generate all moves
		void genMoves(movelist_t &ml);
   
    // generate placement moves
    void genPlacements(movelist_t &ml); 
   
    bool legalMove(size_t rp, size_t cp, size_t ori, size_t mpos); 

    void getScore(size_t & score1, size_t & score2);
		
    void reshuffleBag(); 

    // a faster version for checking legality of chance 
    bool hasAtLeastOnePlacement(); 

    // if the tile at this bag index is swapped with the first, do we have any legal moves?
    bool legalChanceOutcome(int bagIdx);

    void getChanceDist(std::vector<size_t> & freq, size_t & total, std::vector<size_t> & order);

		// a unique 64-bit integer identifying the state
		hash_t hash() const;

    /* returns tile that was swapped */
    size_t setNextTile(size_t tileId);

    /* undoes the swap */
    void undoSetNextTile(size_t replaceIdx);

    // an incremental move generator 
    MoveIterator moveIterator();
    
    size_t turn() { return m_turn; }

    size_t tilesRemaining() const { return m_bag.size(); }

    size_t firstTileId() const { return m_bag[0]->getId(); }

    /* returns the Zobrist hash value of this state computed from scratch */
    unsigned long long hashVal() const;

		// utility of a gamestate
		double utility() const { 

      // pass through tanh to normalize to [-1,1], then always in view of player 1
      double delta = static_cast<double>(m_score1) - static_cast<double>(m_score2);
      double exp = pow(2.7183, delta/10.0);  
      double util = 100.0*(exp - 1.0) / (exp + 1.0);
      return util;

		} 

    double utility(size_t player) const { 
      if (player == 1) return utility(); 
      else return ((-1.0)*utility()); 
    }

		double maxUtility() const { return 100.0; }
		double minUtility() const { return -100.0; }

	private:
  
    // state transition and help functions 
    void getNeighborCoords(size_t row, size_t col, size_t pos, size_t & rowp, size_t & colp);
    void setOccupied(size_t row, size_t col, size_t pos, bool removing);
    void scoreFarms_rec(size_t row, size_t col, size_t mpos, std::vector<MeeplePos> & meeplepos1, 
                        std::vector<MeeplePos> & meeplepos2, std::vector<int> & citiesTouched);
    void scoreFarms(size_t row, size_t col);
    bool discoverCities_rec(size_t row, size_t col, size_t mpos, int cityId);
    void discoverCities(size_t row, size_t col, int & cityId);
    void farmPoints();
    void partPoints();
    void removeMeeple(size_t row, size_t col, size_t mpos);
    void scoreMonk(size_t row, size_t col, bool endPartPoints); 
    bool immediateScoring_rec(size_t row, size_t col, size_t mpos, int type, std::vector<MeeplePos> & meeplepos1, 
                              std::vector<MeeplePos> & meeplepos2, bool crossingBoundary, bool endPartPoints); 
    void immediateScoring(size_t row, size_t col, bool endPartPoints); 
    int meepleOnTile(size_t row, size_t col, size_t & player) const;
    void meeplePlacedRemoved(size_t row, size_t col, size_t mpos, size_t type, bool remove);

    size_t flipped(size_t mpos) const;
    bool spreadOccupied(size_t row, size_t col, size_t mpos, size_t type);
    bool doesMeepleConnectTo(size_t row, size_t col, size_t mpos, size_t type) const; 
    bool isMeepleHere(size_t row, size_t col, size_t mpos) const;
    size_t meeplePlayer(size_t row, size_t col, size_t mpos) const;

    bool inBounds(size_t row, size_t col) const;
    bool legalTilePlacement(size_t tileId, size_t row, size_t col) const;
    bool legalMeeplePlacement(size_t tileId, size_t row, size_t col, size_t mpos) const;
    int repositionBoard(size_t row, size_t col); 
    void undoReposition(size_t repos);

		size_t m_score1;
		size_t m_score2;
    size_t m_meeples1;       // number of meeples remaining for p1
    size_t m_meeples2;       // number of meeples remaining for p2
		size_t m_turn;           // whose turn is it
    size_t m_rows;           // current number of rows
    size_t m_cols;           // current number of columns
    size_t m_tilesRemaining;
    size_t m_bagIndex;
    
    // -1 means no tile, 0-95 is the tile's ID (index into m_bag)
    int m_board[74][74];  

    // 
    bool m_occupied[74][74][18];

    // bag containing tiles
    tilelist_t m_bag;

    // list of meeple positions
    mposlist_t m_meeplepos;

    // needed for Undo
    MoveDelta m_curDelta;
    std::vector<MoveDelta> m_deltas;

};
extern std::ostream &operator<<(std::ostream &o, const GameState &s);

// parse the game from the string rep
bool parseCarcState(const std::string &str, GameState &g); 

#endif // __RULES_HPP__

