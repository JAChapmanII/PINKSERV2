#include "sed.hpp"
using std::string;
using modules::Word;

#include "regex.hpp"
#include "journal.hpp"
#include "expression.hpp"
#include "util.hpp"

string s(Bot *bot, string regex) {
	try {
		Regex r{regex};
		auto here = bot->vars.get("where").toString();
		auto entries = bot->journal.fetch(AndPredicate{
			[=](Entry &e) {
				// only replace on non-executed things
				if(e.etype == ExecuteType::Hook || e.etype == ExecuteType::Function
						|| e.etype == ExecuteType::Unknown)
					return false;
				// only find things in the same channel
				// TODO: same channel but different server needs handled
				// TODO: optionally specify different channel/server?
				if(e.where != here)
					return false;
				// if a nick was specified as a flag and it's not who said it, skip
				if(!r.flags().empty() && r.flags() != e.nick())
					return false;
				return true;
			}, RegexPredicate{r.match()}}, 1);

		if(entries.empty())
			return "";

		auto e = entries.front();

		string result;
		r.execute(e.arguments, result);

		if(e.etype == ExecuteType::None)
			result = "<" + e.nick() + "> " + result;

		return result;
	} catch(string &e) {
		return e;
	}
	throw string{"s: ran off edge?"};
}

string push(Bot *bot, Word name, string regex) {
	// remove name from plist, will be readded if needed
	auto ps = util::split(bot->vars.get("bot.plist").toString());
	ps.erase(std::remove(ps.begin(), ps.end(), name), ps.end());
	bot->vars.set("bot.plist", util::join(ps, " "));

	if(regex.empty()) {
		bot->vars.erase(name);
		return "removed " + name;
	}

	try {
		Regex validate{regex};
		auto s = "${ " + name + " => !s '" + reEscape(regex) + "' }";
		auto r = bot->evaluate(s, bot->vars.get("nick").toString());

		ps.push_back(name);
		bot->vars.set("bot.plist", util::join(ps, " "));

		return r;
	} catch(string &e) {
		return e;
	}
	throw string{"push: ran off edge?"};
}

string rlist(Bot *bot) { return bot->vars.get("bot.plist").toString(); }

