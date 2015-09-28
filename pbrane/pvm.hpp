#ifndef PVM_HPP
#define PVM_HPP

#include <functional>
#include <vector>
#include <map>
#include <string>
#include "variable.hpp"
#include "varstore.hpp"

using InjectedFunction = std::function<Variable(std::vector<Variable>)>;

struct Pvm {
	// options
	bool debugFunctionBodies{false};

	// variable store
	VarStore vars;

	// store of injected functions
	std::map<std::string, InjectedFunction> functions{};

	Pvm(VarStore ivars, bool idBF = false)
		: debugFunctionBodies{idBF}, vars{ivars} { }
};

#endif // PVM_HPP
