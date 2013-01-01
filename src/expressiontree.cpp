#include "expressiontree.hpp"
using std::vector;
using std::string;
using std::pair;

#include <stack>
using std::stack;

#include <algorithm>
using std::find;

#include <cmath>

#include "permission.hpp"

#include "util.hpp"
using util::startsWith;
using util::contains;
using util::asString;
using util::fromString;

#include "global.hpp"


// TODO: remove after debug?
#include <iostream>
using std::cerr;
using std::endl;
static void printexprends(pair<unsigned, unsigned> sexpr, vector<TokenFragment> frags);

void printexprends(pair<unsigned, unsigned> sexpr, vector<TokenFragment> frags) { // {{{
	vector<unsigned> fstarts;
	string expr;
	for(auto frag : frags) {
		fstarts.push_back(expr.length());
		expr += frag.text + " ";
	}

	int first = fstarts[sexpr.first], second = fstarts[sexpr.second];
	//cout << first << ", " << second << endl;
	cerr << expr << endl;
	cerr << string(first, ' ') + '^' +
		string(second - first - 1, '~') + '^' << endl;
} // }}}


ExpressionTree::ExpressionTree(TokenFragment ifrag, unsigned ieid) : // {{{
		folded(false), eid(ieid), fragment(ifrag),
		child(NULL), rchild(NULL), prev(NULL), next(NULL) {
} // }}}
ExpressionTree::~ExpressionTree() { // {{{
	if(this->child)
		delete this->child;
	if(this->rchild)
		delete this->rchild;
	if(this->next)
		delete this->next;
} // }}}

