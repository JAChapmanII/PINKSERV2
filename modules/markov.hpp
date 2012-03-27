#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include "function.hpp"

// Handles returning markov chains
class MarkovFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};
// list chain count
class ChainCountFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_MARKOV_HPP
