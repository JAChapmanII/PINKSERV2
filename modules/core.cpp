#include "core.hpp"
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
using util::contains;
using util::join;
#include "parser.hpp"
#include "expression.hpp"

#include "config.hpp"

Entry regexRandomEntry(Bot &bot, string regex);

string help(Bot *bot, string function) {
	if(!function.empty()) {
		if(!contains(global::moduleFunctionList, function))
			return "error: requested function does not exist";
		return bot->vars.getString(function + ".help");
	}
	return join(global::moduleFunctionList, ", ");
}

void irc(string command) { cout << command << endl; }

string echo(string args) { return args; }

Variable core_or(Bot *bot, vector<Variable> arguments) {
	uniform_int_distribution<> uid(0, arguments.size() - 1);
	unsigned target = uid(bot->rengine);
	return arguments[target];
}

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

string type(vector<Variable> arguments) {
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
	return res;
}

bool defined(Bot *bot, string name) { return bot->vars.defined(name); }
bool undefined(Bot *bot, string name) { return !bot->vars.defined(name); }

// TODO: we need to know the caller for this to work... (perms)
void rm(Bot *bot, string name) { bot->vars.erase(name); }

void sleep(Bot *bot) { bot->done = true; }

long jsize(Bot *bot) { return bot->journal.size(); }

Entry regexRandomEntry(Bot &bot, string regex) {
	auto lines = bot.journal.fetch(RegexPredicate{regex});
	if(lines.size() < 1)
		throw (string)"no matches";

	uniform_int_distribution<> uid(0, lines.size() - 1);
	unsigned target = uid(bot.rengine);
	return lines[target];
}

string rgrep(Bot *bot, string regex) {
	return regexRandomEntry(*bot, regex).arguments;
}

string rline(Bot *bot, string regex) {
	auto line = regexRandomEntry(*bot, regex);
	return "<" + line.nick() + "> " + line.arguments;
}

string debug(string text) {
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
	return "see cerr";
}

string todo(string text) {
	// TODO: db
	ofstream out(config::todoFileName, std::ios::app);
	if(!out.good())
		return "error: couldn't save to TODO file";
	out << text << endl;
	return "saved!";
}

Variable toint(Variable var) { return var.asInteger(); }

// TODO: vector of long
string bmess(vector<Variable> arguments) {
	if(arguments.size() < 1)
		return "error: bmess takes a list of bytes";
	string res = "bmess: ";
	for(auto &v : arguments)
		res += (char)(v.asInteger().value.l & 0xFF);
	return res;
}

string pol(string body) {
	auto e = Parser::parse("${ " + body + " }");
	return e->prettyOneLine();
}

