#include "tiles.hpp"

#include <cassert>
#include <cstdlib>

// maximum length of game
const size_t TotalTiles = 72;

// number of different types of tiles
const size_t TotalUniqueTileTypes = 24;

// number of different types of tiles
const int TotalUniqueTileTypesInt = static_cast<int>(TotalUniqueTileTypes);

// number of different types of tiles after they have been rotated
const size_t TotalTileTypes = 96;

// Includes all the different tiles and all four orientations of each
// Indices 0, 4, 8, etc. contain the unrotated tile type
tilelist_t allTiles; 

// Includes the appropriate number of copies of each of the 24 unique tiles unrotated
// probably want this to be part of the game state
//static tilelist_t tileBag; 

std::ostream &operator<<(std::ostream &o, const Tile &s) { 

  // ... 
  return o;
}

static size_t rotatedIndices[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 1, 2, 13, 15, 12, 14, 16, 17, 18 }; 

void Tile::addConnection(size_t e1, size_t e2, size_t t) {
  if (e2 == 17) { 
    addOneWayConnection(e1, e2, t); 
  }
  else {
    m_connections[e1][e2][t] = true;
    m_connections[e2][e1][t] = true;
  }
}

void Tile::addFCon(size_t e1, size_t e2) { 
  FCon fc; 
  fc.fromPos = e1; 
  fc.toPos = e2;
  m_fcons.push_back(fc);
}

void Tile::addOneWayConnection(size_t e1, size_t e2, size_t t) {
  m_connections[e1][e2][t] = true;
  m_connections[e2][e1][t] = false;
}
    
void Tile::buildTypeCache()
{
  for (size_t e1 = 0; e1 < 18; e1++) 
  {
    bool done = false;
 
    for (size_t t = 0; !done && t < 4; t++) {
      for (size_t e2 = 0; !done && e2 < 18; e2++) {
        if (m_connections[e1][e2][t]) {
          m_typeCache[e1] = t; 
          done = true;
        }
      }
    }

    if (!done) m_typeCache[e1] = -1;
  }
}

int Tile::getType(size_t e1) const {

  return (m_typeCache[e1]);

  #if 0  
  for (size_t t = 0; t < 4; t++) {
    for (size_t e2 = 0; e2 < 18; e2++) {
      if (m_connections[e1][e2][t]) {
        m_typeCache[e1] = t; 
        return t; 
      }
    }
  }

  return -1;
  #endif
}

int Tile::getType(size_t e1, size_t e2) const {

  for (int t = 0; t < 4; t++) 
    if (m_connections[e1][e2][t])
      return t; 

  return -1;
}

void Tile::getFCons(std::vector<FCon> & fc) const {
  fc = m_fcons;
}
    
void Tile::addPlacement(size_t e1, size_t t) {
  assert(t >= 0 && t <= 3);
  m_placements[t].push_back(e1); 
}

bool Tile::isValidPlacement(size_t e1) const {
  for (size_t t = 0; t < 4; t++) {
    for (size_t idx = 0; idx < m_placements[t].size(); idx++) {
      if (m_placements[t][idx] == e1) 
        return true;
    }
  }

  return false;
}

void Tile::rotate90cw() { 
   
    for (size_t t = 0; t < m_placements.size(); t++) {
      for (size_t j = 0; j < m_placements[t].size(); j++) {
        m_placements[t][j] = rotatedIndices[m_placements[t][j]]; 
      }
    }
 
    // what are the terrain type connections in this tile
    bool tmp_connections[18][18][4]; 

    for (size_t e1 = 0; e1 < 18; e1++) 
      for (size_t e2 = 0; e2 < 18; e2++) 
      { 
        size_t rotated_e1 = rotatedIndices[e1]; 
        size_t rotated_e2 = rotatedIndices[e2];

        for (size_t t = 0; t < 4; t++) 
          tmp_connections[rotated_e1][rotated_e2][t] = m_connections[e1][e2][t];
      }

    for (size_t e1 = 0; e1 < 18; e1++) 
      for (size_t e2 = 0; e2 < 18; e2++) 
        for (size_t t = 0; t < 4; t++) 
          m_connections[e1][e2][t] = tmp_connections[e1][e2][t];

    // farm connections (to cities)
    for (size_t i = 0; i < m_fcons.size(); i++) {
      m_fcons[i].fromPos = rotatedIndices[ m_fcons[i].fromPos ];
      m_fcons[i].toPos = rotatedIndices[ m_fcons[i].toPos ];
    }

}

