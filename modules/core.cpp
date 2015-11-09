#include "core.hpp"
using std::vector;
using std::string;
using std::to_string;

#include <random>
using std::uniform_int_distribution;
using std::uniform_real_distribution;

#include <cstdlib>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include "pbrane/modules.hpp"
#include "sekisa/util.hpp"
using util::contains;
using util::join;
#include "sekisa/web.hpp"
#include "pbrane/parser.hpp"
#include "pbrane/expression.hpp"

Entry regexRandomEntry(Bot &bot, string regex);

string help(Bot *bot, string function) {
	if(!function.empty()) {
		if(bot->vars.defined(function + ".help"))
			return bot->vars.get(function + ".help").toString();

		return "error: requested function help does not exist";
	}

	if(modules::hfmap.empty())
		return "error: no module functions exist";

	// TODO: join with lambda for toString?
	string res{""};
	for(auto &func : modules::hfmap)
		res += func.first + ", ";
	return res.substr(res.length() - 2);
}

string list(Bot *bot) {
	auto ps = util::split(bot->vars.get("bot.plist").toString());
	auto fs = bot->vars.getVariablesOfType(Type::Function);
	for(auto &n : ps)
		fs.erase(n);
	return util::join(fs);
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
			case Type::Number: res += arg.value + ":Number"; break;
			case Type::Boolean: res += arg.value + ":Boolean"; break;
			case Type::String: res += arg.value + ":String"; break;
			case Type::Function: res += arg.value + ":Function"; break;
			default: res += "(" + arg.value + ":error)"; break;
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

string fsearch(Bot *bot, string regex) {
	auto lines = bot->journal.ffetch(RegexPredicate{regex}, 1);
	if(lines.empty()) return "no results";
	auto line = lines.front();
	return "<" + line.nick() + "> " + line.arguments;
}

sqlite_int64 fcount(Bot *bot, string regex) {
	auto lines = bot->journal.ffetch(RegexPredicate{regex});
	return (sqlite_int64)lines.size();
}

string fret(Bot *bot, long which, string regex) {
	auto lines = bot->journal.ffetch(RegexPredicate{regex}, which + 1);
	if(which >= (sqlite_int64)lines.size())
		throw ("error: out of range " + to_string(which)
			+ " >= " + to_string(lines.size()));
	auto line = lines[which];
	return to_string(line.id) + " <" + line.nick() + "> " + line.arguments;
}

string sline(Bot *bot, long which) {
	// TODO: line out of range, not PRIVMSG?
	auto line = bot->journal.fetch(which);
	switch(line.type) {
		case EntryType::Text:
			return to_string(line.id) + " <" + line.nick() + "> " + line.arguments;
			break;
		case EntryType::Join:
			return to_string(line.id) + " --> " + line.nick() + " joins " + line.where;
			break;
		case EntryType::Quit:
		case EntryType::Part:
			return to_string(line.id) + " <-- " + line.nick() + " leaves " + line.where;
			break;
	}
	return "error: unknown entry type: " + to_string((int)line.type);
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
	} catch(StackTrace &e) {
		cerr << e.toString() << endl;
	} catch(string &s) {
		cerr << "string type error: " << s << endl;
	}
	return "see cerr";
}

string todo(Bot *bot, string text) {
	if(bot->todos.push(text))
		return "saved!";
	return "error: couldn't save";
}

Variable toint(Variable var) { return var.asNumber(); }

// TODO: vector of long
string bmess(vector<Variable> arguments) {
	if(arguments.size() < 1)
		return "error: bmess takes a list of bytes";
	string res = "bmess: ";
	for(auto &v : arguments)
		res += (char)((long)v.toNumber() & 0xFF);
	return res;
}

string pol(string body) {
	auto e = Parser::parse("${ " + body + " }");
	return e->prettyOneLine();
}

void restart(Bot *bot) {
	// TODO: perms?
	bot->done = true;
}

string umiki(Bot *bot, string content) {
	if(!bot->vars.defined("bot.umiki.key"))
		throw string{"bug jac, api key does not exist"};

	// TODO: handle grabbing the api_key better...
	auto apiKey = bot->vars.get("bot.umiki.key").toString();

	auto response = web::post("https://serv2.pink/api/umiki/v1", {
			{ "api_key", apiKey }, { "content", content }
		});

	// TODO: strip out just the url part...
	return to_string(response.code) + " " + response.body;
}

string lastlog(Bot *bot) {
	auto here = bot->vars.get("where").toString();
	auto lines = bot->journal.fetch([=](Entry &e) {
			return e.type == EntryType::Text && e.where == here;
		}, 100);

	// TODO: format this data better
	string log{""};
	for(int i = lines.size() - 1; i >= 0; --i) {
		auto &line = lines[i];
		log += to_string(line.id) + " <" + line.nick() + "> " + line.arguments + "\n";
	}

	return umiki(bot, log);
}

string context(Bot *bot, long which) {
	auto here = bot->vars.get("where").toString();
	bool foundWhich{false};
	long countSoFar{0};
	// TODO: this isn't garaunteed to give us 20 on either side
	// TODO: store if we've seen id yet, and how many lines we have saved?
	auto lines = bot->journal.fetch([&](Entry &e) {
			if(e.type != EntryType::Text || e.where != here)
				return false;

			if(!foundWhich && e.id == which) {
				foundWhich = true;
				countSoFar = 0;
			}

			// can return exact number after finding which
			countSoFar++;
			if(foundWhich && countSoFar > 20)
				return false;

			// before finding which, it's all guess work...
			return (abs(e.id - which) <= 2000);
			// TODO: we could run fetch and ffetch, one for each 'side', but
			// TODO: ineffecient. If we parallize this, we sort of break
			// TODO: capturing and the inherent ordering of processing.
			// TODO: may be better handled straight in journal
		}, 100);

	long occursAt = find_if(lines.begin(), lines.end(), [=](Entry &e) {
			return e.id == which;
		}) - lines.begin();

	vector<Entry> filteredLines{};
	for(long i = 0; i < (long)lines.size(); ++i)
		if(abs(occursAt - i) < 20)
			filteredLines.push_back(lines[i]);
	lines = filteredLines;

	// TODO: format this data better, deduplicate logic
	string log{""};
	for(int i = lines.size() - 1; i >= 0; --i) {
		auto &line = lines[i];
		log += to_string(line.id) + " <" + line.nick() + "> " + line.arguments + "\n";
	}

	return umiki(bot, log);
}

