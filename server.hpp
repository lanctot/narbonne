#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <string>
#include <cstdlib>

struct ServerParameters {

	// setup defaults
	ServerParameters() {
		seed   = 0; 
    seedSpecified = false;
    ssfile = "";  // starting state file
    runname = "default";
	}

	// initialise paramaters from the commmand line, false on failure
	bool init(int argc, const char *argv[]) {

		if (argc == 2) return false;

		for (int i=1; i < argc; i++) {
			std::string s = argv[i];
			if      (s == "-seed") { seed = atoi(argv[++i]); seedSpecified = true; }
			else if (s == "-ssfile") ssfile = argv[++i]; 
      else if (s == "-runname") runname = argv[++i];
			else return false;
		}

		return sane();
	}

	// are the command line parameter values sane?
	bool sane() const {
		
		// I'm sure this will get filled at some point
		return true;
	}

  bool seedSpecified;
	int seed;  
  std::string ssfile;
  std::string runname;
};


// global configuration options
extern ServerParameters gbl_srvopt;

#endif // __SERVER_HPP__