void destroyTiles() {
  for (size_t idx = 0; idx < 96; idx++) 
    delete allTiles[idx];
}

void createTiles() {
  std::cerr << "Creating tiles..." << std::endl;
  assert(allTiles.size() == 0); 

  for (size_t i = 0; i < TotalTileTypes; i++) {
    allTiles.push_back(new Tile(i)); 
  }

  for (size_t i = 0; i < TotalTileTypes; i++) {

    // the 3 that follow each 4th index is a rotated version of the previous
    if (i % 4 != 0) { 
      allTiles[i]->copyFrom(*allTiles[i-1]); 
      allTiles[i]->rotate90cw(); 
      continue;
    }

    // every 4th index is a tile with its standard orientation, as taken from the 
    // carcassonne reference sheet in reading order
    switch(i) {

      case 0:  // A
        allTiles[i]->addPlacement(MID, CLOISTER); 
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addConnection(MID, END, CLOISTER); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addPlacement(BOTRIGHT, FARM);
        allTiles[i]->addConnection(BOTRIGHT, BOTLEFT, FARM); 
        allTiles[i]->addConnection(BOTRIGHT, TOP, FARM); 
        allTiles[i]->addConnection(BOTRIGHT, RIGHT, FARM); 
        allTiles[i]->addConnection(BOTRIGHT, LEFT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, TOP, FARM); 
        allTiles[i]->addConnection(BOTLEFT, RIGHT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, LEFT, FARM); 
        allTiles[i]->addConnection(TOP, RIGHT, FARM); 
        allTiles[i]->addConnection(TOP, LEFT, FARM); 
        allTiles[i]->addConnection(RIGHT, LEFT, FARM); 
        break;

      case 4: // B
        allTiles[i]->addPlacement(MID, CLOISTER); 
        allTiles[i]->addPlacement(TOP, FARM); 
        allTiles[i]->addConnection(MID, END, CLOISTER); 
        allTiles[i]->addConnection(TOP, RIGHT, FARM); 
        allTiles[i]->addConnection(TOP, LEFT, FARM); 
        allTiles[i]->addConnection(TOP, BOT, FARM); 
        allTiles[i]->addConnection(RIGHT, LEFT, FARM); 
        allTiles[i]->addConnection(RIGHT, BOT, FARM); 
        allTiles[i]->addConnection(LEFT, BOT, FARM); 
        break;

      case 8: // C
        allTiles[i]->addPlacement(TOP, CITY); 
        allTiles[i]->addConnection(TOP, RIGHT, CITY); 
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(TOP, BOT, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, BOT, CITY); 
        allTiles[i]->addConnection(LEFT, BOT, CITY); 
        allTiles[i]->addShield(); 
        break;

      case 12: // D
        allTiles[i]->addPlacement(TOPMID, ROAD); 
        allTiles[i]->addPlacement(RIGHT, CITY); 
        allTiles[i]->addPlacement(TOPRIGHT, FARM); 
        allTiles[i]->addPlacement(TOPLEFT, FARM); 
        allTiles[i]->addConnection(TOPMID, BOTMID, ROAD); 
        allTiles[i]->addConnection(RIGHT, END, CITY);
        allTiles[i]->addConnection(TOPRIGHT, BOTRIGHT, FARM);
        allTiles[i]->addConnection(TOPLEFT, BOTLEFT, FARM);
        allTiles[i]->addConnection(TOPLEFT, LEFT, FARM);
        allTiles[i]->addConnection(BOTLEFT, LEFT, FARM);
        allTiles[i]->addFCon(TOPRIGHT, RIGHT);
        allTiles[i]->addFCon(BOTRIGHT, RIGHT);
        break;

      case 16: // E
        allTiles[i]->addPlacement(TOP, CITY); 
        allTiles[i]->addPlacement(RIGHT, FARM); 
        allTiles[i]->addConnection(TOP, END, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, FARM);
        allTiles[i]->addConnection(RIGHT, BOT, FARM);
        allTiles[i]->addConnection(LEFT, BOT, FARM); 
        allTiles[i]->addFCon(RIGHT, TOP);
        allTiles[i]->addFCon(BOT, TOP);
        allTiles[i]->addFCon(LEFT, TOP);
        break;

      case 20: // F
        allTiles[i]->addPlacement(RIGHT, CITY); 
        allTiles[i]->addPlacement(TOP, FARM); 
        allTiles[i]->addPlacement(BOT, FARM); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY);
        allTiles[i]->addConnection(BOT, END, FARM);
        allTiles[i]->addConnection(TOP, END, FARM);
        allTiles[i]->addFCon(BOT, RIGHT);
        allTiles[i]->addFCon(TOP, RIGHT);
        allTiles[i]->addShield(); 
        break;

      case 24: // G
        allTiles[i]->addPlacement(TOP, CITY); 
        allTiles[i]->addPlacement(RIGHT, FARM); 
        allTiles[i]->addPlacement(LEFT, FARM); 
        allTiles[i]->addConnection(TOP, BOT, CITY);
        allTiles[i]->addConnection(LEFT, END, FARM);
        allTiles[i]->addConnection(RIGHT, END, FARM);
        allTiles[i]->addFCon(LEFT, TOP);
        allTiles[i]->addFCon(RIGHT, TOP);
        break;

      case 28: // H
        allTiles[i]->addPlacement(RIGHT, CITY); 
        allTiles[i]->addPlacement(LEFT, CITY); 
        allTiles[i]->addPlacement(TOP, FARM); 
        allTiles[i]->addConnection(LEFT, END, CITY);
        allTiles[i]->addConnection(RIGHT, END, CITY);
        allTiles[i]->addConnection(TOP, BOT, FARM);
        allTiles[i]->addFCon(TOP, RIGHT);
        allTiles[i]->addFCon(TOP, LEFT);
        break;
      
      case 32: // I
        allTiles[i]->addPlacement(RIGHT, CITY); 
        allTiles[i]->addPlacement(BOT, CITY); 
        allTiles[i]->addPlacement(TOP, FARM); 
        allTiles[i]->addConnection(RIGHT, END, CITY);
        allTiles[i]->addConnection(BOT, END, CITY);
        allTiles[i]->addConnection(TOP, LEFT, FARM);
        allTiles[i]->addFCon(TOP, RIGHT);
        allTiles[i]->addFCon(TOP, BOT);
        break;
      
      case 36: // J
        allTiles[i]->addPlacement(RIGHTMID, ROAD); 
        allTiles[i]->addPlacement(TOP, CITY); 
        allTiles[i]->addPlacement(LEFT, FARM); 
        allTiles[i]->addPlacement(BOTRIGHT, FARM);
        allTiles[i]->addConnection(RIGHTMID, BOTMID, ROAD);
        allTiles[i]->addConnection(TOP, END, CITY);
        allTiles[i]->addConnection(BOTRIGHT, RIGHTBOT, FARM);
        allTiles[i]->addConnection(RIGHTTOP, BOTLEFT, FARM);
        allTiles[i]->addConnection(RIGHTTOP, LEFT, FARM);
        allTiles[i]->addConnection(BOTLEFT, LEFT, FARM);
        allTiles[i]->addFCon(LEFT, TOP);
        break;

      case 40: // K
        allTiles[i]->addPlacement(TOPMID, ROAD); 
        allTiles[i]->addPlacement(RIGHT, CITY); 
        allTiles[i]->addPlacement(BOT, FARM); 
        allTiles[i]->addPlacement(TOPLEFT, FARM);
        allTiles[i]->addConnection(TOPMID, LEFTMID, ROAD);
        allTiles[i]->addConnection(RIGHT, END, CITY);
        allTiles[i]->addConnection(TOPLEFT, LEFTTOP, FARM);
        allTiles[i]->addConnection(TOPRIGHT, LEFTBOT, FARM);
        allTiles[i]->addConnection(TOPRIGHT, BOT, FARM);
        allTiles[i]->addConnection(LEFTBOT, BOT, FARM);
        allTiles[i]->addFCon(BOT, RIGHT);
        break;

      case 44: // L
        allTiles[i]->addPlacement(TOPMID, ROAD);
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(LEFTMID, ROAD);
        allTiles[i]->addPlacement(TOPRIGHT, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addPlacement(TOPLEFT, FARM);
        allTiles[i]->addPlacement(RIGHT, CITY);
        allTiles[i]->addConnection(TOPMID, END, ROAD); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(LEFTMID, END, ROAD); 
        allTiles[i]->addConnection(RIGHT, END, CITY); 
        allTiles[i]->addConnection(TOPLEFT, LEFTTOP, FARM); 
        allTiles[i]->addConnection(TOPRIGHT, BOTRIGHT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, LEFTBOT, FARM); 
        allTiles[i]->addFCon(TOPRIGHT, RIGHT);
        break;

      case 48: // M
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(RIGHT, FARM);
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, BOT, FARM); 
        allTiles[i]->addFCon(RIGHT, TOP);
        allTiles[i]->addShield(); 
        break;
      
      case 52: // N
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(RIGHT, FARM);
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, BOT, FARM); 
        allTiles[i]->addFCon(RIGHT, TOP);
        break;
      
      case 56: // O
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(RIGHTMID, ROAD);
        allTiles[i]->addPlacement(RIGHTTOP, FARM);
        allTiles[i]->addPlacement(RIGHTBOT, FARM);
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHTMID, BOTMID, ROAD); 
        allTiles[i]->addConnection(RIGHTTOP, BOTLEFT, FARM); 
        allTiles[i]->addConnection(RIGHTBOT, BOTRIGHT, FARM); 
        allTiles[i]->addFCon(RIGHTTOP, TOP);
        allTiles[i]->addShield(); 
        break;
      
      case 60: // P
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(RIGHTMID, ROAD);
        allTiles[i]->addPlacement(RIGHTTOP, FARM);
        allTiles[i]->addPlacement(RIGHTBOT, FARM);
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHTMID, BOTMID, ROAD); 
        allTiles[i]->addConnection(RIGHTTOP, BOTLEFT, FARM); 
        allTiles[i]->addConnection(RIGHTBOT, BOTRIGHT, FARM); 
        allTiles[i]->addFCon(RIGHTTOP, TOP);
        break;

      case 64: // Q
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(BOT, FARM);
        allTiles[i]->addConnection(TOP, RIGHT, CITY); 
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY); 
        allTiles[i]->addConnection(BOT, END, FARM); 
        allTiles[i]->addFCon(BOT, TOP);
        allTiles[i]->addShield(); 
        break;

      case 68: // R
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(BOT, FARM);
        allTiles[i]->addConnection(TOP, RIGHT, CITY); 
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY); 
        allTiles[i]->addConnection(BOT, END, FARM); 
        allTiles[i]->addFCon(BOT, TOP);
        break;
      
      case 72: // S
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(BOTRIGHT, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addConnection(TOP, RIGHT, CITY); 
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(BOTRIGHT, END, FARM); 
        allTiles[i]->addConnection(BOTLEFT, END, FARM); 
        allTiles[i]->addFCon(BOTRIGHT, TOP);
        allTiles[i]->addFCon(BOTLEFT, TOP);
        allTiles[i]->addShield(); 
        break;
      
      case 76: // T
        allTiles[i]->addPlacement(TOP, CITY);
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(BOTRIGHT, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addConnection(TOP, RIGHT, CITY); 
        allTiles[i]->addConnection(TOP, LEFT, CITY); 
        allTiles[i]->addConnection(RIGHT, LEFT, CITY); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(BOTRIGHT, END, FARM); 
        allTiles[i]->addConnection(BOTLEFT, END, FARM); 
        allTiles[i]->addFCon(BOTRIGHT, TOP);
        allTiles[i]->addFCon(BOTLEFT, TOP);
        break;

      case 80: // U
        allTiles[i]->addPlacement(TOPMID, ROAD);
        allTiles[i]->addPlacement(TOPLEFT, FARM);
        allTiles[i]->addPlacement(TOPRIGHT, FARM);
        allTiles[i]->addConnection(TOPMID, BOTMID, ROAD); 
        allTiles[i]->addConnection(TOPRIGHT, BOTRIGHT, FARM); 
        allTiles[i]->addConnection(TOPRIGHT, RIGHT, FARM); 
        allTiles[i]->addConnection(RIGHT, BOTRIGHT, FARM); 
        allTiles[i]->addConnection(TOPLEFT, BOTLEFT, FARM); 
        allTiles[i]->addConnection(TOPLEFT, LEFT, FARM); 
        allTiles[i]->addConnection(LEFT, BOTLEFT, FARM); 
        break;
      
      case 84: // V
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(TOP, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addConnection(BOTMID, LEFTMID, ROAD); 
        allTiles[i]->addConnection(BOTRIGHT, LEFTTOP, FARM); 
        allTiles[i]->addConnection(BOTRIGHT, TOP, FARM); 
        allTiles[i]->addConnection(BOTRIGHT, RIGHT, FARM); 
        allTiles[i]->addConnection(LEFTTOP, TOP, FARM); 
        allTiles[i]->addConnection(LEFTTOP, RIGHT, FARM); 
        allTiles[i]->addConnection(TOP, RIGHT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, LEFTBOT, FARM); 
        break; 
      
      case 88: // W
        allTiles[i]->addPlacement(RIGHTMID, ROAD);
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(LEFTMID, ROAD);
        allTiles[i]->addPlacement(RIGHTTOP, FARM);
        allTiles[i]->addPlacement(RIGHTBOT, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addConnection(RIGHTMID, END, ROAD); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(LEFTMID, END, ROAD); 
        allTiles[i]->addConnection(RIGHTTOP, LEFTTOP, FARM); 
        allTiles[i]->addConnection(RIGHTTOP, TOP, FARM); 
        allTiles[i]->addConnection(LEFTTOP, TOP, FARM); 
        allTiles[i]->addConnection(RIGHTBOT, BOTRIGHT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, LEFTBOT, FARM); 
        break;
        allTiles[i]->addConnection(RIGHTMID, END, ROAD); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(LEFTMID, END, ROAD); 
      
      case 92: // X
        allTiles[i]->addPlacement(TOPMID, ROAD);
        allTiles[i]->addPlacement(RIGHTMID, ROAD);
        allTiles[i]->addPlacement(BOTMID, ROAD);
        allTiles[i]->addPlacement(LEFTMID, ROAD);
        allTiles[i]->addPlacement(RIGHTTOP, FARM);
        allTiles[i]->addPlacement(RIGHTBOT, FARM);
        allTiles[i]->addPlacement(BOTLEFT, FARM);
        allTiles[i]->addPlacement(LEFTTOP, FARM);
        allTiles[i]->addConnection(TOPMID, END, ROAD); 
        allTiles[i]->addConnection(RIGHTMID, END, ROAD); 
        allTiles[i]->addConnection(BOTMID, END, ROAD); 
        allTiles[i]->addConnection(LEFTMID, END, ROAD); 
        allTiles[i]->addConnection(TOPLEFT, LEFTTOP, FARM); 
        allTiles[i]->addConnection(TOPRIGHT, RIGHTTOP, FARM); 
        allTiles[i]->addConnection(RIGHTBOT, BOTRIGHT, FARM); 
        allTiles[i]->addConnection(BOTLEFT, LEFTBOT, FARM);
        break;

      default: 
        std::cerr << "Create Tiles: don't know how to handle index " << i << std::endl;
        exit(-1);
    }
  }

  // build their type caches
  for (size_t i = 0; i < allTiles.size(); i++) 
    allTiles[i]->buildTypeCache();
  
}

void createBag(tilelist_t & bag) {
  assert(allTiles.size() == 96);
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[0]); 
  for (size_t i = 0; i < 4; i++) bag.push_back(allTiles[4]); 
  for (size_t i = 0; i < 1; i++) bag.push_back(allTiles[8]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[12]); 
  for (size_t i = 0; i < 5; i++) bag.push_back(allTiles[16]); 
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[20]); 
  for (size_t i = 0; i < 1; i++) bag.push_back(allTiles[24]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[28]); 
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[32]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[36]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[40]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[44]); 
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[48]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[52]); 
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[56]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[60]); 
  for (size_t i = 0; i < 1; i++) bag.push_back(allTiles[64]); 
  for (size_t i = 0; i < 3; i++) bag.push_back(allTiles[68]); 
  for (size_t i = 0; i < 2; i++) bag.push_back(allTiles[72]); 
  for (size_t i = 0; i < 1; i++) bag.push_back(allTiles[76]); 
  for (size_t i = 0; i < 8; i++) bag.push_back(allTiles[80]); 
  for (size_t i = 0; i < 9; i++) bag.push_back(allTiles[84]); 
  for (size_t i = 0; i < 4; i++) bag.push_back(allTiles[88]); 
  for (size_t i = 0; i < 1; i++) bag.push_back(allTiles[92]); 
  //std::cerr << " size = " << bag.size() << std::endl;
}

