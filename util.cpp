#include "util.hpp"

#include <sstream>
#include <cstdlib>

#if LINUX

#include <sys/timeb.h>
#include <unistd.h>

unsigned int currentMillis() {
	struct timeb tb; 
	ftime(&tb); 
	return static_cast<unsigned int>(tb.time*1000 + tb.millitm); 
}

void mysleep(int seconds) {
  sleep(seconds);
}

unsigned long long unif_rand_64() {
  unsigned long long x = static_cast<unsigned long long>(drand48()*18446744073709551615ULL); 
  return x; 
}

void seed_rng(unsigned int s) {
  srand48(s); 
  srand(s); 
}

double unifrand01() { 
  return drand48();
}

unsigned int myrand() {
  //return static_cast<unsigned int>(unifrand01() * 4294967295UL);
  return static_cast<unsigned int>(rand());
}

#else 

#include <windows.h>

unsigned int currentMillis() {
	DWORD t = GetTickCount();
	return static_cast<unsigned int>(t); 
}

void mysleep(int seconds) {
  std::cerr << "PLEASE IMPLEMENT mysleep() in util.cpp !" << std::endl;
  exit(-1);
}

unsigned long long unif_rand_64() {
  std::cerr << "Implement unif_rand_64!" << std::endl;
  exit(-1); 
}

/* returns a uniform random number between 0 and 1, inclusively */
double unifrand01() { 
  // I don't know if there is a good replacement for drand48() on Windows. This
  // can likely be improved, especially since 1 is not usually included as part
  // of the interval, but I felt RAND_MAX+1 wasn't right either. 
  return double(rand())/double(RAND_MAX); 
}

#endif // LINUX

bool contains(const std::vector<int> & list, int elem) {
  for (size_t i = 0; i < list.size(); i++)
    if (list[i] == elem)
      return true;

  return false;
}

bool contains(const std::vector<size_t> & list, size_t elem) {
  for (size_t i = 0; i < list.size(); i++)
    if (list[i] == elem)
      return true;

  return false;
}


int to_int(std::string & str) {
  std::stringstream stmT;
  int iR;

  stmT << str;
  stmT >> iR;

  return iR;
}

size_t to_size_t(std::string & str) {
  std::stringstream stmT;
  size_t iR;

  stmT << str;
  stmT >> iR;

  return iR;
}

/* tokenizes a string */
void split(std::vector<std::string> & tokens, const std::string &line, char delimiter)
{
  std::string::size_type index = 0;

  while (index < line.length())
  {
    std::string::size_type new_index = line.find(delimiter, index);

    if (new_index == std::string::npos)
    {
      tokens.push_back(line.substr(index));
      break;
    }
    else
    {
      tokens.push_back(line.substr(index, new_index - index));
      index = new_index+1;
    }
  }

  // special case with token as the last character
  if (index == line.length())
    tokens.push_back("");
}

