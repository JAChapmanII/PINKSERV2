#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;
using std::getline;
using std::to_string;

#include <vector>
using std::vector;

#include <random>
using std::random_device;

#include "sekisa/util.hpp"
using util::fromString;
using util::split;
using util::startsWith;
#include "sekisa/db.hpp"
using zidcu::Database;

#include "pbrane/modules.hpp"
#include "pbrane/modules_gen.hpp"
#include "pbrane/expression.hpp"
#include "pbrane/parser.hpp"
#include "pbrane/regex.hpp"

#include "bot.hpp"
#include "sed.hpp"
#include "ngram.hpp"

namespace config {
	static string databaseFileName = "PINKSERV2.db";
}

struct PrivateMessage {
	string network{};
	string message{};
	string nick{};
	string target{};
	Bot &bot;
};
typedef bool (*hook)(PrivateMessage pmsg);

bool regexHook(PrivateMessage pmsg);
bool perfHook(PrivateMessage pmsg);

vector<hook> hooks = { &regexHook, &perfHook };

void prettyPrint(string arg);
void teval(vector<string> args);
void setupBot(Bot *bot);

void setupBot(Bot *bot) {
	modules::init<Bot>(bot);
	cerr << "    dictionary size: " << bot->dictionary.size() << endl;
}

void prettyPrint(string arg) {
	cout << arg << endl;
	try {
		auto et = parse(arg);
		cout << et->pretty(' ', 4) << endl;
	} catch(ParseException e) {
		cerr << e.pretty() << endl;
	}
}

void teval(vector<string> args) {
	random_device randomDevice;
	auto seed = randomDevice();

	Database db{config::databaseFileName};
	Options opts{};
	opts.seed = seed;
	Bot pbrane{db, opts, Clock{}, setupBot};

	pbrane.vars.set("bot.owner", "jac");
	pbrane.vars.set("bot.admins", "jac");

	if(!args.empty()) {
		for(auto &arg : args) {
			if(arg.empty() || startsWith(arg, "--"))
				continue;
			cout << ": " << arg << endl;

			try {
				auto expr = Parser::parse(arg);
				cout << "expr: " << (expr ? "true" : "false") << endl;

				// print computed AST
				cout << "final: " << endl;
				cout << expr->pretty() << endl;
				cout << "stringify: " << expr->toString() << endl;

				cout << "result: " << expr->evaluate(pbrane.vm, "jac").toString() << endl;
				// TODO: other exception types...
			} catch(ParseException e) {
				cout << e.pretty() << endl;
			} catch(StackTrace &e) {
				cout << e.toString() << endl;
			} catch(string &s) {
				cout << "\t: " << s << endl;
			}
		}
		return;
	}

	while(cin.good() && !cin.eof()) {
		string nick, line;
		getline(cin, nick);
		getline(cin, line);
		if(nick.empty() || line.empty())
			break;

		try {
			auto expr = Parser::parse(line);

			cerr << "eval'ing: " << line << " as " << nick << endl;
			cerr << "final AST: " << endl;
			cerr << expr->pretty() << endl;
			cerr << "stringify: " << expr->toString() << endl;

			string res = expr->evaluate(pbrane.vm, nick).toString();
			cerr << "result: " << res << endl;
			cout << nick + ": " << res << endl;

			// TODO: other exception types
		} catch(ParseException e) {
			cerr << e.pretty() << endl;
		} catch(StackTrace &e) {
			cout << e.toString() << endl;
		} catch(string &s) {
			cerr << "\texception: " << s << endl;
			cout << nick + ": error: " + s << endl;
		}
	}
}


void setupDB(Database &db);
void setupDB(Database &db) {
	db.executeVoid("PRAGMA cache_size = 10000;");
	db.executeVoid("PRAGMA page_size = 8192;");
	db.executeVoid("PRAGMA temp_store = MEMORY;");
	db.executeScalar<string>("PRAGMA journal_mode = WAL;");
	db.executeVoid("PRAGMA synchronous = NORMAL;");
}

