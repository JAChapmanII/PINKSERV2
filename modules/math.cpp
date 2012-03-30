#include "math.hpp"
using std::string;
using boost::smatch;
using global::ChatLine;

#include <iostream>
using std::endl;

#include <sstream>
using std::stringstream;

#include "util.hpp"
using util::contains;
using util::fromString;
using util::asString;

string BinaryLogFunction::run(ChatLine line, smatch matches) { // {{{
	return line.nick + ": " + asString(log(fromString<double>(matches[1])));
} // }}}
string BinaryLogFunction::name() const { // {{{
	return "lg";
} // }}}
string BinaryLogFunction::help() const { // {{{
	return "Returns log base 2";
} // }}}
string BinaryLogFunction::regex() const { // {{{
	return "^!lg\\s+(.*)";
} // }}}


string SetFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1];
	int value = fromString<int>(matches[2]);

	global::siMap[variable] = value;

	return line.nick + ": set " + variable + " to " + asString(value);
} // }}}
string SetFunction::name() const { // {{{
	return "set";
} // }}}
string SetFunction::help() const { // {{{
	return "Sets a variable to be an integer";
} // }}}
string SetFunction::regex() const { // {{{
	return "^!set\\s+(\\w+)\\s+(\\d+).*";
} // }}}


string EraseFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1];

	int ecount = global::siMap.erase(variable);
	if(ecount == 0)
		return line.nick + ": variable didn't exist anyway.";
	else
		return line.nick + ": erased " + variable;
} // }}}
string EraseFunction::name() const { // {{{
	return "erase";
} // }}}
string EraseFunction::help() const { // {{{
	return "Erases a variable";
} // }}}
string EraseFunction::regex() const { // {{{
	return "^!erase\\s+(\\w+)(\\s.*)?";
} // }}}


string ListFunction::run(ChatLine line, smatch matches) { // {{{
	string list;
	for(auto i : global::siMap)
		list += i.first + ", ";

	return list.substr(0, list.length() - 2);
} // }}}
string ListFunction::name() const { // {{{
	return "list";
} // }}}
string ListFunction::help() const { // {{{
	return "List stored variables";
} // }}}
string ListFunction::regex() const { // {{{
	return "^!list(\\s.*)?";
} // }}}


string ValueFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1];
	if(!contains(global::siMap, variable))
		return line.nick + ": that variable does not exist";

	return line.nick + ": " + variable + " is " +
		asString(global::siMap[variable]);
} // }}}
string ValueFunction::name() const { // {{{
	return "value";
} // }}}
string ValueFunction::help() const { // {{{
	return "Return the vaule of a store variable";
} // }}}
string ValueFunction::regex() const { // {{{
	return "^!value(\\s.*)?";
} // }}}


string IncrementFunction::run(ChatLine line, smatch matches) { // {{{
	// prefix operator
	string variable = matches[2];
	if(variable.empty())
		// postfix operator
		variable = matches[3];

	++global::siMap[variable];
	return line.nick + ": " + variable + " is now " +
		asString(global::siMap[variable]);
} // }}}
string IncrementFunction::name() const { // {{{
	return "++";
} // }}}
string IncrementFunction::help() const { // {{{
	return "Increment variable";
} // }}}
string IncrementFunction::regex() const { // {{{
	return "^\\s*(\\+\\+\\s*(\\w+)|(\\w+)\\s*\\+\\+)( .*)?";
} // }}}


string DecrementFunction::run(ChatLine line, smatch matches) { // {{{
	// prefix operator
	string variable = matches[2];
	if(variable.empty())
		// postfix operator
		variable = matches[3];

	--global::siMap[variable];
	return line.nick + ": " + variable + " is now " +
		asString(global::siMap[variable]);
} // }}}
string DecrementFunction::name() const { // {{{
	return "--";
} // }}}
string DecrementFunction::help() const { // {{{
	return "Decrement variable";
} // }}}
string DecrementFunction::regex() const { // {{{
	return "^\\s*(--\\s*(\\w+)|(\\w+)\\s*--)( .*)?";
} // }}}

