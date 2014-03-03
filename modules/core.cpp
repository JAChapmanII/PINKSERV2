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
#include "brain.hpp"
#include "util.hpp"
using util::fromString;
using util::asString;
using util::contains;
using util::join;
#include "journal.hpp"
#include "expression.hpp"
#include "parser.hpp"

#include "config.hpp"

void coreLoad(std::istream &in) {
	brain::read(in, global::vars);
}
void coreSave(std::ostream &out) {
	brain::write(out, global::vars);
}

Variable help(vector<Variable> arguments) {
	if(arguments.size() > 1)
		return Variable("error: help can only take a max of one function name",
				Permissions());
	if(arguments.size() == 1) {
		string function = arguments.front().toString();
		if(!contains(global::moduleFunctionList, function))
			return Variable("error: requested function does not exist", Permissions());
		return global::vars[function + ".help"];
	}
	return Variable(join(global::moduleFunctionList, ", "), Permissions());
}

Variable irc(vector<Variable> arguments) {
	string out;
	for(auto arg : arguments)
		out += arg.toString() + " ";
	out.pop_back();
	cout << out << endl;
	return Variable("", Permissions());
}

Variable echo(vector<Variable> arguments) {
	string res;
	for(auto a : arguments) {
		string arg = a.toString();
		// TODO: old semantics of echo?
		if(!res.empty() && (!isspace(res.back()) && !isspace(arg[0])))
			res += " ";
		res += arg;
	}
	// TODO: this should really just append to some other real return
	// TODO: this is broken now >_>
	//if(this->next && all)
		//return argsstr + this->next->evaluate(nick);
	return Variable(res, Permissions());
}

Variable core_or(vector<Variable> arguments) {
	uniform_int_distribution<> uid(0, arguments.size() - 1);
	unsigned target = uid(global::rengine);
	return arguments[target];
}

Variable rand(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"rand takes two parameters; the bounds";
	long low = fromString<long>(arguments[0].toString()),
		high = fromString<long>(arguments[1].toString());
	if(low > high)
		throw (string)"rand's second parameter must be larger";
	if(low == high)
		return Variable(low, Permissions());
	uniform_int_distribution<long> lrng(low, high);
	return Variable(lrng(global::rengine), Permissions());
}

Variable drand(vector<Variable> arguments) {
	if(arguments.size() != 2)
		throw (string)"drand takes two parameters; the bounds";
	double low = fromString<double>(arguments[0].toString()),
			high = fromString<double>(arguments[1].toString());
	if(low > high)
		throw (string)"drand's second parameter must be larger";
	uniform_real_distribution<double> lrng(low, high);
	return Variable(lrng(global::rengine), Permissions());
}

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
	if(global::vars.find(arguments.front().toString()) == global::vars.end())
		return Variable(true, Permissions());
	return Variable(false, Permissions());
}

// TODO: how to do this partly? Gah... :(
Variable rm(vector<Variable>) {
	// TODO: we need to know the caller for this to work...
	//for(auto var : arguments) {
	//}
	throw (string)"(not-implemented, bug " + global::vars["bot.owner"].toString() + ")";
}

Variable die(vector<Variable>) {
	return Variable("what the hell! jac! help! somebody is abusing me!", Permissions());
	//exit(0);
	//throw (string)"uh-oh, we were supposed to die; " + 
		//"bug " + global::vars["bot.owner"].toString();
}

Variable sleep(vector<Variable>) {
	// TODO: can we just exit(non-0)?
	throw (string)"(uh-oh, bug " + global::vars["bot.owner"].toString() + ")";
}

Variable jsize(vector<Variable>) {
	return Variable((long)journal::size(), Permissions());
}

Variable rgrep(vector<Variable> arguments) {
	string regex = arguments.front().toString();
	vector<journal::Entry> lines = journal::search(regex);
	if(lines.size() < 1)
		throw (string)"no matches";

	uniform_int_distribution<> uid(0, lines.size() - 1);
	unsigned target = uid(global::rengine);

	return Variable(lines[target].arguments, Permissions());
}

Variable rline(vector<Variable> arguments) {
	string regex = arguments.front().toString();
	vector<journal::Entry> lines = journal::search(regex);
	if(lines.size() < 1)
		throw (string)"no matches";

	uniform_int_distribution<> uid(0, lines.size() - 1);
	unsigned target = uid(global::rengine);

	return Variable("<" + lines[target].nick() + "> " + lines[target].arguments, Permissions());
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

Variable eventCount(vector<Variable> arguments) {
	if(arguments.size() != 0)
		return Variable("error: eventCount takes no arguments", Permissions());
	return Variable((long)global::eventSystem.eventsSize(EventType::Text), Permissions());
}

Variable getEvent(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: getEvent takes one argument", Permissions());
	return Variable(global::eventSystem.getEvent(EventType::Text,
				arguments.front().asInteger().value.l).body, Permissions());
}

Variable eraseEvent(vector<Variable> arguments) {
	if(arguments.size() != 1)
		return Variable("error: eraseEvent takes one argument", Permissions());
	global::eventSystem.deleteEvent(EventType::Text, arguments.front().asInteger().value.l);
	return Variable((long)global::eventSystem.eventsSize(EventType::Text), Permissions());
}