void importGoogleNGramData();
void importGoogleNGramData() {
	Database db{config::databaseFileName};
	setupDB(db);
	Dictionary dictionary{db};

	const int minYear = 2015 - 30;
	const int maxOrder = 6;

	vector<Database> ngramDBs{maxOrder};
	vector<ngramStore> ngramStores{};
	for(int i = 0; i < maxOrder; ++i) {
		ngramDBs[i].open("ngrams_" + to_string(i) + ".db");
		setupDB(ngramDBs[i]);
		ngramStores.emplace_back(ngramDBs[i]);
	}

	Clock clock{};
	auto lastTime = clock.now();

	sqlite_int64 lines = 0, totalLines = 0;
	string line;
	// while there is more input coming
	while(!cin.eof()) {
		// read the current line of input
		getline(cin, line);

		if(line.empty())
			continue;

		totalLines++;

		if(lastTime < clock.now() - 10) {
			lastTime = clock.now();
			cerr << lastTime << " " << totalLines << " " << lines << endl;
		}

		auto fields = util::split(line, "\t");
		if(fields.size() != 4) {
			cerr << "fields.size: " << fields.size() << endl;
			cerr << "invalid line: \"" << line << "\"" << endl;
			continue;
		}

		auto ngram = fields[0];
		auto year = util::fromString<int>(fields[1]);
		auto count = util::fromString<int>(fields[2]);
		//auto vcount = util::fromString<int>(fields[3]);

		if(year < minYear)
			continue;

		auto bits = util::split(ngram, " ");
		if(bits.size() >= maxOrder) {
			cerr << "bits.size over maxOrder: " << bits.size()
				<< " >= " << maxOrder << endl;
			cerr << "invalid line: \"" << line << "\"" << endl;
			continue;
		}

		auto atom = dictionary[bits.back()];
		bits.pop_back();

		prefix_t prefix;
		for(auto &bit : bits)
			prefix.push_back(dictionary[bit]);

		{
			auto tran = ngramDBs[prefix.size()].transaction();
			ngramStores[prefix.size()].increment(ngram_t{prefix, atom}, count);
		}

		lines++;
	}

	lastTime = clock.now();
	cerr << lastTime << " " << totalLines << " " << lines << endl
		<< lastTime << " done" << endl;
}

int main(int argc, char **argv) {
	vector<string> args;
	for(int i = 1; i < argc; ++i)
		args.push_back(argv[i]);

	Options opts{};
	for(auto &arg : args) {
		if (arg == "--teval") {
			teval(args);
			return 0;
		}
		if(arg == "--pprint") {
			for(auto &arg2 : args)
				if(!startsWith(arg2, "--"))
					prettyPrint(arg2);
			return 0;
		}
		if(arg == "--importGoogle") {
			importGoogleNGramData();
			return 0;
		}

		if(arg == "--import") {
			opts.import = true;
			cerr << "pbrane: import mode enabled" << endl;
		} else if(arg == "--debugSQL") {
			opts.debugSQL = true;
			cerr << "pbrane: debug sql enabled" << endl;
		} else if(arg == "--debugEventSystem") {
			opts.debugEventSystem = true;
			cerr << "pbrane: debug event system enabled" << endl;
		} else if(arg == "--debugFunctionBodies") {
			opts.debugFunctionBodies = true;
			cerr << "pbrane: debug function bodies enabled" << endl;
		} else if(arg == "--importLog") {
			opts.import = true;
			opts.importLog = true;
			cerr << "pbrane: import log enabled" << endl;
		} else {
			opts.seed = fromString<unsigned int>(argv[1]);
		}
	}

	if(args.empty()) {
		random_device randomDevice;
		opts.seed = randomDevice();
	}

	Database db{config::databaseFileName};
	Bot pbrane{db, opts, Clock{}, setupBot};

	// while there is more input coming
	while(!cin.eof() && !pbrane.done) {
		// read the current line of input
		string line;
		getline(cin, line);

		if(line.find_first_not_of(" \t\r\n") == string::npos)
			continue;

		string network;
		auto ts = Clock{}.now();

		if(!opts.importLog) {
			network = line.substr(0, line.find(" "));
			line = line.substr(line.find(" ") + 1);

			if(line.find_first_not_of(" \t\r\n") == string::npos)
				continue;
		} else {
			auto fs = util::split(line, "|");
			if(fs.size() < 3) {
				cerr << "unable to parse log line" << endl;
				cerr << "line: " << line << endl;
				return 32;
			}

			ts = util::fromString<sqlite_int64>(fs[0]);
			network = fs[1];
			line = util::join(fs.begin() + 2, fs.end(), " ");
		}

		Entry entry{ts, network, line};
		pbrane.journal.upsert(entry);

		auto fields = split(line);
		if(fields.empty()) continue;

		if(entry.type == EntryType::Text) {
			auto nick = entry.nick();
			auto message = entry.arguments;
			auto target = entry.where;

			if(target == pbrane.vars.get("bot.nick").toString())
				target = nick;

			// TODO: simplify construction?
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("where", target);
			pbrane.vars.set("text", message);

			// check for a special hook
			bool wasHook = false;
			if(!opts.import) {
				for(auto h : hooks)
					if((*h)({ network, message, nick, target, pbrane })) {
						wasHook = true;
						break;
					}
			}

			if(wasHook)
				entry.etype = ExecuteType::Hook;
			// if the line is a ! command, run it
			else if(message[0] == '!' && message.length() > 1) {
				// it might be a !: to force intepretation line
				if(message.size() > 1 && message[1] == ':')
					pbrane.process(network, message.substr(2), nick, target);
				else
					pbrane.process(network, message, nick, target);
				entry.etype = ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"${" && message.back() == '}') {
				pbrane.process(network, message, nick, target);
				entry.etype = ExecuteType::Function;
			} else if(message.substr(0, 2) == (string)"::") {
				pbrane.process(network, message, nick, target);
				entry.etype = ExecuteType::Function;
			}
			// otherwise, run on text triggers
			else {
				entry.etype = ExecuteType::None;

				vector<Variable> results = pbrane.events.process(EventType::Text, pbrane.vm);
				if(results.size() == 1)
					pbrane.send(network, target, results.front().toString(), true);
			}
		}
		if(entry.type == EntryType::Join) {
			auto nick = entry.nick(), where = entry.where;

			// TODO: proper environment for triggers
			pbrane.vars.set("nick", nick);
			pbrane.vars.set("where", where);
			pbrane.vars.erase("text");

			auto results = pbrane.events.process(EventType::Join, pbrane.vm);
			if(results.size() == 1)
				pbrane.send(network, where, results.front().toString(), true);
		}
		if(entry.type == EntryType::Nick) {
			;// run nick triggers
		}
		if(entry.type == EntryType::Part || entry.type == EntryType::Quit) {
			;// run leave triggers
		}

		pbrane.journal.upsert(entry);
	}

	cerr << "pbrane: exited main loop" << endl;
	return 0;
}

