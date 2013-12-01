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
#include "expression.hpp"
#include "parser.hpp"
#include "modules.hpp"
#include "config.hpp"

int main(int argc, char **argv) {
	global::vars["bot.owner"] = "jac";
	global::vars["bot.admins"] = "Jext, RGCockatrices, bonzairob, quairlzr, Nybbles, ajanata";
	global::vars["bot.maxIterations"] = "10";

	// initialize modules
	modules::init(config::brainFileName);

	random_device randomDevice;
	unsigned int seed = randomDevice();
	global::init(seed);

	if(argc > 1) {
		for(int i = 1; i < argc; ++i) {
			if(string(argv[i]).empty())
				continue;
			cout << i << ": " << argv[i] << endl;

			try {
				auto expr = Parser::parse(argv[i]);
				cout << "expr: " << (expr ? "true" : "false") << endl;

				// print computed AST
				cout << "final: " << endl;
				cout << expr->pretty() << endl;
				cout << "stringify: " << expr->toString() << endl;

				cout << "result: " << expr->evaluate("jac").toString() << endl;
				// TODO: other exception types...
			} catch(ParseException e) {
				cout << e.pretty() << endl;
			} catch(StackTrace e) {
				cout << e.toString() << endl;
			} catch(string &s) {
				cout << "\t: " << s << endl;
			}
		}
	} else {
		while(cin.good() && !cin.eof()) {
			string nick, line;
			getline(cin, nick);
			getline(cin, line);
			if(nick.empty() || line.empty())
				break;

			try {
				auto expr = Parser::parse(line);

				cerr << "eval'ing: " << line << " as " << nick << endl;
				cerr << "final AST: " << endl;
				cerr << expr->pretty() << endl;
				cerr << "stringify: " << expr->toString() << endl;

				string res = expr->evaluate(nick).toString();
				cerr << "result: " << res << endl;
				cout << nick + ": " << res << endl;

				// TODO: other exception types
			} catch(ParseException e) {
				cerr << e.pretty() << endl;
			} catch(StackTrace e) {
				cout << e.toString() << endl;
			} catch(string &s) {
				cerr << "\texception: " << s << endl;
				cout << nick + ": error: " + s << endl;
			}
		}
	}

	// free memory associated with modules
	modules::deinit(config::brainFileName);

	return 0;

}