vector<pair<unsigned, unsigned>> ExpressionTree::delimitExpressions( // {{{
		vector<TokenFragment> &fragments) {
	static vector<string> startSubCharacters = { "(", "{", "[" },
		stopSubCharacters = { ")", "}", "]" };
	stack<unsigned> sexprStack;
	vector<pair<unsigned, unsigned>> sexprlist;

	// insert a semicolon at the front and back to ensure correct parsing
	fragments.insert(fragments.begin(), TokenFragment(";", true));
	fragments.push_back(TokenFragment(";", true));

	static vector<string> stringDelimiters = { "'", "\"" };

	// squash strings
	vector<TokenFragment> fragments_squashed;
	for(unsigned i = 0; i < fragments.size(); ++i) {
		bool isDelimiter = false;
		for(auto delimiter : stringDelimiters) {
			if(fragments[i].isSpecial(delimiter)) {
				isDelimiter = true;
				break;
			}
		}
		if(isDelimiter)
			continue;
		fragments_squashed.push_back(fragments[i]);
	}
	fragments.clear();
	fragments.insert(fragments.begin(),
			fragments_squashed.begin(), fragments_squashed.end());

	// make a pass to make sure there are no empty semicolons
	vector<TokenFragment> fragments_noempty;
	for(unsigned i = 0; i < fragments.size() - 1; ++i)
		if(!(fragments[i].isSpecial(";") && fragments[i + 1].isSpecial(";")))
			fragments_noempty.push_back(fragments[i]);
	fragments.clear();
	fragments.insert(fragments.begin(),
			fragments_noempty.begin(), fragments_noempty.end());
	fragments.push_back(TokenFragment(";", true));

	// make a pass to ensure there aren't any strange mismatched sub
	// expression related tokens and to find start and ends of all
	// expressions
	sexprStack.push(0);
	for(unsigned i = 1; i < fragments.size(); ++i) {
		if(fragments[i].special) {
			// if we have a semicolon,
			if(fragments[i].text == ";") {
				// make sure the semicolon is duplicated
				if((i != fragments.size() - 1) &&
						!fragments[i - 1].isSpecial(";") &&
						!fragments[i + 1].isSpecial(";") &&
						// TODO: util::contains fails to compile here now >_>
						(find(stopSubCharacters.begin(), stopSubCharacters.end(),
								fragments[i + 1].text) == stopSubCharacters.end())) {
					fragments.insert(fragments.begin() + i, TokenFragment(";", true));
				}

				// close current expression and start a new one here
				sexprlist.push_back({ sexprStack.top(), i });
				sexprStack.pop();
				sexprStack.push(i);
				continue;
			}

			// open sub expressions
			if(contains(startSubCharacters, fragments[i].text)) {
				sexprStack.push(i);
				fragments.insert(fragments.begin() + i + 1, TokenFragment(";", true));
				sexprStack.push(i + 1);
				i++;
				continue;
			}

			// close expressions
			unsigned pIndex = 0;
			for(; pIndex < stopSubCharacters.size(); ++pIndex)
				if(stopSubCharacters[pIndex] == fragments[i].text)
					break;
			// couldn't find the stop character, just move on
			if(pIndex == stopSubCharacters.size())
				continue;

			// if we're at an end, but there was no semicolon before now, add
			// one there and move back to it
			if(!fragments[i - 1].special || fragments[i - 1].text != ";") {
				fragments.insert(fragments.begin() + i, TokenFragment(";", true));
				i--;
				continue;
			}

			// if there is semicolon on the top (there should be for
			// subexpressions) then ditch it as it's not needed
			if(!sexprStack.empty() && (fragments[sexprStack.top()].text == ";"))
				sexprStack.pop();
			// if the stack is empty there's nothing to close
			if(sexprStack.empty())
				throw (string)"extra " + fragments[i].text;

			// if the close matches up with the start
			if(fragments[sexprStack.top()].text == startSubCharacters[pIndex]) {
				// we have a clean end so push the pair and ditch tho top
				sexprlist.push_back({ sexprStack.top(), i });
				sexprStack.pop();
			} else {
				// otherwise we've got mismatched stuff
				throw (string)"mismatched " + fragments[sexprStack.top()].text +
					" with " + fragments[i].text;
			}
		}
	}
	// if there's exactly one thing left
	if(sexprStack.size() == 1) {
		unsigned i = sexprStack.top();
		// and if it's a semicolon
		if(fragments[i].special && fragments[i].text == ";") {
			// close the final expression and pop it
			sexprlist.push_back({ i, fragments.size() });
			sexprStack.pop();
		}
	}
	// if there's still things open, we have an error
	if(!sexprStack.empty())
		throw asString(sexprStack.size()) + " expressions unclosed";

	// make sure there are no empty subexpressions
	vector<pair<unsigned, unsigned>> sexprlist_noempty;
	for(auto sexpr : sexprlist) {
		if(sexpr.first == sexpr.second || sexpr.first == sexpr.second - 1)
			continue;
		sexprlist_noempty.push_back(sexpr);
	}

	return sexprlist_noempty;
} // }}}
ExpressionTree *ExpressionTree::parse(string statement) { // {{{
	// turn string into a set of fragments
	vector<TokenFragment> frags = TokenFragment::fragment(statement);

	// find ends of expressions
	vector<pair<unsigned, unsigned>> sexprlist = delimitExpressions(frags);

	// parse into a tree
	vector<ExpressionTree *> exprs;
	for(unsigned i = 0; i < frags.size(); ++i) {
		ExpressionTree *here = new ExpressionTree(frags[i], i);
		// hook up the previous to here
		if(exprs.size() > 0) {
			exprs[i - 1]->next = here;
			here->prev = exprs[i - 1];
		}
		exprs.push_back(here);
	}

	// store pointers to the starts and ends of all expressions
	vector<pair<ExpressionTree *, ExpressionTree *>> exprEnds;
	for(auto sexpr : sexprlist)
		exprEnds.push_back({ exprs[sexpr.first], exprs[sexpr.second] });

	ExpressionTree *last = NULL;
	// convert all subexpressions into a tree
	for(unsigned i = 0; i < exprEnds.size(); ++i) {
		printexprends(sexprlist[i], frags);
		last = treeify(exprEnds[i].first, exprEnds[i].second);
		//last->print();
	}

	// find the real starts and ends of the overall expression
	ExpressionTree *start = last, *end = NULL;
	for(; start->prev; start = start->prev)
		;// "spin"
	for(end = start; end->next; end = end->next)
		;// "spin"

	// last ditch attempt for leftover semicolons
	start = dropSemicolons(start, end);

	return start;
} // }}}

