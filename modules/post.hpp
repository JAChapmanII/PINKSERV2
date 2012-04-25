#ifndef MODULES_POST_HPP
#define MODULES_POST_HPP

#include "function.hpp"

// Handles returning a POST-ed sentence
class POSTFunction : public Function {
	public:
		POSTFunction();
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
	protected:
		std::ostream &output(std::ostream &out);
		std::istream &input(std::istream &in);
};

// Handles training the tagger
class POSTTrainFunction : public Function {
	public:
		virtual std::string run(ChatLine line, boost::smatch matches);
		virtual std::string name() const;
		virtual std::string help() const;
		virtual std::string regex() const;
};

#endif // MODULES_POST_HPP
