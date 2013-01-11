#include "variable.hpp"
using std::string;
using std::vector;
using std::map;

#include <algorithm>
using std::transform;

#include "util.hpp"
using util::split;
using util::trimWhitespace;

vector<string> makeList(string lists) {
	vector<string> list = split(lists, ",");
	transform(list.begin(), list.end(), list.begin(), trimWhitespace);
	return list;
}

vector<string> getList(map<string, string> vars, string variable) {
	string lists = vars[variable];
	return makeList(lists);
}

