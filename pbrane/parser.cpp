#include "parser.hpp"
using std::string;
using std::unique_ptr;

#include <map>
using std::map;

#include <vector>
using std::vector;

#include "expression.hpp"


/* reorginize
 * by default,
 * 	only !function calls exist (either all params are grouped into one
 * 		string, or are all split?) -- no evaluation of arguments
 * ${} context,
 * 	old style, evaluate everything and must have proper syntax
 * ${} can be used as operand of default context, will evaluate. \$ can be
 * used to embed a ${} in the default context without actual evaluation. It is
 * not needed but harmless if there won't be a { following the $.
 *
 * if a default context call needs more than one argument (or needs strings
 * grouped correctly), then ${"the words"} can be used, as by default it will
 * simply return the string. If needed multiple times, it will probably be
 * easier for the user to move the function call inside the ${}.
 *
 * default context does not allow ; to separate commands.
 *
 * ${} context function binding will be changed to be lower precedence than
 * even ;, so that ${f => v = 1; !echo v; } will bind "v = 1; !echo v;" as
 * the function body. If function binding should not consume entire line, then
 * it will need to be put in () or {}
 *
 * hardcoded functions can still parse arguments how they expect them, and all
 * user-defined functions will be made through the ${funcName => body} binding
 * anyway.
 *
 * add actual list-y datatype? $arg[1] will return second thing? if string,
 * second char, otherwise second list element? Datatype in list can be
 * completely generic and non-uniform. [elem, elem2, ...] to create list?
 * $arg[1..3] for slicing? use $ as end as in D? Enables functional
 * programming a lot better...
 */

unique_ptr<Expression> Parser::parse(string str) {
	Parser p(str);
	return p.parse();
}
unique_ptr<Expression> Parser::parseCanonical(string str) {
	Parser p(str);
	return p.parseCanonical();
}

unique_ptr<Expression> parse(string str) {
	return Parser::parse(str);
}

static map<string, OpInfo> opMap = {
	{ "=>", { 1, false } },
	{ ";", { 2 } },
	{ "?", { 3 } },
	{ ":", { 4 } }, // TODO: make lower? currently is higher? ? lower than :
	{ "=", { 5, false } },
	{ "+=", { 5, false } }, { "-=", { 5, false } },
	{ "*=", { 5, false } }, { "/=", { 5, false } }, { "%=", { 5, false } },
	{ "~=", { 5, false } }, { "^=", { 5, false } },
	{ "&=", { 5, false } }, { "|=", { 5, false } },
	{ "||", { 6 } },
	{ "&&", { 7 } },
	{ "==", { 8 } }, { "!=", { 8 } }, { "=~", { 8 } },
	{ "<", { 9 } }, { "<=", { 9 } }, { ">", { 9 } }, { ">=", { 9 } },
	{ "~", { 10 } },
	{ "+", { 11 } }, { "-", { 11 } },
	{ "*", { 12 } }, { "/", { 12 } }, { "%", { 12 } },
	{ "^", { 13 } }
};

#include <iostream>
using std::cerr;
using std::endl;
unique_ptr<Expression> Parser::parse() {
	return parseDefaultContext();
}
unique_ptr<Expression> Parser::parseCanonical() {
	return parseExpression();
}

void Parser::ignoreWhiteSpace() {
	while(isspace(_str[_idx]))
		_idx++;
}
void Parser::expect(string needle) {
	if(!is(needle))
		except("expected '" + needle + "'");
	_idx += needle.length();
}
void Parser::except(string err) {
	throw ParseException(_str, err, _idx);
}
bool Parser::is(string needle) const {
	return (_str.substr(_idx, needle.length()) == needle);
}
bool Parser::atEnd() const {
	return _len == _idx;
}

