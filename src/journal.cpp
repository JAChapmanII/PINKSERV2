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
#include <exception>
using std::exception;

#include <boost/regex.hpp>
#include "util.hpp"
using util::fromString;
using util::asString;
using util::split;
using util::join;
#include "config.hpp"
#include "global.hpp"

vector<Entry> journal_entries;
ofstream journal_file;
bool journal_init = false;

bool journal_tryCreate();
bool journal_tryCreate() {
	journal_file.open(config::journalFileName, std::ios_base::app);
	return journal_file.good();
}

// TODO: if journal doesn't exist, doesn't open for append
bool journal::init() {
	if(journal_init)
		return true;
	
	ifstream jin(config::journalFileName);
	if(!jin.good()) {
		return journal_tryCreate();
	}

	string line;
	while(!jin.eof() && jin.good()) {
		getline(jin, line);
		if(line.empty())
			continue;
		journal_entries.push_back(Entry::fromLog(line));
	}
	
	journal_file.open(config::journalFileName, std::ios_base::app);
	cerr << "opened journal file: " << config::journalFileName << endl;

	return true;
}

bool journal::deinit() {
	journal_file.close();
	return false;
}

void journal::push(Entry nentry) {
	journal_entries.push_back(nentry);
	journal_file << nentry.format() << endl;
}

std::vector<Entry> journal::search(string regex) {
	vector<Entry> matches;
	boost::regex r(regex, boost::regex::perl);

	for(auto i : journal_entries) {
		if(i.type != EntryType::Text)
			continue;
		try {
			if(boost::regex_search(i.arguments, r, boost::match_default))
				matches.push_back(i);
		} catch(exception &e) {
			cerr << "journal::search: regex exception: " << e.what() << endl;
		}
	}
	return matches;
}

unsigned journal::size() {
	return journal_entries.size();
}


Entry::Entry(string icontents) :
		contents(icontents) { this->parse(); }
Entry::Entry(long long itimestamp, string icontents) :
		timestamp(itimestamp), contents(icontents) { this->parse(); }

Entry Entry::fromLog(string line) {
	size_t firstPipe = line.find("|"),
			secondPipe = line.find("|", firstPipe + 1);
	auto timestamp = fromString<long long>(line.substr(0, firstPipe));
	auto contents = line.substr(secondPipe + 1);
	Entry e{timestamp, contents};
	e.etype = (ExecuteType)fromString<int>(
			line.substr(firstPipe + 1, secondPipe));
	return e;
}

void Entry::parse() {
	auto parts = split(this->contents, " ");
	if(parts.size() < 3) {
		cerr << "Entry::parse: strange parts in \"" << this->contents << "\"" << endl;
		return;
	}
	this->who = parts[0];
	this->where = parts[2];

	this->command = parts[1];
	if(this->command == "PRIVMSG")
		this->type = EntryType::Text;

	if(parts.size() > 3) {
		this->arguments = join(parts.begin() + 3, parts.end(), " ");
		if(this->arguments[0] == ':')
			this->arguments = this->arguments.substr(1);
	}
}

string Entry::format() const {
	return asString(this->timestamp) + "|" + asString((int)this->etype) + "|" +
		this->contents;
}

string Entry::nick() const {
	return this->who.substr(1, this->who.find("!") - 1);
}

