#ifndef __TILES_HPP__
#define __TILES_HPP__

#include <iostream>
#include <vector>
#include <cassert>

typedef unsigned long long hash_t; 

/**
 * Every tile can be encoded as a list of 3-tuples of the form:
 *   (e_1, e_2, t) 
 *
 * Where there is a connection of e_1 -> e_2 of land type t
 *
 * There are 3 of points per tiles edge, 4 more points summarizing each tile edge, 
 * 1 representing the middle, and 1 representing "the connection ends" so 
 * e_i \in { 0, ... 17 }
 *
 * t is one of 4 land types: (cloister = 0, road = 1, city = 2, green farmland = 3)
 *
 * To represent that the connection (e_1, e_2, t) ecists we set connections[e1][e2][t] to true. 
 */

// maximum length of game
extern const size_t TotalTiles;

// number of different types of tiles
extern const size_t TotalUniqueTileTypes;

// number of different types of tiles
extern const int TotalUniqueTileTypesInt;

// number of different types of tiles after they have been rotated
extern const size_t TotalTileTypes;

#define TOPLEFT   0
#define TOPMID    1
#define TOPRIGHT  2
#define RIGHTTOP  3
#define RIGHTMID  4
#define RIGHTBOT  5
#define BOTRIGHT  6
#define BOTMID    7
#define BOTLEFT   8
#define LEFTBOT   9
#define LEFTMID   10
#define LEFTTOP   11
#define TOP       12
#define RIGHT     13
#define BOT       15
#define LEFT      14
#define MID       16
#define END       17

#define CLOISTER  0
#define ROAD      1
#define CITY      2
#define FARM      3

struct FCon {
  size_t fromPos;  // always a farm
  size_t toPos;    // always a city
};

class Tile { 

	public:
		// default constructor
		Tile(size_t id) { 
      m_id = id; 

      m_placements.push_back(std::vector<size_t>()); 
      m_placements.push_back(std::vector<size_t>()); 
      m_placements.push_back(std::vector<size_t>()); 
      m_placements.push_back(std::vector<size_t>()); 

      for (size_t i = 0; i < 18; i++)
        for (size_t j = 0; j < 18; j++)
          for (size_t k = 0; k < 4; k++)
            m_connections[i][j][k] = false;

      for (size_t i = 0; i < 18; i++) 
        m_typeCache[i] = 5; 

      m_fcons.clear(); 
      m_shield = false; 
    }
		
    // copy constructor
		Tile(const Tile &t) {
      copyFrom(t);
    }

    void copyFrom(const Tile &t) {

      // copy placements
      for (size_t i = 0; i < t.m_placements.size(); i++) {
        m_placements[i].clear(); 

        for (size_t j = 0; j < t.m_placements[i].size(); j++) 
          m_placements[i].push_back(t.m_placements[i][j]); 
      }
     
      // copy connections
      for (size_t i = 0; i < 18; i++)
        for (size_t j = 0; j < 18; j++)
          for (size_t k = 0; k < 4; k++)
            m_connections[i][j][k] = t.m_connections[i][j][k];

      // copy type cache
      for (size_t i = 0; i < 18; i++) 
        m_typeCache[i] = t.m_typeCache[i]; 

      m_fcons = t.m_fcons;
      m_shield = t.m_shield;
    }

    void buildTypeCache();
    bool hasShield() { return m_shield; }
    void addShield() { m_shield = true; }
    void addPlacement(size_t e1, size_t t); 
    void addConnection(size_t e1, size_t e2, size_t t); 
    void addFCon(size_t e1, size_t e2); 
    void addOneWayConnection(size_t e1, size_t e2, size_t t); 
    void rotate90cw(); 

    size_t getId() { return m_id; }

    int getType(size_t pos) const;
    int getType(size_t e1, size_t e2) const; 

    bool isValidPlacement(size_t e1) const;
   
    void getFCons(std::vector<FCon> & fc) const;
		
	private: 

    // where can we place meeples on this tiles
    std::vector< std::vector<size_t> > m_placements; 

    // farm connections (to cities)
    std::vector<FCon> m_fcons; 

    // what are the terrain type connections in this tile
    bool m_connections[18][18][4]; 

    int m_typeCache[18];
    
    bool m_shield;
    size_t m_id; 
};

extern std::ostream &operator<<(std::ostream &o, const Tile &s);
typedef std::vector<Tile*> tilelist_t;

extern tilelist_t allTiles; 

extern void createTiles(); 
extern void destroyTiles();
extern void createBag(tilelist_t & bag); 

#endif