unique_ptr<Expression> Parser::parseDefaultContextValue() {
	ignoreWhiteSpace();
	string str;
	vector<unique_ptr<Expression>> args;
	for(; !atEnd() && !isspace(_str[_idx]);) {
		if(is("${")) {
			if(!str.empty())
				args.emplace_back(new Expression("str", str));
			str.clear();
			args.push_back(parseSingleExpression());
		} else {
			if(_str[_idx] == '\\') // consume backslash and next char
				str += _str[_idx++];
			str += _str[_idx++];
		}
	}
	if(!str.empty())
		args.emplace_back(new Expression("str", str));
	str.clear();

	// simple string, or only a ${} (which is really just an expression...)
	if(args.size() == 1)
		return move(args[0]);

	// compound string (may be multi-expressioned...)
	return unique_ptr<Expression>(new Expression("strcat", args));
}
unique_ptr<Expression> Parser::parseVariableName() {
	// TODO: basically dupe of above
	ignoreWhiteSpace();
	string str;
	vector<unique_ptr<Expression>> args;
	// TODO: better way to decide end points?
	for(; !atEnd() && (_str[_idx] == '$' || _str[_idx] == '.' || isalnum(_str[_idx]));) {
		if(is("${")) {
			if(!str.empty())
				args.emplace_back(new Expression("str", str));
			str.clear();
			args.push_back(parseExpression());
		} else {
			if(_str[_idx] == '\\') // consume backslash and next char
				str += _str[_idx++];
			str += _str[_idx++];
		}
	}
	if(!str.empty())
		args.emplace_back(new Expression("str", str));
	str.clear();

	// simple string, or only a ${} (which is really just an expression...)
	if(args.size() == 1)
		return move(args[0]);

	// compound string (may be multi-expressioned...)
	return unique_ptr<Expression>(new Expression("strcat", args));
}

unique_ptr<Expression> Parser::parseDefaultContext() {
	ignoreWhiteSpace();
	switch(_str[_idx]) {
		case '!':
			return parseDefaultContextFunction();
			break;
		case '$':
			if(!is("${"))
				except("expected { after $");
			return parseExpression();
			break;
		default:
			break;
	}
	except("encountered unexpected '" + string(1, _str[_idx]) + "'");
	return nullptr;
}
unique_ptr<Expression> Parser::parseDefaultContextFunction() {
	expect("!");
	if(isspace(_str[_idx]))
		except("unexpected space following ! call");

	// function name comes first, but is also simply default context value
	vector<unique_ptr<Expression>> args;
	while(!atEnd())
		args.push_back(parseDefaultContextValue());

	if(args.empty())
		except("expected function name after !");
	// TODO: other context expects "var" not "str"
	unique_ptr<Expression> fname = move(args[0]);
	args[0] = unique_ptr<Expression>(new Expression("var"));
	args[0]->args.push_back(move(fname));

	return unique_ptr<Expression>(new Expression("!", args));
}
unique_ptr<Expression> Parser::parseSingleExpression() {
	expect("${");
	unique_ptr<Expression> res = parseExpression();
	expect("}");
	return res;
}
unique_ptr<Expression> Parser::parseExpression(int cLevel) {
	ignoreWhiteSpace();
	if(is(")") || is("}"))
		return nullptr; // TODO?
	unique_ptr<Expression> left{nullptr};
	if(is("${")) {
		expect("${");
		left.reset(new Expression("${}"));
		left->args.push_back(parseExpression());
		expect("}");
	} else if(is("!")) {
		left = parseFunctionCall();
	} else if(is("(") || is("{")) {
		string start = string(1, _str[_idx]);
		static map<string, string> close{ { "(", ")" }, { "{", "}" } };

		expect(start);
		left.reset(new Expression(start + close[start]));
		left->args.push_back(parseExpression());
		expect(close[start]);
	} else if(is("+") || is("-") || isdigit(_str[_idx])) {
		left = parseNumber();
	} else if(is("'") || is("\"")) {
		left = parseString();
	} else if(is("*")) {
		left = parseVariableAccess();
	} else {
		left = parseVariableAccess();
	}

	int nextPrec = nextPrecedence();
	while(nextPrec >= cLevel) {
		unique_ptr<Expression> op = getBinaryOp(),
			right = parseExpression(nextPrec +
					(leftAssociative(op->type) ? 1 : 0));
		if(!right && op->type != ";")
			except("expected expression after " + op->type);

		op->args.push_back(move(left));
		op->args.push_back(move(right));
		left = move(op);

		nextPrec = nextPrecedence();
	}

	return left;
}
unique_ptr<Expression> Parser::parseVariableAccess() {
	ignoreWhiteSpace();
	// there's a level of indirection
	if(is("*")) {
		expect("*");
		unique_ptr<Expression> va{new Expression("var")},
			vname = parseVariableAccess();
		va->args.push_back(move(vname));
		return va;
	}
	unique_ptr<Expression> va{new Expression("var")},
		vname = parseVariableName();
	va->args.push_back(move(vname));
	return va;
}
unique_ptr<Expression> Parser::parseFunctionCall() {
	expect("!");

	vector<unique_ptr<Expression>> args;
	args.push_back(parseVariableAccess());
	// all args are expressions
	// TODO: proper ending?
	while(ignoreWhiteSpace(), !atEnd() && !is("}") && !is(")") && !is(";"))
		args.push_back(parseExpression());

	if(args.empty())
		except("expected function name after !");

	return unique_ptr<Expression>(new Expression("!", args));
}