// TODO: remove after debug?
void ExpressionTree::print(int level, bool sprint) { // {{{
	//cerr << "(l:" << level << ") ";
	if(level == 0)
		cerr << this->fragment.text << endl;
	else if(sprint)
		cerr << string(level * 2, ' ') << "a: '" << this->fragment.text << "'" << endl;

	string sstring(level * 2 + 2, ' ');
	if(this->isSpecial("!")) {
		cerr << sstring << "name: " << this->child->fragment.text << endl;
		for(ExpressionTree *a = this->rchild; a; a = a->next) {
			if(a->fragment.special && a->folded) {
				//cerr << a->fragment.text << endl;
				a->print(level + 1, true);
			} else
				cerr << sstring << "arg: " << a->fragment.text << endl;
		}
	} else if(this->isSpecial("()")) {
		cerr << sstring << "(): " << endl;
		for(ExpressionTree *sexpr = this->child; sexpr; sexpr = sexpr->next) {
			sexpr->print(level + 1, true);
		}
	} else {
		if(this->child) {
			cerr << sstring << "l: " << this->child->fragment.text << endl;
			this->child->print(level + 1);
		}
		if(this->rchild) {
			cerr << sstring << "r: " << this->rchild->fragment.text << endl;
			this->rchild->print(level + 1);
		}
	}

	if(this->next && !sprint)
		this->next->print(level);
} // }}}

bool ExpressionTree::validAssignmentOperand() { // {{{
	if(!this->isSpecial("$"))
		return false;
	if(this->rchild && this->rchild->validOperand())
		return true;
	return false;
} // }}}
bool ExpressionTree::validOperand() { // {{{
	if(this->fragment.special && !this->folded)
		return false;
	return true;
} // }}}
bool ExpressionTree::validUrnaryOperand() { // {{{
	return false;
} // }}}

bool ExpressionTree::validIdentifier() { // {{{
	if(this->fragment.special)
		return false;
	return this->fragment.validIdentifier();
} // }}}

bool ExpressionTree::isSpecial(string token) { // {{{
	if(!this->fragment.special)
		return false;
	return (this->fragment.text == token);
} // }}}

// TODO: failure of: $t = (thing1; thing2) and
// TODO: func => (thing1; thing2)

