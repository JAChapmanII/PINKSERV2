#include "journal.hpp"
using std::string;
using std::to_string;
using std::vector;
using zidcu::Database;

#include <exception>
using std::exception;

#include <boost/regex.hpp>

#include "sekisa/util.hpp"
using util::split;
using util::join;
#include "sekisa/err.hpp"

#include <iostream>
using std::cerr;
using std::endl;

Entry::Entry(sqlite_int64 itimestamp, string inetwork, string icontents)
	: timestamp{itimestamp}, network{inetwork}, contents{icontents} { this->parse(); }
Entry::Entry(sqlite_int64 iid, sqlite_int64 itimestamp, SentType isent,
		ExecuteType ietype, string inetwork, string icontents)
	: id{iid}, timestamp{itimestamp}, sent{isent}, etype{ietype},
		network{inetwork}, contents{icontents} { this->parse(); }

void Entry::parse() {
	if(this->sent == SentType::Log)
		return;
	auto parts = split(this->contents, " ");
	if(parts.size() < 3) {
		cerr << "Entry::parse: strange parts in \"" << this->contents << "\"" << endl;
		return;
	}
	this->who = parts[0];

	if(this->command == "NICK") {
		this->type = EntryType::Nick;
		this->arguments = parts[2];
		if(this->arguments[0] == ':')
			this->arguments = this->arguments.substr(1);
		return;
	}

	if(this->command == "QUIT") {
		this->type = EntryType::Quit;
		this->arguments = join(parts.begin() + 2, parts.end(), " ");
		if(this->arguments[0] == ':')
			this->arguments = this->arguments.substr(1);
	}

	this->where = parts[2];

	this->command = parts[1];
	if(this->command == "PRIVMSG")
		this->type = EntryType::Text;
	if(this->command == "JOIN")
		this->type = EntryType::Join;
	if(this->command == "PART")
		this->type = EntryType::Part;
	
	if(parts.size() > 3) {
		this->arguments = join(parts.begin() + 3, parts.end(), " ");
		if(this->arguments[0] == ':')
			this->arguments = this->arguments.substr(1);
	}
}

string Entry::nick() const {
	return this->who.substr(1, this->who.find("!") - 1);
}

bool NoopPredicate(Entry &) { return true; }

RegexPredicate::RegexPredicate(string regex) : _regex{regex} { }

bool RegexPredicate::operator()(Entry &e) {
	boost::regex r{_regex, boost::regex::perl};

	if(e.type != EntryType::Text)
		return false;

	try {
		return boost::regex_search(e.arguments, r, boost::match_default);
	} catch(exception &ex) {
		throw make_except(string{"regex exception: "} + ex.what());
	}
}

AndPredicate::AndPredicate(EntryPredicate p1, EntryPredicate p2)
	: _p1{p1}, _p2{p2} { }

bool AndPredicate::operator()(Entry &e) { return _p1(e) && _p2(e); }

Journal::Journal(Database &db, string table) : _db{db}, _table{table} { }

sqlite_int64 Journal::upsert(Entry &entry) {
	createTable();
	auto tran = _db.transaction();
	if(entry.id == -1) {
		_db.executeVoid("INSERT INTO " + _table + "(ts, sent, etype, network, contents)"
				+ " VALUES(?1, ?2, ?3, ?4, ?5)", entry.timestamp, (int)entry.sent,
				(int)entry.etype, entry.network, entry.contents);
		entry.id = sqlite3_last_insert_rowid(_db.getDB());
	} else {
		_db.executeVoid("UPDATE " + _table
				+ " SET ts = ?1, sent = ?2, etype = ?3, network = ?4, contents = ?5"
				+ " WHERE id = ?6", entry.timestamp, (int)entry.sent,
				(int)entry.etype, entry.network, entry.contents, entry.id);
	}
	return entry.id;
}
void Journal::log(sqlite_int64 ts, string msg) {
	cerr << ts << ":==log==: " << msg << endl;
	Entry e{-1, ts, SentType::Log, ExecuteType::None, "{log}", msg};
	this->upsert(e);
}

vector<Entry> Journal::filter(string sql, EntryPredicate predicate, int limit) {
	createTable();
	vector<Entry> results;
	if(limit == 0) return results;

	auto row = _db.execute(sql);
	while((limit >= 0 ? (int)results.size() < limit : true)
			&& row.status() == SQLITE_ROW) {
		Entry e{row.getLong(0), row.getLong(1), (SentType)row.getInteger(2),
			(ExecuteType)row.getInteger(3), row.getString(4), row.getString(5)};

		if(predicate(e))
			results.push_back(e);
		row.step();
	}
	if(row.status() == SQLITE_DONE) return results;
	if(row.status() != SQLITE_ROW)
		throw make_except("sqite error: " + to_string(row.status()));
	return results;
}

vector<Entry> Journal::fetch(EntryPredicate predicate, int limit) {
	return this->filter("SELECT * FROM " + _table + " ORDER BY ts DESC",
			predicate, limit);
}

vector<Entry> Journal::ffetch(EntryPredicate predicate, int limit) {
	return this->filter("SELECT * FROM " + _table + " ORDER BY ts ASC",
			predicate, limit);
}

Entry Journal::fetch(sqlite_int64 id) {
	createTable();

	auto row = _db.execute("SELECT * FROM " + _table + " WHERE id = ?1", id);
	// TODO: correct return here
	if(row.status() == SQLITE_DONE)
		throw -1;
	if(row.status() != SQLITE_ROW)
		throw -2;

	Entry e{row.getLong(0), row.getLong(1), (SentType)row.getInteger(2),
		(ExecuteType)row.getInteger(3), row.getString(4), row.getString(5)};
	return e;
}


sqlite_int64 Journal::size() {
	createTable();
	return _db
		.executeScalar<sqlite_int64>("SELECT COUNT(1) FROM " + _table)
		.value_or(0);
}

void Journal::createTable() {
	if(_tableCreated) return;

	auto tran =  _db.transaction();
	_db.executeVoid("CREATE TABLE IF NOT EXISTS " + _table
			+ " (id integer primary key, ts integer, "
			+ "  sent integer, etype integer, network string, contents string)");

	_tableCreated = true;
}

