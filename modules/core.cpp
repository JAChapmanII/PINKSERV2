#include "core.hpp"
using std::ostream;
using std::istream;
using std::vector;
using std::string;

#include <random>
using std::uniform_int_distribution;
using std::uniform_real_distribution;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
using std::ofstream;

#include "global.hpp"
#include "util.hpp"
using util::fromString;
using util::asString;
using util::contains;
using util::join;
#include "journal.hpp"
#include "expression.hpp"
#include "parser.hpp"

#include "config.hpp"

Entry regexRandomEntry(string regex);

string help(Bot *bot, string function) {
	if(!function.empty()) {
		if(!contains(global::moduleFunctionList, function))
			return "error: requested function does not exist";
		return bot->get(function + ".help");
	}
	return join(global::moduleFunctionList, ", ");
}

void irc(string command) { cout << command << endl; }

string echo(string args) { return args; }

/*
Variable core_or(vector<Variable> arguments) {
	uniform_int_distribution<> uid(0, arguments.size() - 1);
	unsigned target = uid(global::rengine);
	return arguments[target];
}
*/

long rand(Bot *bot, long low, long high) {
	if(low > high)
		throw (string)"rand's second parameter must be larger";
	if(low == high)
		return low;
	uniform_int_distribution<long> lrng(low, high);
	return lrng(bot->rengine);
}

double drand(Bot *bot, double low, double high) {
	if(low > high)
		throw (string)"drand's second parameter must be larger";
	uniform_real_distribution<double> lrng(low, high);
	return lrng(bot->rengine);
}

/*
Variable type(vector<Variable> arguments) {
	string res;
	for(auto arg : arguments) {
		switch(arg.type) {
			case Type::Integer: res += arg.toString() + ":Integer"; break;
			case Type::Double: res += arg.toString() + ":Double"; break;
			case Type::Boolean: res += arg.toString() + ":Boolean"; break;
			case Type::String: res += arg.toString() + ":String"; break;
			default: res += "(" + arg.toString() + ":error)"; break;
		}
		res += " ";
	}
	res.pop_back();
	return Variable(res, Permissions());
}

Variable undefined(std::vector<Variable> arguments) {
	if(arguments.size() != 1)
		throw (string)"error: undefined takes one argument";
	if(global::vars.defined(arguments.front().toString()))
		return Variable(false, Permissions());
	return Variable(true, Permissions());
}

// TODO: how to do this partly? Gah... :(
Variable rm(vector<Variable> arguments) {
	// TODO: we need to know the caller for this to work... (perms)
	for(auto var : arguments) {
		global::vars.erase(var.toString());
	}
	return Variable("erased", Permissions());
	throw (string)"(not-implemented, bug " + global::vars.getString("bot.owner") + ")";
}

Variable sleep(vector<Variable>) {
	// TODO: can we just exit(non-0)?
	throw (string)"(uh-oh, bug " + global::vars.getString("bot.owner") + ")";
}

Variable jsize(vector<Variable>) {
	return Variable((long)global::journal.size(), Permissions());
}

Entry regexRandomEntry(string regex) {
	vector<Entry> lines = global::journal.fetch(RegexPredicate{regex});
	if(lines.size() < 1)
		throw (string)"no matches";

	uniform_int_distribution<> uid(0, lines.size() - 1);
	unsigned target = uid(global::rengine);

	return lines[target];
}

Variable rgrep(vector<Variable> arguments) {
	string regex = arguments.front().toString();
	auto line = regexRandomEntry(regex);
	return Variable(line.arguments, Permissions());
}

Variable rline(vector<Variable> arguments) {
	string regex = arguments.front().toString();
	auto line = regexRandomEntry(regex);
	return Variable("<" + line.nick() + "> " + line.arguments, Permissions());
}

Variable debug(vector<Variable> arguments) {
	string text = join(arguments, " ");
	cerr << "debug: \"" << text << "\"" << endl;
	try {
		auto expr = Parser::parse(text);
		if(!expr)
			cerr << "expr is null" << endl;
		else
			cerr << "expr: " << endl << expr->pretty() << endl;
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
	} catch(StackTrace e) {
		cerr << e.toString() << endl;
	} catch(string &s) {
		cerr << "string type error: " << s << endl;
	}
	return Variable(true, Permissions());
}

Variable todo(std::vector<Variable> arguments) {
	string text = join(arguments, " ");
	ofstream out(config::todoFileName, std::ios::app);
	if(!out.good())
		return Variable("error: couldn't save to TODO file", Permissions());
	out << text << endl;
	return Variable("saved!", Permissions());
}

Variable toint(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: toint takes one argument", Permissions());
	return arguments.front().asInteger();
}

Variable bmess(vector<Variable> arguments) {
	if(arguments.size() < 1)
		return Variable("error: bmess takes a list of bytes", Permissions());
	string res = "bmess: ";
	for(auto &v : arguments)
		res += (char)(v.asInteger().value.l & 0xFF);
	return Variable(res, Permissions());
}

Variable pol(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: pol takes one argument", Permissions());
	auto e = Parser::parse("${ " + arguments.front().toString() + " }");
	return Variable(e->prettyOneLine(), Permissions());
}
*/

