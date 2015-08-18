#include "expression.hpp"
using std::vector;
using std::string;
using std::unique_ptr;

#include <memory>
using std::move;

#include <cmath>

#include "permission.hpp"

#include "util.hpp"
using util::asString;
using util::join;
using util::contains;

#include "regex.hpp"

#include "global.hpp"
#include "modules.hpp"

#include "parser.hpp"


#include <iostream>
using std::cerr;
using std::endl;


Expression::Expression(string itype, string itext) :
		type(itype), text(itext) {
}
Expression::Expression(string itype, vector<unique_ptr<Expression>> &iargs) :
		type(itype) {
	for(unique_ptr<Expression> &arg : iargs)
		args.push_back(move(arg));
	iargs.clear();
}
Expression::Expression(const Expression &rhs) : type(rhs.type), text(rhs.text) {
	for(auto &i : rhs.args)
		this->args.emplace_back(new Expression(*(i.get())));
}

string Expression::toString() const {
	if(this == nullptr)
		return "[null]";
	if(this->type == "str" || this->type == "num")
		return this->text;
	if(this->type == "\"" || this->type == "'") {
		string res = this->type;
		for(auto &arg : this->args)
			res += arg->toString();
		return res + this->type;
	}
	if(this->type == "!") {
		string res = this->type + this->args[0]->toString();
		for(int i = 1; i < this->args.size(); ++i)
			res += " " + this->args[i]->toString();
		return res;
	}
	if(this->type == "ind")
		return "*" + this->args[0]->toString();
	if(this->type == "var")
		return this->args[0]->toString();
	if(this->type == "${}")
		return "${" + this->args[0]->toString() + "}";
	if(this->type == "()")
		return "(" + this->args[0]->toString() + ")";
	if(this->type == "{}")
		return "{" + this->args[0]->toString() + "}";
	if(this->type == ";") {
		string result;
		for(int i = 0; i < this->args.size(); ++i) {
			if(i > 0)
				result += "; ";
			if(this->args[i])
				result += this->args[i]->toString();
		}
		return result;
	}
	return this->args[0]->toString() + " " + this->type + " " + this->args[1]->toString();
}
string Expression::pretty(char ident, int count, int level) const {
	string res(count * level, ident);

	res += type;
	if(!text.empty())
		res += " [" + text + "]";
	for(const unique_ptr<Expression> &i : args)
		if(!i)
			res += "\n" + string(count * (level + 1), ident) + "null";
		else
			res += "\n" + i->pretty(ident, count, level + 1);
	return res;
}

string Expression::prettyOneLine() const {
	string res("(");

	res += type;
	if(!text.empty())
		res += " [" + text + "]";
	for(const unique_ptr<Expression> &i : args)
		if(!i)
			res += " (null)";
		else
			res += " " + i->prettyOneLine();
	return res + ")";
}

string StackTrace::toString() const {
	return error + " [stacktrace: " + join(frames, " -> ") + "]";
}
void StackTrace::except(string err) {
	error = err;
	throw *this;
}

Variable Expression::evaluate(string who) const {
	StackTrace st;
	return evaluate(who, st);
}

// TODO: ability to tag ExpressionTree as various types. string, int,
// TODO: double, variable, function?

// TODO: on throw, rollback changes. This will be a lot of work...

