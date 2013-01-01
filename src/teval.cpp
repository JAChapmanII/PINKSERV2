#include <map>
using std::map;
using std::pair;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <algorithm>
using std::transform;
#include <stack>
using std::stack;
#include <cctype>

#include <iostream>
using std::cout;
using std::endl;

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
	global::vars["bot.admins"] = "Jext, RGCockatrices, bonzairob, quairlzr";
	for(int i = 1; i < argc; ++i) {
		if(string(argv[i]).empty())
			continue;
		cout << i << ": " << argv[i] << endl;

		ExpressionTree *etree = NULL;
		try {
			//Permissions p = Permissions::parse(argv[i]);
			vector<TokenFragment> tfv = TokenFragment::fragment(argv[i]);
			for(TokenFragment tf : tfv)
				cout << (tf.special ? "_" : "") << tf.text
					<< (tf.special ? "_" : "") << " ";
			cout << endl;

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
	return 0;
}

