#ifndef EXPRESSIONTREE_HPP
#define EXPRESSIONTREE_HPP

#include <vector>
#include <string>
#include <utility>
#include "tokenfragment.hpp"

// TODO: struct -> class?
struct ExpressionTree {
	bool folded;
	unsigned eid;
	TokenFragment fragment;
	ExpressionTree *child, *rchild;
	ExpressionTree *prev, *next;

	ExpressionTree(TokenFragment ifrag, unsigned ieid);
	~ExpressionTree();

	static std::vector<std::pair<unsigned, unsigned>> delimitExpressions(std::vector<TokenFragment> &fragments);
	static ExpressionTree *parse(std::string statement);
	// TODO: move operator type/precedence map out of here?
	enum OperatorType { Binary, Prefix, Suffix };
	static ExpressionTree *treeify(ExpressionTree *begin, ExpressionTree *end);
	static ExpressionTree *dropSemicolons(ExpressionTree *begin, ExpressionTree *end);


	bool validAssignmentOperand();
	bool validOperand();
	bool validUrnaryOperand();

	bool validIdentifier();

	bool isSpecial(std::string token);

	void print(int level = 0, bool sprint = false);
	std::string toString(bool all = true);

	// TODO: ability to tag ExpressionTree as various types. string, int,
	// TODO: double, variable, function?

	std::string evaluate(std::string nick);
};

#endif // EXPRESSIONTREE_HPP
