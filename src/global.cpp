#include "global.hpp"
using std::string;
using std::to_string;
using std::vector;
using std::map;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;

#include "config.hpp"
#include "util.hpp"
using util::contains;
using util::fromString;
#include "expression.hpp"
#include "parser.hpp"

vector<string> global::moduleFunctionList;


vector<string> global::ignoreList;
unsigned global::minSpeakTime = 0;


