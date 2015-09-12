#include "err.hpp"
#include "util.hpp"

namespace modules {
	namespace IFHelper {
		template<int Arity> struct ArityBinder {
			template<typename Func, typename Arg>
			decltype(auto) bind(Func func, Arg arg);
		};
		template<> struct ArityBinder<1> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg);
			}
		};
		template<> struct ArityBinder<2> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg, std::placeholders::_1);
			}
		};
		template<> struct ArityBinder<3> {
			template<typename Func, typename Arg>
			decltype(auto) operator()(Func func, Arg arg) {
				return std::bind(func, arg, std::placeholders::_1, std::placeholders::_2);
			}
		};

		template<typename Arg> Arg coerce(std::vector<Variable> &vars) {
			throw make_except("unexpected coerce type");
		}
		template<> std::string coerce(std::vector<Variable> &vars);
		template<> long coerce(std::vector<Variable> &vars);
		template<> double coerce(std::vector<Variable> &vars);
		template<> std::vector<Variable> coerce(std::vector<Variable> &vars);

		template<typename Ret> Variable makeVariable(Ret ret) {
			return Variable(ret, Permissions());
		}
		template<> Variable makeVariable(Variable var);

		template<typename F>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			if(!pargs.empty()) throw pargs.size();
			return func;
		}
		/*template<typename F, typename Arg>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			return std::bind(func, coerce<Arg>(pargs));
		}*/
		template<typename F, typename Arg, typename... Args>
				decltype(auto) if_bind(F func, std::vector<Variable> &pargs) {
			auto binder = ArityBinder<sizeof...(Args) + 1>{};
			auto fNext = binder(func, coerce<Arg>(pargs));
			return if_bind<decltype(fNext), Args...>(fNext, pargs);
		}
		template<typename Ret, typename... Args> decltype(auto)
				bind(std::function<Ret(Args...)> func, std::vector<Variable> pargs) {
			return if_bind<std::function<Ret(Args...)>, Args...>(func, pargs);
		}
	}

	template<typename Ret, typename... Args>
			IFWrapper<Ret, Args...>::IFWrapper(std::function<Ret(Args...)> func)
				: _func{func} { }
	template<typename Ret, typename... Args>
			Variable IFWrapper<Ret, Args...>::operator()(std::vector<Variable> args) {
		auto f = IFHelper::bind(_func, args);
		auto ret = f();
		return IFHelper::makeVariable(ret);
	}


	template<typename... Args>
			IFWrapper<void, Args...>::IFWrapper(std::function<void(Args...)> func)
				: _func{func} { }
	template<typename... Args>
			Variable IFWrapper<void, Args...>::operator()(std::vector<Variable> args) {
		auto f = IFHelper::bind(_func, args);
		f();
		return IFHelper::makeVariable("");
	}

	template<typename Ret, typename... Args> IFWrapper<Ret, Args...>
			make_wrapper(Ret (*func)(Args...)) {
		return IFWrapper<Ret, Args...>{func};
	}
}

