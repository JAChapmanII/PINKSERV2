#include "math.hpp"
using std::string;

#include <iostream>
using std::endl;

#include <sstream>
using std::stringstream;

#include "util.hpp"
using util::contains;

string BinaryLogFunction::run(FunctionArguments fargs) { // {{{
	string str = fargs.matches[1];
	stringstream ss;
	ss << str;
	double val = 0;
	ss >> val;
	stringstream outss;
	outss << fargs.nick << ": " << log(val) << endl;
	return outss.str();
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


string SetFunction::run(FunctionArguments fargs) { // {{{
	string varName = fargs.matches[1];
	stringstream is(fargs.matches[2]);
	int value = 0;
	is >> value;

	(*fargs.siMap)[varName] = value;

	stringstream ss;
	ss << "Set " << varName << " to " << value;
	return ss.str();
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


string EraseFunction::run(FunctionArguments fargs) { // {{{
	string varName = fargs.matches[1];

	int ecount = (*fargs.siMap).erase(varName);
	if(ecount == 0)
		return "Variable didn't exist anyway.";
	else
		return "Erased " + varName;
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


string ListFunction::run(FunctionArguments fargs) { // {{{
	stringstream ss;
	unsigned j = 0, last = (*fargs.siMap).size() - 1;
	for(auto i : (*fargs.siMap)) {
		ss << i.first;
		if(j != last)
			ss << ", ";
		++j;
	}

	return ss.str();
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


string ValueFunction::run(FunctionArguments fargs) { // {{{
	string var = fargs.matches[1];
	if(!contains((*fargs.siMap), var))
		return "That variable does not exist";

	stringstream ss;
	ss << var << " == " << (*fargs.siMap)[var];
	return ss.str();
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


string IncrementFunction::run(FunctionArguments fargs) { // {{{
	// prefix operator
	string varName = fargs.matches[2];
	if(varName.empty())
		// postfix operator
		varName = fargs.matches[3];

	stringstream ss;
	ss << varName << " is now " << (++((*fargs.siMap)[varName]));
	return ss.str();
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


string DecrementFunction::run(FunctionArguments fargs) { // {{{
	// prefix operator
	string varName = fargs.matches[2];
	if(varName.empty())
		// postfix operator
		varName = fargs.matches[3];

	stringstream ss;
	ss << varName << " is now " << (--((*fargs.siMap)[varName]));
	return ss.str();
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

