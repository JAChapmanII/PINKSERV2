#include <iostream>
using std::cout;
using std::cin;
#include <string>
using std::string;

#include "modules.hpp"
#include "config.hpp"
#include "markov.hpp"

int main(int argc, char **argv) {
	// initialize modules
	modules::init(config::brainFileName);

	if(argc > 1 && (string)"--dump" == argv[1])
		dumpMarkov(cout);
	if(argc > 1 && (string)"--read" == argv[1])
		readMarkov(cin);

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	return 0;
}