// TODO: various debug things in here
ExpressionTree *ExpressionTree::treeify(ExpressionTree *begin, ExpressionTree *end) {
	static vector<string> assignments = { // {{{
			"=", "+=", "-=", "*=", "/=", "%=", "^=",
			"++", "--"
	};
	static vector<string> fassignments = { 
			"=>", "+=>"
	}; // }}}
	static vector<vector<pair<string, unsigned>>> precedenceMap = { // {{{
		{ { "++", Suffix }, { "--", Suffix } },
		{
			{ "!", Prefix }, { "$", Prefix },
			{ "++", Prefix }, { "--", Prefix },
			{ "+", Prefix }, { "-", Prefix }, { "~", Prefix }
		},
		{ { "^", Binary } },
		{ { "*", Binary }, { "/", Binary }, { "%", Binary } },
		{ { "+", Binary }, { "-", Binary } },
		{
			{ "<", Binary }, { "<=", Binary },
			{ ">", Binary }, { ">=", Binary }
		},
		{ { "==", Binary }, { "~=", Binary }, { "=~", Binary } },
		{ { "&&", Binary } },
		{ { "||", Binary } },
		{ { ":", Binary } },
		{ { "?", Binary } },
		{
			{ "=", Binary },
			{ "+=", Binary }, { "-=", Binary },
			{ "*=", Binary }, { "/=", Binary }, { "%=", Binary },
			{ "^=", Binary }
		},
		{ { "+=>", Binary }, { "=>", Binary } }
	}; // }}}

	// TODO: if previous before parenthesis is a function call, use this as
	// arguments? If no args currently?
	// if we've got a parenthized or braced subexpression, simply ditch the {{{
	// surroundings and hook the contents up directly
	if((begin->isSpecial("(") && end->isSpecial(")")) ||
			(begin->isSpecial("{") && end->isSpecial("}"))) {
		// make the inner semicolon enclosed sequence it's own thing
		ExpressionTree *sexpr0 = begin->next, *sexpr1 = end->prev;
		sexpr0->prev = NULL;
		sexpr1->next = NULL;

		// create a new expression tree for the expression
		ExpressionTree *here = new ExpressionTree(TokenFragment("()", true), 0);
		// set its child to be the inner sequence
		here->child = dropSemicolons(sexpr0, sexpr1);
		here->folded = true;

		// hook up start to surroundings
		here->prev = begin->prev;
		begin->prev->next = here;

		// hook up end to surroundings
		here->next = end->next;
		end->next->prev = here;

		// delete now unused () or {} tokens
		begin->next = NULL;
		end->next = NULL;
		delete begin;
		delete end;

		// return the inner contents
		return here;
	} // }}}

	// loop over precedence levels
	for(auto level : precedenceMap) { // {{{
		// loop over the expression
		for(ExpressionTree *here = begin->next; here != end; here = here->next) {
			if(!here) {
				cerr << "oops: ExpressionTree::treeify got NULL here? (never hit end)" << endl;
				break;
			}
			// if it's not a special token, skip it
			if(!here->fragment.special)
				continue;
			// if it's already folded, then skip it
			if(here->folded)
				continue;

			// if we have a function call, parse and bind arguments {{{
			if(here->isSpecial("!")) {
				// make sure there is a valid function name next
				if(!here->next)
					throw (string)"function call with no function name";
				if(!here->next->validIdentifier())
					throw here->next->fragment.text +
						" is not a valid identifier for a call";

				// save the places of needed information
				ExpressionTree *bang = here, *name = here->next,
					*farg = name->next, *larg = end->prev;

				// store the function name tree
				bang->child = name;
				name->prev = NULL;
				name->next = NULL;

				// drop the name from the tree list
				bang->next = farg;
				if(farg)
					farg->prev = bang;

				// mark this call as folded so we don't rehandle it
				bang->folded = true;

				// if there are no arguments
				if(!farg || farg == end) {
					// name shouldn't be followed by anything
					here->child->next = NULL;
					// make sure we don't have extra semicolons
					return treeify(begin, end);
				}

				// begind and end semicolons of args
				ExpressionTree *bSemicolon =
					new ExpressionTree({ ";", true }, end->eid - 1);
				ExpressionTree *eSemicolon =
					new ExpressionTree({ ";", true }, here->eid + 1);

				// make first arg have a semicolon before it
				farg->prev = bSemicolon;
				bSemicolon->next = farg;

				// make the last arg have a semicolon after it
				larg->next = eSemicolon;
				eSemicolon->prev = larg;

				// treeify arguments
				bang->rchild = treeify(bSemicolon, eSemicolon);

				// make the list into "bang <-> end" (drop arguments)
				bang->next = end;
				end->prev = bang;

				// make sure there aren't extra semicolons
				return treeify(begin, end);
			} // }}}

			// loop over all operators on this level
			for(auto op : level) {
				// if the op doesnt' match, skip it
				if(here->fragment.text != op.first)
					continue;
				// make sure it's the proper type
				switch(op.second) {
					case OperatorType::Binary: // {{{
						// check left and right sides
						if(contains(assignments, op.first)) {
							if(!here->prev->validAssignmentOperand()) {
								cerr << here->prev->fragment.text << endl;
								throw (string)"assignment with invalid lhs";
							}
						} else if(contains(fassignments, op.first)) {
							if(!here->prev->validIdentifier())
								throw (string)"function assignment with invalid lhs";
						} else {
							if(!here->prev->validOperand()) {
								cerr << "prev: " << here->prev->fragment.text << endl;
								here->prev->print();
								throw (string)"binary op (" + here->fragment.text +
									") with invalid lhs";
							}
						}
						if(!here->next->validOperand()) {
							//cout << "next: " << endl;
							//here->next->print();
							throw (string)"binary op (" + here->fragment.text +
								") with invalid rhs";
						}

						// fold into tree
						// left
						here->child = here->prev;
						//cout << "  child: " << here->child->fragment.text << endl;
						here->prev = here->child->prev;
						here->child->prev = NULL;
						here->child->next = NULL;
						here->prev->next = here;

						// right
						here->rchild = here->next;
						//cout << " rchild: " << here->rchild->fragment.text << endl;
						here->next = here->rchild->next;
						here->rchild->next = NULL;
						here->rchild->prev = NULL;
						here->next->prev = here;

						here->folded = true;
						//cout << "  child: " << here->child->fragment.text << endl;
						//cout << " rchild: " << here->rchild->fragment.text << endl;

						break; // }}}
					case OperatorType::Prefix: // {{{
						// TODO: unspecial case?
						if(op.first == "$") {
							if(!here->next->validIdentifier())
								throw here->next->fragment.text + " is not valid identifier";
							;//cout << "dollar" << endl;// do something?
						} else if(contains(assignments, op.first)) {
							if(!here->next->validAssignmentOperand())
								throw (string)"prefix assignment with bad operand";
						} else {
							if(!here->next->validUrnaryOperand())
								continue;
								//throw (string)"prefix operator with bad operand";
						}
						// TODO not special case
						if(op.first == "+" || op.first == "-")
							if(!here->prev->validOperand())
								continue;
						// right
						here->rchild = here->next;
						here->next = here->rchild->next;
						here->rchild->next = NULL;
						here->rchild->prev = NULL;
						here->next->prev = here;

						here->folded = true;

						break; // }}}
					case OperatorType::Suffix: // {{{
						if(contains(assignments, op.first)) {
							if(!here->prev->validAssignmentOperand())
								throw (string)"suffix assignment with bad operand";
						} else {
							if(!here->prev->validUrnaryOperand())
								continue;
								//throw (string)"suffix operator with bad operand";
						}
						// left
						here->child = here->prev;
						here->prev = here->child->prev;
						here->child->prev = NULL;
						here->child->next = NULL;
						here->prev->next = here;

						here->folded = true;

						break; // }}}
				}
			}
		}
	} // }}}

	return dropSemicolons(begin, end);
}

