#include "journal.hpp"
using std::string;
using std::vector;

using journal::Entry;
using journal::EntryType;

#include <boost/regex.hpp>

vector<Entry> journal::entries;

bool journal::init() {
	return false;
}

bool journal::deinit() {
	return false;
}

std::vector<Entry> journal::search(string regex) {
	auto start = entries.begin();
	if(entries.size() > 10000)
		start = entries.end() - 10000;

	vector<Entry> matches;
	boost::regex r(regex, boost::regex::perl);

	for(auto i = start; i != entries.end(); ++i) {
		if(i->type() != EntryType::Text)
			continue;
		if(boost::regex_search(i->message(), r, boost::match_default))
			matches.push_back(*i);
	}
	return matches;
}

EntryType Entry::type() const {
	return EntryType::Invalid;
}
string Entry::message() const {
	return this->contents;
}
