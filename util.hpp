#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <vector>
#include <cstdlib>
#include <cassert>
#include <string>

#define MAX(a,b)     ((a) > (b) ? (a) : (b))
#define MIN(a,b)     ((a) < (b) ? (a) : (b))
#define ABS(x)       ((x) < 0.0 ? (-(x)) : (x))
#define EQZERO(x)    (ABS(x) < 0.000000000001) 

typedef std::vector<std::vector<size_t> > turnoutcome_list_t; 

// returns true if the vector contains the specified element
bool contains(const std::vector<int> & list, int elem);
bool contains(const std::vector<size_t> & list, size_t elem);

// number of milliseconds since linux? epoch : windows? last boot 
extern unsigned int currentMillis(); 

// OS-dependent
void mysleep(int seconds);

// returns a uniform random number between 0 and 1, inclusively
double unifrand01(); 

/* tokenizes a string */
void split(std::vector<std::string> & tokens, const std::string &line, char delimiter);

int to_int(std::string & str);
size_t to_size_t(std::string & str);

void seed_rng(unsigned int); 
unsigned int myrand();
unsigned long long unif_rand_64();

#endif // __UTIL_HPP__