// TODO: some error stuff in here too
ExpressionTree *ExpressionTree::dropSemicolons( // {{{
		ExpressionTree *begin, ExpressionTree *end) {
	bool done = false;
	for(ExpressionTree *here = begin; !done;) {
		if(here == end)
			done = true;
		if(here == NULL) {
			if(here == end)
				break;
			if(here == begin)
				cerr << "ExpressionTree::dropSemicolons: begin is NULL? wat" << endl;
			cerr << "ExpressionTree::dropSemicolons: ERRORRRRR" << endl;
			break;
		}
		// if it's not a special token, skip it
		if(!here->fragment.special) {
			here = here->next;
			continue;
		}
		// if it's not a semicolon, skip it
		if(here->fragment.text != ";") {
			here = here->next;
			continue;
		}

		// hook "here" out
		if(here->prev)
			here->prev->next = here->next;
		if(here->next)
			here->next->prev = here->prev;

		ExpressionTree *tmp = here;
		if(here == begin) {
			if(here->prev)
				begin = here->prev;
			else
				begin = here->next;
		}
		if(here == end) {
			if(here->next)
				end = here->next;
			else
				end = here->prev;
		}

		// advance
		here = here->next;

		// ditch tmp
		// TODO: make these auto-pointers or something more C++-ey
		tmp->next = NULL;
		delete tmp;
	}

	return begin;
} // }}}

