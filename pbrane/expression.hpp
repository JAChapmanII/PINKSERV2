#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <string>
#include <vector>
#include <memory>
#include "variable.hpp"
#include "pvm.hpp"

using StackFrame = std::string;

struct StackFrameLifetime;


// may be thrown during Expression::evaluate
struct StackTrace {
	std::vector<StackFrame> frames{};
	bool hasError{false};
	std::string error{};
	std::string owner{};

	std::string toString() const;
	void except(std::string err);

	StackFrameLifetime push(StackFrame frame);
};

struct StackFrameLifetime {
	StackFrameLifetime(StackTrace &trace, int which = -1);
	~StackFrameLifetime();

	StackFrameLifetime(const StackFrameLifetime &rhs) = delete;
	StackFrameLifetime &operator=(StackFrameLifetime &rhs) = delete;

	StackFrameLifetime(StackFrameLifetime &&rhs);

	private:
		StackTrace &_trace;
		int _which;
};

struct Expression {
	std::string type;
	/* if type == str, just use text
	 * if type == strcat, evaluate all args and combine them with " "
	 * otherwise, intepret
	 */
	std::string text{}; // if type == str
	std::vector<std::unique_ptr<Expression>> args{};

	// construct expression without arguments
	Expression(std::string itype, std::string itext = "");

	// construct by stealing arguments from iargs
	Expression(std::string itype, std::vector<std::unique_ptr<Expression>> &iargs);

	// copy constructor
	Expression(const Expression &rhs);

	// return a parseable reperesentation of this
	std::string toString() const;

	// return a human readable (but not parseable) representation of this
	std::string pretty(char ident = ' ', int count = 1, int level = 0) const;

	// return a oneline human readable representation of this
	std::string prettyOneLine() const;

	// return the result of evaluating the expression
	// note: may throw StackTrace if there's an error
	Variable evaluate(Pvm &vm, std::string who) const;

	private:
		Variable evaluate(Pvm &vm, StackTrace &context) const;
};

std::string reEscape(std::string str);

#endif // EXPRESSION_HPP
