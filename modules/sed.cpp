#include "sed.hpp"
using std::string;
using modules::Word;

#include "regex.hpp"
#include "journal.hpp"
#include "expression.hpp"

string s(Bot *bot, string regex) {
	try {
		Regex r{regex};
		auto entries = bot->journal.fetch(AndPredicate{
			[=](Entry &e) {
				// only replace on non-executed things
				if(e.etype == ExecuteType::Hook || e.etype == ExecuteType::Function
						|| e.etype == ExecuteType::Unknown)
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
	try {
		Regex validate{regex};
		auto s = "${ " + name + " => !s '" + reEscape(regex) + "' }";
		auto r = bot->evaluate(s, bot->vars.getString("nick"));
		return r;
	} catch(string &e) {
		return e;
	}
	throw string{"push: ran off edge?"};
}

