#include "util.hpp"
using std::vector;
using std::string;

vector<string> util::split(string str, string on) { // {{{
	vector<string> results;
	size_t first = str.find_first_not_of(on);
	while(first != string::npos) {
		size_t last = str.find_first_of(on, first + 1);
		results.push_back(str.substr(first, last - first));
		if(last == string::npos)
			break;
		first = str.find_first_not_of(on, last + 1);
	}
	return results;
} // }}}
string util::join(vector<string> strs, string with) { // {{{
	string res;
	for(string str : strs)
		res += str + with;
	return res.substr(0, res.length() - with.length());
} // }}}

