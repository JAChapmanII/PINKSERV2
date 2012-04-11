#include "util.hpp"
using std::vector;
using std::string;
#include <unistd.h>

vector<string> util::split(string str, string on) { // {{{
	vector<string> fields;
	// there are no separators
	if(str.find_first_of(on) == string::npos) {
		fields.push_back(str);
		return fields;
	}
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
vector<string> util::last(vector<string> vec, size_t n) {
	if(n > vec.size())
		n = vec.size();
	return subvector(vec, vec.size() - n, n);
}

string util::trim(string str, string what) {
	size_t firstNotOf = str.find_first_not_of(what);
	if(firstNotOf == string::npos)
		return "";
	str = str.substr(firstNotOf);
	return str.substr(0, str.find_last_not_of(what) + 1);
}

bool util::file::exists(string filename) {
	return (access(filename.c_str(), F_OK) == 0);
}
bool util::file::executable(string filename) {
	return (access(filename.c_str(), X_OK) == 0);
}

