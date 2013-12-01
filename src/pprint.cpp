#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "expression.hpp"
#include "parser.hpp"

int main(int argc, char **argv) {
	for(int i = 1; i < argc; ++i) {
		cout << argv[i] << endl;
		try {
			auto et = parse(argv[i]);
			cout << et->pretty(' ', 4) << endl;
		} catch(ParseException e) {
			cerr << e.pretty() << endl;
		}
	}

	return 0;
}

