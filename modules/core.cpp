#include "core.hpp"
using std::vector;
using std::string;

#include <random>
using std::uniform_int_distribution;
using std::uniform_real_distribution;

#include "global.hpp"
#include "util.hpp"
using util::fromString;
using util::asString;

string echo(vector<string> arguments) {
	string res;
	for(string arg : arguments) {
		// TODO: old semantics of echo?
		if(!res.empty() && (!isspace(res.back()) && !isspace(arg[0])))
			res += " ";
		res += arg;
	}
	// TODO: this should really just append to some other real return
	// TODO: this is broken now >_>
	//if(this->next && all)
		//return argsstr + this->next->evaluate(nick);
	return res;
}

string core_or(vector<string> arguments) {
	uniform_int_distribution<> uid(0, arguments.size() - 1);
	unsigned target = uid(global::rengine);
	return arguments[target];
}

string rand(vector<string> arguments) {
	if(arguments.size() != 2)
		throw (string)"rand takes two parameters; the bounds";
	long low = fromString<long>(arguments[0]),
		high = fromString<long>(arguments[1]);
	if(low > high)
		throw (string)"rand's second parameter must be larger";
	if(low == high)
		return asString(low);
	uniform_int_distribution<long> lrng(low, high);
	return asString(lrng(global::rengine));
}

string drand(vector<string> arguments) {
	if(arguments.size() != 2)
		throw (string)"drand takes two parameters; the bounds";
	double low = fromString<double>(arguments[0]),
			high = fromString<double>(arguments[1]);
	if(low > high)
		throw (string)"drand's second parameter must be larger";
	uniform_real_distribution<double> lrng(low, high);
	return asString(lrng(global::rengine));
}

