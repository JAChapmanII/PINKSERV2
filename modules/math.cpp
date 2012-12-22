#include "math.hpp"
using std::string;
using boost::smatch;

#include <iostream>
using std::endl;

#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <stack>
using std::stack;

#include <exception>
using std::exception;

#include <gmpxx.h>

#include "global.hpp"

#include "util.hpp"
using util::contains;
using util::startsWith;
using util::fromString;
using util::asString;
using util::split;
using util::join;

string simplify(string expr);

enum VariableType { Integer, Rational };

struct Variable {
	VariableType type;
	mpz_class integer;
	mpf_class rational;

	~Variable() { }
	Variable(const Variable &rhs) { // {{{
		this->type = rhs.type;
		this->integer = rhs.integer;
		this->rational = rhs.rational;
	} // }}}
	Variable() : type(Integer) { // {{{
		this->integer = 0;
	} // }}}

	Variable &operator=(const string &rhs) { // {{{
		this->type = Integer;
		string tmp = rhs;
		if(tmp.find(".") != string::npos)
			this->type = Rational;
		if(tmp[tmp.length() - 1] == 'r') {
			this->type = Rational;
			tmp = tmp.substr(0, tmp.length() - 1);
			
		}
		switch(this->type) {
			case Integer:
				this->integer = tmp;
				break;
			case Rational:
				this->rational = tmp;
				break;
			default:
				break;
		}
	} // }}}

	string get_str() { // {{{
		switch(this->type) {
			case Integer:
				return this->integer.get_str();
			case Rational: {
				mp_exp_t exponent;
				string tmp = this->rational.get_str(exponent) + "e";
				return tmp.substr(0, 1) + "." + tmp.substr(1) + asString(exponent - 1);
			}
			default:
				break;
		}
	} // }}}

	void decrement() { // {{{
		switch(this->type) {
			case Integer:
				this->integer -= 1;
				break;
			case Rational:
				this->rational -= 1.0;
				break;
			default:
				break;
		}
	} // }}}
	void increment() { // {{{
		switch(this->type) {
			case Integer:
				this->integer += 1;
				break;
			case Rational:
				this->rational += 1.0;
				break;
			default:
				break;
		}
	} // }}}
};

static map<string, Variable> variableMap;

SetFunction::SetFunction() : Function( // {{{
		"set", "Sets a variable to be an integer", "^!set\\s+(\\w+)\\s+(\\S+).*") {
} // }}}
string SetFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1], value = matches[2];
	try {
		variableMap[variable] = value;
	} catch(exception &e) {
		global::err << "set: exception: " << e.what() << std::endl;
		return line.nick + ": wuh-oh: " + e.what();
	}

	return line.nick + ": set " + variable + " to " +
		variableMap[variable].get_str();
} // }}}


EraseFunction::EraseFunction() : Function( // {{{
		"erase", "Erases a variable", "^!erase\\s+(\\w+)(\\s.*)?") {
} // }}}
string EraseFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1];

	int ecount = variableMap.erase(variable);
	if(ecount == 0)
		return line.nick + ": variable didn't exist anyway.";
	else
		return line.nick + ": erased " + variable;
} // }}}


ListFunction::ListFunction() : Function( // {{{
		"list", "List stored variables", "^!list(\\s.*)?") {
} // }}}
string ListFunction::run(ChatLine line, smatch matches) { // {{{
	string list;
	for(auto i : variableMap)
		list += i.first + ", ";

	return list.substr(0, list.length() - 2);
} // }}}


ValueFunction::ValueFunction() : Function( // {{{
		"value", "Return the vaule of a store variable", "^!value(\\s(.+))") {
} // }}}
string ValueFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[2];
	if(!contains(variableMap, variable))
		return line.nick + ": that variable does not exist";

	return line.nick + ": " + variable + " is " +
		variableMap[variable].get_str();
} // }}}


IncrementFunction::IncrementFunction() : Function( // {{{
		"++", "Increment variable",
		"^\\s*(\\+\\+\\s*(\\w+)|(\\w+)\\s*\\+\\+)( .*)?") {
} // }}}
string IncrementFunction::run(ChatLine line, smatch matches) { // {{{
	// prefix operator
	string variable = matches[2];
	if(variable.empty())
		// postfix operator
		variable = matches[3];

	variableMap[variable].increment();
	return line.nick + ": " + variable + " is now " +
		variableMap[variable].get_str();
} // }}}


DecrementFunction::DecrementFunction() : Function( // {{{
		"--", "Decrement variable", "^\\s*(--\\s*(\\w+)|(\\w+)\\s*--)( .*)?") {
} // }}}
string DecrementFunction::run(ChatLine line, smatch matches) { // {{{
	// prefix operator
	string variable = matches[2];
	if(variable.empty())
		// postfix operator
		variable = matches[3];

	variableMap[variable].decrement();
	return line.nick + ": " + variable + " is now " +
		variableMap[variable].get_str();
} // }}}

