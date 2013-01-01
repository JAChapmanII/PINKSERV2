#include <map>
using std::map;
using std::pair;
#include <string>
using std::string;
using std::getline;
#include <vector>
using std::vector;
#include <algorithm>
using std::transform;
#include <stack>
using std::stack;
#include <random>
using std::random_device;
#include <cctype>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::cin;

#include "util.hpp"
using util::contains;
using util::startsWith;
using util::asString;
using util::fromString;

#include "permission.hpp"
#include "variable.hpp"
#include "global.hpp"
#include "expressiontree.hpp"

void execute(string statement) {
	// tmp variable map
	map<string, string> tvars;
}

int main(int argc, char **argv) {
	global::vars["bot.owner"] = "jac";
	global::vars["bot.admins"] = "Jext, RGCockatrices, bonzairob, quairlzr, Nybbles, ajanata";

	random_device randomDevice;
	unsigned int seed = randomDevice();
	global::init(seed);

	if(argc > 1) {
		for(int i = 1; i < argc; ++i) {
			if(string(argv[i]).empty())
				continue;
			cout << i << ": " << argv[i] << endl;

			ExpressionTree *etree = NULL;
			try {
				etree = ExpressionTree::parse(argv[i]);

				// print computed AST
				cout << "final: " << endl;
				etree->print();
				cout << "stringify: " << etree->toString() << endl;

				cout << "result: " << etree->evaluate("jac") << endl;
			} catch(string &s) {
				cout << "\t: " << s << endl;
			}

			delete etree;
		}
	} else {
		while(cin.good() && !cin.eof()) {
			string nick, line;
			getline(cin, nick);
			getline(cin, line);
			if(nick.empty() || line.empty())
				break;

			ExpressionTree *etree = NULL;
			try {
				etree = ExpressionTree::parse(line);

				cerr << "eval'ing: " << line << " as " << nick << endl;
				cerr << "final AST: " << endl;
				etree->print();
				cerr << "stringify: " << etree->toString() << endl;

				string res = etree->evaluate(nick);
				cerr << "result: " << res << endl;
				cout << nick + ": " << res << endl;
			} catch(string &s) {
				cerr << "\texception: " << s << endl;
				cout << nick + ": error: " + s << endl;
			}
			delete etree;
		}
	}
	return 0;
}

