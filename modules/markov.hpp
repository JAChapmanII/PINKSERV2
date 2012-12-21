#ifndef MODULES_MARKOV_HPP
#define MODULES_MARKOV_HPP

#include "function.hpp"

// Handles returning markov chains
class MarkovFunction : public Function {
	public:
		MarkovFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string passive(ChatLine line, bool parsed);

	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);
};
// list chain count
class ChainCountFunction : public Function {
	public:
		ChainCountFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// autocorrect function
class CorrectionFunction : public Function {
	public:
		CorrectionFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string passive(ChatLine line, bool parsed);

	protected:
		std::string correct(std::string line);
};

// dictionary size
class DictionarySizeFunction : public Function {
	public:
		DictionarySizeFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};

// random word
class RandomWordFunction : public Function {
	public:
		RandomWordFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
};


#endif // MODULES_MARKOV_HPP
