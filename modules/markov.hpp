#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include "function.hpp"

// Handles returning markov chains
class MarkovFunction : public Function {
	public:
		virtual std::string run(FunctionArguments fargs);
		virtual void passive(global::ChatLine line, bool parsed);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);
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
