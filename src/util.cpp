#include "util.hpp"
using std::vector;
using std::string;

vector<string> util::split(string str, string on) { // {{{
	vector<string> fields;
	size_t fsep = 0;
	while((fsep = str.find_first_of(on)) != string::npos) {
		fields.push_back(str.substr(0, fsep));
		str = str.substr(fsep + 1);
	}
	if(!str.empty())
		fields.push_back(str);
	return fields;
} // }}}
string util::join(vector<string> strs, string with) { // {{{
	if(strs.empty())
		return "";
	string res;
	for(string str : strs)
		res += str + with;
	return res.substr(0, res.length() - with.length());
} // }}}

vector<string> util::subvector(vector<string> vec, size_t s, size_t n) { // {{{
	vector<string> res;
	if(s >= vec.size())
		return res;
	if(s + n >= vec.size())
		n = vec.size() - s;
	res.insert(res.begin(), vec.begin() + s, vec.begin() + s + n);
	return res;
} // }}}

