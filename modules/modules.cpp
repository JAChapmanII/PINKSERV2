#include "modules.hpp"
using std::string;

#include <iostream>
using std::cerr;
using std::endl;

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::fstream;

#include "config.hpp"

// module includes
#include "core.hpp"
#include "markov.hpp"
#include "math.hpp"
#include "regex.hpp"
#include "script.hpp"
#include "simple.hpp"
#include "todo.hpp"

std::map<string, Function *> modules::map;
static bool modules_inited = false;

bool modules::init(std::string fileName) {
	if(modules_inited)
		return true;

	map["o/"] = new WaveFunction();
	map["fish"] = new FishFunction();
	map["<3"] = new LoveFunction();
	map["sl"] = new TrainFunction();
	map["dubstep"] = new DubstepFunction();
	map["or"] = new OrFunction();
	map["yes"] = new YesFunction(config::nick);
	map["say"] = new SayFunction();

	map["set"] = new SetFunction();
	map["++"] = new IncrementFunction();
	map["--"] = new DecrementFunction();
	map["erase"] = new EraseFunction();
	map["value"] = new ValueFunction();
	map["list"] = new ListFunction();
	map["lg"] = new BinaryLogFunction();

	map["s"] = new RegexFunction();
	map["push"] = new PushFunction();
	map["rlist"] = new ListRegexesFunction();
	map["invoke"] = new InvokeFunction();

	map["markov"] = new MarkovFunction();
	map["count"] = new ChainCountFunction();

	map["todo"] = new TodoFunction(config::todoFileName);

	map["ignore"] = new IgnoreFunction();
	map["help"] = new HelpFunction();

	map["on"] = new OnRegexFunction();
	map["explain"] = new ExplainFunction();

	ifstream in(fileName, fstream::binary);
	cerr << "module init: ";
	while(!in.eof() && in.good()) {
		int length = in.get();
		if(!in.good()) {
			break;
		}
		string name;
		for(int i = 0; i < length; ++i) {
			int c = in.get();
			if(!in.good()) {
				break;
			}
			name += (string)"" + (char)c;
		}
		Function *f = NULL;
		for(auto module : map)
			if(module.second->name() == name)
				f = module.second;
		if(f != NULL) {
			in >> *f;
		} else {
			cerr << " (unable to find function! " << name << ") " << endl;
			break;
		}
		cerr << " " << name;
	}
	cerr << endl;
	return true;
}

/*
ReplaceFunction
RegexFunction
PredefinedRegexFunction
PushFunction
PushXMLFunction
TodoFunction
WaveFunction
LoveFunction
FishFunction
TrainFunction
DubstepFunction
OrFunction
YesFunction
BinaryLogFunction
SetFunction
EraseFunction
ListFunction
IncrementFunction
DecrementFunction
IgnoreFunction
MarkovFunction
ChainCountFunction
*/

bool modules::deinit(std::string fileName) {
	ofstream out(fileName, fstream::binary | fstream::trunc);
	cerr << "module deinit: ";
	if(out.good()) {
		for(auto m : map) {
			string name = m.second->name();
			unsigned char length = name.length();
			out << length;
			for(int i = 0; i < length; ++i)
				out << name[i];
			out << (*m.second);
			cerr << " " << name;
		}
	} else
		cerr << "out not good";
	cerr << endl;

	for(auto m : map)
		delete m.second;
	return true;
}

