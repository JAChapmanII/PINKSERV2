#include "journal.hpp"
using std::string;
using std::vector;

using journal::Entry;
using journal::EntryType;

#include <fstream>
using std::ofstream;
using std::ifstream;
using std::endl;
#include <iostream>
using std::cerr;

#include <boost/regex.hpp>
#include "util.hpp"
using util::fromString;
using util::asString;
#include "config.hpp"
#include "global.hpp"

vector<Entry> journal_entries;
ofstream journal_file;
bool journal_init = false;

bool journal::init() {
	if(journal_init)
		return true;
	
	ifstream jin(config::journalFileName);
	if(!jin.good())
		return false;
	while(!jin.eof() && jin.good()) {
		string line;
		getline(jin, line);
		if(line.empty())
			continue;
		journal_entries.push_back(Entry::parse(line));
	}
	
	journal_file.open(config::journalFileName, std::ios_base::app);
	cerr << "opened journal file: " << config::journalFileName << endl;

	return true;
}

bool journal::deinit() {
	return false;
}

void journal::push(Entry nentry) {
	journal_entries.push_back(nentry);
	journal_file << nentry.format() << endl;
}
std::vector<Entry> journal::search(string regex) {
	auto start = journal_entries.begin();
	if(journal_entries.size() > 10000)
		start = journal_entries.end() - 10000;

	vector<Entry> matches;
	boost::regex r(regex, boost::regex::perl);

	for(auto i = start; i != journal_entries.end(); ++i) {
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

Entry::Entry(std::string icontents) : timestamp(global::now()), contents(icontents) {}
Entry Entry::parse(string line) {
	Entry result;
	result.timestamp = fromString<unsigned long>(line.substr(0, line.find("|")));
	result.contents = line.substr(line.find("|"));
	return result;
}
string Entry::format() const {
	return asString(this->timestamp) + "|" + this->contents;
}
