#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include <functional>
#include "variable.hpp"
#include "pvm.hpp"

namespace modules {
	struct Module {
		std::string name;
		std::string desc;
		bool loaded;
	};

	template<typename Ret, typename... Args>
	struct IFWrapper {
		IFWrapper(std::function<Ret(Args...)> func);

		Variable operator()(std::vector<Variable> args);

		private:
			std::function<Ret(Args...)> _func;
	};

	template<typename Ret, typename... Args>
			IFWrapper<Ret, Args...> make_wrapper(Ret (*func)(Args...));

	extern std::map<std::string, InjectedFunction> hfmap;
	extern std::vector<Module> modules;

	bool init();
}

#include "modules.inc.hpp"

#endif // MODULES_HPP