bool regexHook(PrivateMessage pmsg) {
	if(pmsg.message[0] != 's')
		return false;
	if(pmsg.message.size() < 2)
		return false;
	if(((string)":/|").find(pmsg.message[1]) == string::npos)
		return false;

	auto regex = pmsg.message.substr(1);
	pmsg.bot.send(pmsg.network, pmsg.target, s(&pmsg.bot, regex), true);

	return true;
}

#include <time.h>

bool perfHook(PrivateMessage pmsg) {
	// TODO: lots of duplicated logic...
	static string pname{"!perf "};
	if(!util::startsWith(pmsg.message, pname))
		return false;

	auto script = pmsg.message.substr(pname.size());
	string result{""};

	bool triedParse{false}, triedEval{false};
	clock_t startParse, endParse;
	clock_t startEval, endEval;

	clock_t start = clock();

	bool simpleCall{false};
	try {
		// TODO: RAII wrapper?
		triedParse = true;
		startParse = clock();
		auto expr = Parser::parse(script);
		endParse = clock();

		if(!expr) {
			result = "Parser::parse(script) result is null";
		} else {
			if(expr->type == "!") simpleCall = true;
			triedEval = true;
			startEval = clock();
			result = expr->evaluate(pmsg.bot.vm, pmsg.nick).toString();
			endEval = clock();
		}
	} catch(ParseException e) {
		result = e.msg + " @" + util::asString(e.idx);
		endParse = clock();
	} catch(StackTrace &e) {
		if(e.type == ExceptionType::FunctionDoesNotExist && simpleCall) {
			result = "simple call to nonexistant function error supressed";
		} else {
			result = e.toString();
		}
		endEval = clock();
	} catch(string &s) {
		result = s;
		if(triedParse)
			endParse = clock();
		if(triedEval)
			endEval = clock();
	}

	clock_t end = clock();

	double  parseMs = (double)(endParse - startParse)  / CLOCKS_PER_SEC * 1000,
			evalMs = (double)(endEval - startEval) / CLOCKS_PER_SEC * 1000,
			totalMs = (double)(end - start) / CLOCKS_PER_SEC *  1000;

	result = string{"["}
		+ (triedParse ? "parse(" + to_string(parseMs) + ") " : "")
		+ (triedEval ? "eval(" + to_string(evalMs) + ") " : "")
		+ "total(" + to_string(totalMs) + ")"
		+ "] " + result;

	pmsg.bot.send(pmsg.network, pmsg.target, result, true);

	return true;
}

