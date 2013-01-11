#include "modules.hpp"
#include "config.hpp"

int main(int argc, char **argv) {
	// initialize modules
	modules::init(config::brainFileName);

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	return 0;
}