// TODO: this is far from done
// TODO: we could have a test suite for this... parse -> toString -> parse,
// TODO: compare? We modify it somewhat, but how badly do we mangle it?
// TODO: this is totally screwed up :( Needs to be parenthisized
string ExpressionTree::toString(bool all) { // {{{
	if(!this->fragment.special) {
		// TODO: use type tags to not do this?
		if(this->next && all)
			return "'" + this->fragment.text + "'; " + this->next->toString();
		else
			return "'" + this->fragment.text + "'";
	}

	if(this->isSpecial("()")) {
		string ret = "(" + this->child->toString() + ")";
		if(this->next && all)
			ret += "; " + this->next->toString();
		return ret;
	}

	if(this->isSpecial("$")) {
		if(this->next && all)
			return "$" + this->rchild->fragment.text + "; " + this->next->toString();
		else
			return "$" + this->rchild->fragment.text;
	}
	if(this->isSpecial("!")) {
		string ret = "(!" + this->child->fragment.text;
		for(ExpressionTree *arg = this->rchild; arg; arg = arg->next)
			ret += " (" + arg->toString(false) + ")";
		ret += ")";
		if(this->next && all)
			ret += "; " + this->next->toString();
		return ret;
	}
	string here;
	if(this->next && all)
		here = "(";
	if(this->child)
		here += this->child->toString() + " ";
	here += this->fragment.text;
	if(this->rchild)
		here += " " + this->rchild->toString();
	//here += ")";
	if(this->next && all)
		return here + ");" + this->next->toString();
		//return here + "; " + this->next->toString(true);
	return here;
} // }}}

// TODO: ability to tag ExpressionTree as various types. string, int,
// TODO: double, variable, function?

string ExpressionTree::evaluate(string nick) {
	if(!this->fragment.special) {
		return this->fragment.text;
	}
	// TODO: de-int this. double? Only when "appropriate"?
	// standard form: + - * / % {{{
	if(this->fragment.isSpecial("+")) {
		return asString(fromString<int>(this->child->evaluate(nick)) +
				fromString<int>(this->rchild->evaluate(nick)));
	}
	if(this->fragment.isSpecial("-")) {
		return asString(fromString<int>(this->child->evaluate(nick)) -
				fromString<int>(this->rchild->evaluate(nick)));
	}
	if(this->fragment.isSpecial("*")) {
		return asString(fromString<int>(this->child->evaluate(nick)) *
				fromString<int>(this->rchild->evaluate(nick)));
	}
	if(this->fragment.isSpecial("/")) {
		return asString(fromString<int>(this->child->evaluate(nick)) /
				fromString<int>(this->rchild->evaluate(nick)));
	}
	if(this->fragment.isSpecial("%")) {
		return asString(fromString<int>(this->child->evaluate(nick)) %
				fromString<int>(this->rchild->evaluate(nick)));
	} // }}}
	if(this->fragment.isSpecial("^")) {
		return asString(pow(fromString<int>(this->child->evaluate(nick)),
				fromString<int>(this->rchild->evaluate(nick))));
	}
	if(this->fragment.isSpecial(".")) {
		return this->child->evaluate(nick) + this->rchild->evaluate(nick);
	}
	if(this->fragment.isSpecial("=")) {
		// left child is $, with right child varname
		string var = this->child->rchild->fragment.text;
		string reval = this->rchild->evaluate(nick);
		// TODO: standard way of doing this is going to mean that if the
		// overall thing fails, we may have partial evals to roll back. Store
		// in special map, and do an evaulate and then an apply?
		if(!hasPermission(Permission::Write, nick, var))
			throw nick + " does not have permission to write to " + var;
		return (string)"wrote " + reval + " to $" + var;
	}
	if(this->fragment.isSpecial("!")) {
		string func = this->child->fragment.text;
		vector<string> args;
		for(ExpressionTree *arg = this->rchild; arg; arg = arg->next)
			args.push_back(arg->evaluate(nick));
		// TODO: check for function existence
		// TODO: move permission messages into throw'ing function?
		if(!hasPermission(Permission::Execute, nick, func))
			throw nick + " does not have permission to execute " + func;
		string ret = "called " + func;
		if(args.empty())
			return ret;
		ret += " with arg";
		if(args.size() > 1)
			ret += "s";
		for(string arg : args)
			ret += " " + arg;
		return ret;
	}
	throw (string)"unkown node { \"" + this->fragment.text + "\", " +
		(this->fragment.special ? "" : "not") + " special }, bug " +
		global::vars["bot.owner"] + " to fix";
}