// TODO: better timing control
// TODO: max recursion depth? Just run in thread and abort after x time?
Variable Expression::evaluate(string who, StackTrace &context) const {
	if(this == nullptr) // TODO: something probably went wrong.... empty expression?
		context.except("this == nullptr");
	context.frames.push_back(this->type);

	// might have side effects
	// TODO: permissions on creation with = and => (mostly execute and owner)

	// simple wrappers
	if(this->type == "${}" || this->type == "()" || this->type == "{}")
		return this->args[0]->evaluate(who);

	// multiple expressions
	if(this->type == ";") {
		// first might have side effects
		Variable result;
		for(int i = 0; i < this->args.size(); ++i)
			if(this->args[i])
				result = this->args[i]->evaluate(who);
		return result;
	}

	// ? and ? : operators
	if(this->type == "?") {
		Variable cond = this->args[0]->evaluate(who);

		// if we have a ? : operator
		if(this->args[1]->type == ":") {
			// TODO: only evaluates one side...
			if(cond.isFalse())
				return this->args[1]->args[1]->evaluate(who);
			else
				return this->args[1]->args[0]->evaluate(who);
		}

		// just ?, no false branch
		if(cond.isFalse())
			return Variable("", Permissions());

		return this->args[1]->evaluate(who);
	}

	if(this->type == "=") {
		if(this->args[0]->type != "var")
			context.except("lhs of = is not a variable");

		// get var name and the contents
		// note: we don't actually evaluate the $, but it's argument
		string var = this->args[0]->args[0]->evaluate(who).toString();
		Variable result = this->args[1]->evaluate(who);

		// if assigning to null, don't really assign
		if(var == "null")
			return result;

		// it's a new variable, add it to the map under who
		if(global::vars.find(var) == global::vars.end())
			result.permissions = Permissions(who);
		else // make sure we have permission
			// this can throw? TODO: wrap this throw into StackTrace
			ensurePermission(Permission::Write, who, var);

		return (global::vars[var] = result); // TODO: assignment should merge permissions?
	}

	// see = implementation above for comments
	if(this->type == "=>") {
		if(this->args[0]->type != "var")
			context.except("lhs of => is not a variable");

		string func = this->args[0]->args[0]->evaluate(who).toString(),
			body = this->args[1]->toString(); // TODO: requires Expression::toString

		if(func == "null")
			return Variable(body, Permissions());

		if(global::vars.find(func) == global::vars.end())
			return (global::vars[func] = Variable(body, Permissions(who)));

		ensurePermission(Permission::Write, who, func);
		return (global::vars[func] = Variable(body, Permissions()));
	}

	// function calls, special :D
	if(this->type == "!") {
		if(this->args[0]->type != "var")
			context.except("rhs of ! is not a variable");
		string func = this->args[0]->args[0]->evaluate(who).toString();
		if((global::vars.find(func) == global::vars.end()) &&
				!(contains(modules::hfmap, func)))
			context.except(func + " does not exist as a callable function");

		string body = global::vars[func].toString();
		cerr << "! body: " << body << endl;
		ensurePermission(Permission::Execute, who, func);

		// figure out the result of the arguments
		vector<Variable> argVars;
		for(unsigned i = 1; i < args.size(); ++i)
			argVars.push_back(args[i]->evaluate(who));

		string argsstr;
		for(auto arg : argVars)
			argsstr += " " + arg.toString();

		// clear out argument variables
		for(int i = 0; i < 10; ++i)
			global::vars["$" + asString(i + 1)] = "";

		// set the argument values
		global::vars["args"] = argsstr;
		global::vars["$0"] = Variable(func, Permissions());
		for(unsigned i = 0; i < args.size() - 1; ++i)
			global::vars["$" + asString(i + 1)] = argVars[i];

		// a module function
		if(contains(modules::hfmap, func)) {
			modules::Function mfunc = modules::hfmap[func];
			return mfunc(argVars);
		}

		try {
			unique_ptr<Expression> expr = Parser::parseCanonical(body);
			context.frames.push_back(func);
			return expr->evaluate(who, context);
		} catch(ParseException e) {
			context.except(e.msg + " @" + asString(e.idx));
		}
	}

	// TODO: reimplement ++ and --, needs parser support too

	// TODO: regex is reinterpreting the argument escapes?
	// TODO: $text =~ ':o/|\o:' behaves like ':o/|o:'
	if(this->type == "=~" || this->type == "~") {
		string text = this->args[0]->evaluate(who).toString(), result;
		// TODO: may throw, wrap into StackTrace
		try {
			Regex r(this->args[1]->evaluate(who).toString());

			// if match equals, just return if we match
			if(this->type == "=~")
				return Variable(r.matches(text), Permissions());

			// wanted a replacement, but that's not the type of regex we have
			if(r.type() != RegexType::Replace)
				context.except("cannot attempt replace without result text");

			// TODO: group variables, r0, r1, etc
			bool matches = r.execute(text, result);
			return (global::vars["r_"] =
					Variable(result, Permissions(Permission::Read)));
		} catch(string &e) {
			context.except(e);
		}
	}

	// simple strings an numbers
	if(this->type == "str")
		return Variable(this->text, Permissions());
	if(this->type == "num")
		return Variable::parse(this->text);

	// TODO: descape
	// strcat means to concatenate arguments and return a string
	if(this->type == "'" || this->type == "\"" || this->type == "strcat") {
		string result;
		for(auto &i : this->args)
			result += i->evaluate(who).toString();
		return Variable(result, Permissions());
	}

	// variable access
	if(this->type == "var") {
		string var = this->args[0]->evaluate(who).toString();
		if(var == "true")
			return Variable::parse("true");
		if(var == "false")
			return Variable::parse("false");
		if(global::vars.find(var) == global::vars.end())
			global::vars[var] = Variable(0L, Permissions(who));

		ensurePermission(Permission::Read, who, var);
		return global::vars[var];
	}

	// TODO: de-int this. double? Only when "appropriate"?
	/*
	if(this->fragment.isSpecial("^")) {
		return asString(pow(fromString<long>(this->child->evaluate(who)),
				fromString<long>(this->rchild->evaluate(who))));
	}
	*/

	if(this->type == "+")
		return this->args[0]->evaluate(who) + this->args[1]->evaluate(who);
	if(this->type == "-")
		return this->args[0]->evaluate(who) - this->args[1]->evaluate(who);
	if(this->type == "*")
		return this->args[0]->evaluate(who) * this->args[1]->evaluate(who);
	if(this->type == "/")
		return this->args[0]->evaluate(who) / this->args[1]->evaluate(who);
	if(this->type == "%")
		return this->args[0]->evaluate(who) % this->args[1]->evaluate(who);

	vector<string> comparisons = { "==", "!=", "<=", ">=", "<", ">" };
	for(auto c : comparisons)
		if(this->type == c)
			return this->args[0]->evaluate(who).compare(
					this->args[1]->evaluate(who), c);

	// TODO: un-double this? Also, unstring for == and ~=
	if(this->type == "&&")
		return this->args[0]->evaluate(who) & this->args[1]->evaluate(who);
	if(this->type == "||")
		return this->args[0]->evaluate(who) | this->args[1]->evaluate(who);

	vector<string> compoundOpAssigns = { "+", "-", "*", "/", "%", "^", "~" };
	for(auto op : compoundOpAssigns) {
		if(this->type == (op + "=")) {
			Expression opExpr(op);
			// push back copies of the arguments
			for(int i = 0; i < 2; ++i)
				opExpr.args.push_back(
						unique_ptr<Expression>(new Expression(*this->args[i].get())));

			Variable result = opExpr.evaluate(who);

			// steal args from opExpr to avoid a copy, since we don't need opExpr
			// anymore anyway
			Expression assign("=", opExpr.args);
			assign.args[1] = unique_ptr<Expression>(new Expression("str", result.toString()));
			return assign.evaluate(who);
		}
	}

	context.except("unknown node " + this->type + " -- bug " +
			global::vars["bot.owner"].toString() + " to fix");
}

