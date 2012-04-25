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

enum NodeType { Leaf, Urnary, Binary };

static map<string, vector<NodeType>> operators = {
	{ "++", { Urnary } }, { "--", { Urnary } },
	{ "+=", { Binary } }, { "-=", { Binary } },
	{ "*=", { Binary } }, { "/=", { Binary } },
	{ "%=", { Binary } },
	{  "=", { Binary } },
	{  "+", { Urnary, Binary } }, {  "-", { Urnary, Binary } },
	{  "*", { Binary } }, {  "/", { Binary } },
	{  "%", { Binary } }
};

#include <iostream>
using std::cerr;
using std::endl;

class AST {
	public:
		~AST() {
			if(this->m_type != Leaf)
				delete this->m_left;
			if(this->m_type == Binary)
				delete this->m_right;
		}

		string get_str() { // {{{
			switch(this->m_type) {
				case Leaf:
					return this->m_value;
				case Urnary:
					return (string)"(" + this->m_value + " " +
						this->m_left->get_str() + ")";
				case Binary:
					return (string)"(" + this->m_value + " " +
						this->m_left->get_str() + " " + this->m_right->get_str() + ")";
			}
			return "(oh-shit-what-happened!?)";
		} // }}}

		static unsigned oplen(string expr) { // {{{
			for(auto i : operators)
				if(startsWith(expr, i.first)) {
					cerr << "expr: " << expr << " matches " << i.first << endl;
					return i.first.length();
				}
			return 0;
		} // }}}

		static unsigned vlen(string expr) { // {{{
			unsigned olen = 0;
			for(unsigned i = 0; i < expr.length(); ++i) {
				olen = oplen(expr.substr(i));
				if(olen > 0)
					return i;
			}
			return expr.length();
		} // }}}

		static AST *parse(string expression);

	protected:
		NodeType m_type;
		string m_value;
		AST *m_left, *m_right;
};
AST *AST::parse(string expression) {
	expression = expression.substr(0, expression.find(";"));
	stack<AST *> nstack;
	stack<unsigned> openParenthesis;
	for(unsigned i = 0; i < expression.length(); ++i) {
		while((expression[i] == ' ') || (expression[i] == '\t'))
			++i;
		if(expression[i] == '(') {
			openParenthesis.push(i);
			continue;
		}
		if(expression[i] == ')') {
			if(openParenthesis.empty())
				throw (string)"Too many close parentheses at " + asString(i);
			openParenthesis.pop();
			continue;
		}
		unsigned olen = oplen(expression.substr(i));
		if(olen) {
			string op = expression.substr(i).substr(0, olen);
			vector<NodeType> optype;
			for(auto j : operators) {
				if(j.first == op) {
					optype = j.second;
					break;
				}
			}
			if(optype.empty())
				throw (string)"Unable to find operator: " + op;
			i += olen - 1;
		} else {
			unsigned varlen = vlen(expression.substr(i));
			i += varlen - 1;
		}
	}
	if(!openParenthesis.empty()) {
		if(openParenthesis.size() > 1)
			throw (string)"Multiple unclosed parentheses";
		else
			throw (string)"Unclosed parenthesis at " + asString(openParenthesis.top());
	}
	return NULL;
}

static map<string, Variable> variableMap;

string simplify(string expr) {
	vector<string> exprs = split(expr, ";"), sexprs;
	for(unsigned i = 0; i < exprs.size(); ++i) {
		try {
			AST *ast = AST::parse(exprs[i]);
			if(ast == NULL)
				sexprs.push_back("(NULL-AST-error)");
			else
				sexprs.push_back(ast->get_str());
		} catch(string &e) {
			throw (string)"in expression " + asString(i) + ": " + e;
		}
	}
	return join(sexprs, "; ");
}

/*
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
*/


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
string SetFunction::name() const { // {{{
	return "set";
} // }}}
string SetFunction::help() const { // {{{
	return "Sets a variable to be an integer";
} // }}}
string SetFunction::regex() const { // {{{
	return "^!set\\s+(\\w+)\\s+(\\S+).*";
} // }}}


string EraseFunction::run(ChatLine line, smatch matches) { // {{{
	string variable = matches[1];

	int ecount = variableMap.erase(variable);
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
	for(auto i : variableMap)
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
	string variable = matches[2];
	if(!contains(variableMap, variable))
		return line.nick + ": that variable does not exist";

	return line.nick + ": " + variable + " is " +
		variableMap[variable].get_str();
} // }}}
string ValueFunction::name() const { // {{{
	return "value";
} // }}}
string ValueFunction::help() const { // {{{
	return "Return the vaule of a store variable";
} // }}}
string ValueFunction::regex() const { // {{{
	return "^!value(\\s(.+))";
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

	variableMap[variable].decrement();
	return line.nick + ": " + variable + " is now " +
		variableMap[variable].get_str();
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


string MathFunction::run(ChatLine line, smatch matches) { // {{{
	try {
		return line.nick + ": " + simplify(matches[1]);
	} catch(string &e) {
		return line.nick + ": " + e;
	}
} // }}}
string MathFunction::name() const { // {{{
	return "math";
} // }}}
string MathFunction::help() const { // {{{
	return "Evalute some math";
} // }}}
string MathFunction::regex() const { // {{{
	return "^!math\\s+(.+)";
} // }}}

