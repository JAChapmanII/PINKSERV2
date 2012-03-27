#include "modules.hpp"
using std::string;

#include "config.hpp"

// module includes
#include "markov.hpp"
#include "math.hpp"
#include "regex.hpp"
#include "simple.hpp"
#include "todo.hpp"

std::map<string, Function *> modules::map;
static bool modules_inited = false;

bool modules::init() {
	if(modules_inited)
		return true;

	map["o/"] = new WaveFunction();
	map["fish"] = new FishFunction();
	map["<3"] = new LoveFunction();
	map["sl"] = new TrainFunction();
	map["dubstep"] = new DubstepFunction();
	map["or"] = new OrFunction();

	map["set"] = new SetFunction();
	map["++"] = new IncrementFunction();
	map["--"] = new DecrementFunction();
	map["erase"] = new EraseFunction();
	map["value"] = new ValueFunction();
	map["list"] = new ListFunction();
	map["s"] = new ReplaceFunction();
	map["s2"] = new RegexFunction();

	map["push"] = new PushFunction();
	map["push2"] = new PushXMLFunction();

	map["markov"] = new MarkovFunction();
	map["count"] = new ChainCountFunction();
	map["yes"] = new YesFunction(config::nick);

	map["todo"] = new TodoFunction(config::todoFileName);

	map["lg"] = new BinaryLogFunction();
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

bool modules::deinit() {
	for(auto i : map)
		delete i.second;
}