unique_ptr<Expression> Parser::parseString() {
	ignoreWhiteSpace();
	string delimiter(1, _str[_idx]);

	expect(delimiter);

	string str;
	vector<unique_ptr<Expression>> args;
	for(; !is(delimiter); ) {
		if(atEnd())
			except("expected string delimiter: " + delimiter);

		if(is("${")) {
			if(!str.empty())
				args.push_back(unique_ptr<Expression>(new Expression("str", str)));
			str.clear();
			args.push_back(parseExpression());
		} else {
			if(is("\\"))
				str += _str[_idx++];
			str += _str[_idx++];
		}
	}
	if(!str.empty())
		args.push_back(unique_ptr<Expression>(new Expression("str", str)));
	str.clear();

	expect(delimiter);

	// an empty string
	if(args.empty())
		return unique_ptr<Expression>(new Expression(delimiter, ""));

	// a complex string
	return unique_ptr<Expression>(new Expression(delimiter, args));
}

string Parser::grabNumber() {
	string num;
	while(!atEnd() && isdigit(_str[_idx]))
		num += _str[_idx++];
	return num;
}
unique_ptr<Expression> Parser::parseNumber() {
	ignoreWhiteSpace();
	string num;
	if(is("+")) {
		num += "+";
		expect("+");
	} else if(is("-")) {
		num += "-";
		expect("-");
	}

	num += grabNumber();
	if(is(".")) {
		num += ".";
		expect(".");
		num += grabNumber();
	}
	return unique_ptr<Expression>(new Expression("num", num));
}
// function calls should always work the same way? So you can't do
// !func 5 + 5 7 * 7
// you would need !func ${5 + 5} ${7 * 7}
// ????
string Parser::nextOp() {
	ignoreWhiteSpace();
	for(int i = 3; i > 0; --i)
		if(opMap.find(_str.substr(_idx, i)) != opMap.end())
			return _str.substr(_idx, i);
	return "";
}
bool Parser::leftAssociative(string op) const {
	if(op.empty() || opMap.find(op) == opMap.end())
		return true;
	return opMap[op].leftAssociative;
}
int Parser::precedence(string op) const {
	if(op.empty() || opMap.find(op) == opMap.end())
		return -1;
	return opMap[op].precedence;
}
int Parser::nextPrecedence() {
	return precedence(nextOp());
}
unique_ptr<Expression> Parser::getBinaryOp() {
	string op = nextOp();
	if(op.empty())
		except("expected binary operator");

	expect(op);
	return unique_ptr<Expression>(new Expression(op));
}


