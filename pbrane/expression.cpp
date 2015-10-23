#include "expression.hpp"
using std::vector;
using std::string;
using std::unique_ptr;
using std::to_string;

#include <map>
using std::map;

#include <cmath>

#include "permission.hpp"

#include "util.hpp"
using util::asString;
using util::join;
using util::contains;

#include "regex.hpp"

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

string reEscape(string str) {
	string result;
	for(char c : str)
		switch(c) {
			case '\\': result += '\\';
			default: result += c;
		}
	return result;
}

string Expression::toString() const {
	if(this == nullptr)
		return "[null]";
	if(this->type == "str" || this->type == "num")
		return reEscape(this->text);
	if(this->type == "\"" || this->type == "'") {
		string res = this->type;
		for(auto &arg : this->args)
			res += arg->toString();
		return res + this->type;
	}
	if(this->type == "!") {
		string res = this->type + this->args[0]->toString();
		for(int i = 1; i < (int)this->args.size(); ++i)
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
		for(int i = 0; i < (int)this->args.size(); ++i) {
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
	return error + " [stacktrace: " + join(frames, " ") + "]";
}
void StackTrace::except(ExceptionType t, std::string err) {
	error = err;
	type = t;
	throw *this;
}
void StackTrace::except(string err) { this->except(ExceptionType::Other, err); }

StackFrameLifetime StackTrace::push(StackFrame frame) {
	frames.push_back(frame);
	return StackFrameLifetime{*this};
}

StackFrameLifetime::StackFrameLifetime(StackTrace &trace, int which)
		: _trace{trace}, _which{which} {
	if(_which < 0)
		_which = _trace.frames.size() - 1;
}
StackFrameLifetime::~StackFrameLifetime() {
	if(_trace.hasError || _which < 0)
		return;
	if(_trace.frames.size() != _which + 1)
		throw make_except("TODO");
	_trace.frames.pop_back();
}

StackFrameLifetime::StackFrameLifetime(StackFrameLifetime &&rhs)
		: _trace{rhs._trace}, _which{rhs._which} {
	rhs._which = -1;
}

void ExpressionContext::except(string err) {
	trace.except(err);
}
void ExpressionContext::except(ExceptionType type, string err) {
	trace.except(type, err);
}


Variable Expression::evaluate(Pvm &vm, string who) const {
	StackTrace st;
	st.owner = who;

	TransactionalVarStore tvs{vm.vars};
	Pvm nvm{tvs, vm.debugFunctionBodies};
	nvm.functions = vm.functions; // TODO: messy

	// TODO: bleh, vm has to get in somehow?
	st.push("#");

	ExpressionContext context{st, nvm};

	try {
		return evaluate(context);
	} catch(StackTrace &trace) {
		cerr << "abort" << endl;
		tvs.abort();
		throw;
	}
}


// TODO: ability to tag ExpressionTree as various types. string, int,
// TODO: double, variable, function?

// TODO: on throw, rollback changes. Have object that stores locally in map,
// TODO: then on destruction commits changes all at once to underlying VarStore
// TODO: very easy local variables, just check name and don't save? But
// TODO: sub-context...

// TODO: better timing control
// TODO: abort after Xms?
Variable Expression::evaluate(ExpressionContext &context) const {
	if(this == nullptr) // TODO: something probably went wrong.... empty expression?
		context.except("this == nullptr");

	// might have side effects
	// TODO: permissions on creation with = and => (mostly execute and owner)

	// simple wrappers
	if(this->type == "${}" || this->type == "()" || this->type == "{}")
		return this->args[0]->evaluate(context);

	// multiple expressions
	if(this->type == ";") {
		// first might have side effects
		Variable result;
		for(int i = 0; i < (int)this->args.size(); ++i)
			if(this->args[i])
				result = this->args[i]->evaluate(context);
		return result;
	}

	// monitor stacktrace, skipping simple wrapper contexts
	auto lifetime = context.trace.push(this->type);
	auto &vm = context.vm;
	auto &trace = context.trace;
	auto &frame = trace.frames.back();
	auto &vars = context.vm.vars;

	// TODO: configurable max recursion depth?
	if(trace.frames.size() > 32)
		context.except("exceeded max recursion depth");


	// ? and ? : operators
	if(this->type == "?") {
		Variable cond = this->args[0]->evaluate(context);

		// if we have a ? : operator
		if(this->args[1]->type == ":") {
			if(cond.isFalse())
				return this->args[1]->args[1]->evaluate(context);
			else
				return this->args[1]->args[0]->evaluate(context);
		}

		// just ?, no false branch
		if(cond.isFalse())
			return Variable();

		return this->args[1]->evaluate(context);
	}

	if(this->type == "=") {
		if(this->args[0]->type != "var")
			context.except("lhs of = is not a variable");

		// get var name and the contents
		// note: we don't actually evaluate the $, but it's argument
		string var = this->args[0]->args[0]->evaluate(context).toString();
		Variable result = this->args[1]->evaluate(context);

		// if assigning to null, don't really assign
		if(var == "null")
			return result;

		return vars.set(var, result); // TODO: perms?
	}

	// see = implementation above for comments
	if(this->type == "=>") {
		if(this->args[0]->type != "var")
			context.except("lhs of => is not a variable");

		string func = this->args[0]->args[0]->evaluate(context).toString(),
			body = this->args[1]->toString(); // TODO: requires Expression::toString

		if(func == "null")
			return Variable(body);

		auto res = vars.set(func, body);
		auto fVar = vars.get(func);
		fVar.type = Type::Function;
		vars.set(func, fVar);
		return res;
	}

	// function calls, special :D
	if(this->type == "!") {
		// store $ variables incase they're clobbered
		Variable oargs = vars.get("args");
		map<string, Variable> ovars;
		for(int i = 0; i < 10; ++i) {
			auto v = "$" + to_string(i);
			if(vars.defined(v))
				ovars[v] = vars.get(v);
		}

		if(this->args[0]->type != "var")
			context.except("rhs of ! is not a variable");
		string func = this->args[0]->args[0]->evaluate(context).toString();
		if(!vars.defined(func) && !contains(modules::hfmap, func)) {
			trace.arg = func;
			trace.except(ExceptionType::FunctionDoesNotExist,
					func + " does not exist as a callable function");
		}

		if(!contains(modules::hfmap, func)) {
			auto fVar = vars.get(func);
			fVar.type = Type::Function;
			vars.set(func, fVar);
		}

		auto body = vars.get(func).toString();

		if(vm.debugFunctionBodies)
			cerr << "! body: " << body << endl;
		ensurePermission(Permission::Execute, trace.owner, func);

		// figure out the result of the arguments
		vector<Variable> argVars;
		for(unsigned i = 1; i < args.size(); ++i)
			argVars.push_back(args[i]->evaluate(context));

		auto argsstr = util::join(argVars, " ");

		// clear out argument variables
		for(int i = 0; i < 10; ++i)
			vars.erase("$" + asString(i + 1));

		// set the argument values
		vars.set("args", argsstr);
		vars.set("$0", func);
		for(unsigned i = 0; i < args.size() - 1; ++i)
			vars.set("$" + asString(i + 1), argVars[i]);

		Variable result;

		// a module function
		if(contains(modules::hfmap, func)) {
			result = modules::hfmap[func](argVars);
		} else {
			try {
				auto expr = Parser::parseCanonical(body);

				// rewrite frame name to be correct
				trace.frames.back() = "!" + func;

				result = expr->evaluate(context);
			} catch(ParseException e) {
				context.except(e.msg + " @" + asString(e.idx));
			}
		}

		// restort $ variables
		vars.set("args", oargs);
		for(auto &e : ovars)
			vars.set(e.first, e.second);

		return result;
	}

	// TODO: reimplement ++ and --, needs parser support too

	// TODO: regex is reinterpreting the argument escapes?
	// TODO: $text =~ ':o/|\o:' behaves like ':o/|o:'
	if(this->type == "=~" || this->type == "~") {
		string result, rtext = this->args[0]->evaluate(context).toString();
		// TODO: may throw, wrap into StackTrace
		try {
			Regex r(this->args[1]->evaluate(context).toString());

			// if match equals, just return if we match
			if(this->type == "=~")
				return Variable(r.matches(rtext));

			// wanted a replacement, but that's not the type of regex we have
			if(r.type() != RegexType::Replace)
				context.except("cannot attempt replace without result text");

			// TODO: group variables, r0, r1, etc
			bool matches = r.execute(rtext, result);
			return vars.set("r_", result); // TODO: read-only
		} catch(string &e) {
			context.except(e);
		}
	}

	// simple strings an numbers
	if(this->type == "str")
		return Variable(this->text);
	if(this->type == "num")
		return Variable::parse(this->text);

	// TODO: descape
	// strcat means to concatenate arguments and return a string
	if(this->type == "'" || this->type == "\"" || this->type == "strcat") {
		string result;
		for(auto &i : this->args)
			result += i->evaluate(context).toString();
		return Variable(result);
	}

	// variable access
	if(this->type == "var") {
		string var = this->args[0]->evaluate(context).toString();
		if(var == "true")
			return Variable::parse("true");
		if(var == "false")
			return Variable::parse("false");
		if(!vars.defined(var))
			return vars.set(var, "0");
		return vars.get(var);
	}

	// TODO: de-int this. double? Only when "appropriate"?
	/*
	if(this->fragment.isSpecial("^")) {
		return asString(pow(fromString<long>(this->child->evaluate(context)),
				fromString<long>(this->rchild->evaluate(context))));
	}
	*/

	try {
		if(this->type == "+")
			return this->args[0]->evaluate(context) + this->args[1]->evaluate(context);
		if(this->type == "-")
			return this->args[0]->evaluate(context) - this->args[1]->evaluate(context);
		if(this->type == "*")
			return this->args[0]->evaluate(context) * this->args[1]->evaluate(context);
		if(this->type == "/")
			return this->args[0]->evaluate(context) / this->args[1]->evaluate(context);
		if(this->type == "%")
			return this->args[0]->evaluate(context) % this->args[1]->evaluate(context);

		vector<string> comparisons = { "==", "!=", "<=", ">=", "<", ">" };
		for(auto c : comparisons)
			if(this->type == c)
				return this->args[0]->evaluate(context).compare(
						this->args[1]->evaluate(context), c);

		// TODO: un-double this? Also, unstring for == and ~=
		if(this->type == "&&")
			return this->args[0]->evaluate(context) & this->args[1]->evaluate(context);
		if(this->type == "||")
			return this->args[0]->evaluate(context) | this->args[1]->evaluate(context);

		vector<string> compoundOpAssigns = { "+", "-", "*", "/", "%", "^", "~" };
		for(auto op : compoundOpAssigns) {
			if(this->type == (op + "=")) {
				Expression opExpr(op);
				// push back copies of the arguments
				for(int i = 0; i < 2; ++i)
					opExpr.args.push_back(
							unique_ptr<Expression>(new Expression(*this->args[i].get())));

				Variable result = opExpr.evaluate(context);

				// steal args from opExpr to avoid a copy, since we don't need opExpr
				// anymore anyway
				Expression assign("=", opExpr.args);
				assign.args[1] = unique_ptr<Expression>(new Expression("str", result.toString()));
				return assign.evaluate(context);
			}
		}
	} catch(string &err) {
		trace.except(err);
	}

	context.except("unknown node " + this->type + " -- bug " +
			vars.get("bot.owner").toString() + " to fix");
}

