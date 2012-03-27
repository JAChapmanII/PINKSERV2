#ifndef MODULES_CORE_HPP
#define MODULES_CORE_HPP

#include "function.hpp"

// ignore
class IgnoreFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_CORE_HPP
