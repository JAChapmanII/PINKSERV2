#ifndef MODULES_HPP
#define MODULES_HPP

#include <map>
#include <vector>
#include <string>
#include <functional>
#include "variable.hpp"
#include "pvm.hpp"
#include "bot.hpp"

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
	template<typename... Args>
	struct IFWrapper<void, Args...> {
		IFWrapper(std::function<void(Args...)> func);

		Variable operator()(std::vector<Variable> args);

		private:
			std::function<void(Args...)> _func;
	};


	template<typename Ret, typename... Args>
			IFWrapper<Ret, Args...> make_wrapper(Bot *bot, Ret (*func)(Args...));
	template<typename Ret, typename... Args>
			IFWrapper<Ret, Args...> make_wrapper(Bot *bot, Ret (*func)(Bot &, Args...));

	extern std::map<std::string, InjectedFunction> hfmap;
	extern std::vector<Module> modules;

	bool init(Bot *bot);
}

#include "modules.inc.hpp"

#endif // MODULES_HPP
