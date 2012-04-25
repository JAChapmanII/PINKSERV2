#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include "function.hpp"

// Handles returning markov chains
class MarkovFunction : public Function {
	public:
		MarkovFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string passive(ChatLine line, bool parsed);
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
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

// autocorrect function
class CorrectionFunction : public Function {
	public:
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string passive(ChatLine line, bool parsed);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::string correct(std::string line);
};

// dictionary size
class DictionarySizeFunction : public Function {
	public:
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};


#endif // MODULES_MARKOV_HPP
