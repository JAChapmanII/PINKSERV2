#include "expressiontree.hpp"
using std::vector;
using std::string;
using std::pair;

#include <stack>
using std::stack;

#include <algorithm>
using std::find;

#include <boost/regex.hpp>
using boost::regex;

#include <cmath>

#include "permission.hpp"

#include "util.hpp"
using util::startsWith;
using util::contains;
using util::asString;
using util::fromString;
using util::join;

#include "global.hpp"
#include "modules.hpp"

#include <iostream>
using std::cerr;
using std::endl;

ExpressionTree::ExpressionTree(TokenFragment ifrag, unsigned ieid) :
		folded(false), eid(ieid), fragment(ifrag),
		child(NULL), rchild(NULL), prev(NULL), next(NULL) {
}
ExpressionTree::~ExpressionTree() {
	if(this->child)
		delete this->child;
	if(this->rchild)
		delete this->rchild;
	if(this->next)
		delete this->next;
}

// throw out string delimiters and set string properties
void squashStrings(vector<TokenFragment> &fragments) {
	static vector<string> stringDelimiters = { "'", "\"" };

	// squash strings
	vector<TokenFragment> fragments_squashed;
	for(unsigned i = 0; i < fragments.size(); ++i) {
		string delimiter;
		for(auto del : stringDelimiters)
			if(fragments[i].isSpecial(del))
				delimiter = del;

		if(delimiter.empty()) {
			// we're not on a boundary and need to do nothing
			fragments_squashed.push_back(fragments[i]);
			continue;
		}

		// TODO: past bounds?
		bool emptyString = fragments[i + 1].isSpecial(delimiter);
		if(!emptyString && !(fragments[i + 2].isSpecial(delimiter)))
			throw (string)"strangely delimited string";

		TokenFragment stringPart;
		if(!emptyString) {
			++i; // advance onto body
			stringPart = fragments[i];
		}
		// record string properties
		stringPart.isString = true;
		stringPart.sdelim = delimiter;
		fragments_squashed.push_back(stringPart);

		++i; // advance onto end delimiter
		// continue on past the delimiter
	}
	fragments.clear();
	fragments.insert(fragments.begin(),
			fragments_squashed.begin(), fragments_squashed.end());
}

vector<pair<unsigned, unsigned>> ExpressionTree::delimitExpressions(
		vector<TokenFragment> &fragments) {
	static vector<string> startSubCharacters = { "(", "{", "[" },
		stopSubCharacters = { ")", "}", "]" };
	stack<unsigned> sexprStack;
	vector<pair<unsigned, unsigned>> sexprlist;

	// insert a semicolon at the front and back to ensure correct parsing
	fragments.insert(fragments.begin(), TokenFragment(";", true));
	fragments.push_back(TokenFragment(";", true));

	squashStrings(fragments);


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
}
ExpressionTree *ExpressionTree::parse(string statement) {
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
	for(unsigned i = 0; i < exprEnds.size(); ++i)
		last = treeify(exprEnds[i].first, exprEnds[i].second);

	// find the real starts and ends of the overall expression
	ExpressionTree *start = last, *end = NULL;
	for(; start->prev; start = start->prev)
		;// "spin"
	for(end = start; end->next; end = end->next)
		;// "spin"

	// last ditch attempt for leftover semicolons
	start = dropSemicolons(start, end);

	return start;
}

// TODO: remove after debug?
void ExpressionTree::print(int level, bool sprint) {
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
}

bool ExpressionTree::validAssignmentOperand() {
	if(!this->isSpecial("$"))
		return false;
	if(this->rchild && this->rchild->validOperand())
		return true;
	return false;
}
bool ExpressionTree::validOperand() {
	if(this->fragment.special && !this->folded)
		return false;
	return true;
}
bool ExpressionTree::validUrnaryOperand() {
	return false;
}

bool ExpressionTree::validIdentifier() {
	if(this->fragment.special)
		return false;
	return this->fragment.validIdentifier();
}

bool ExpressionTree::isSpecial(string token) {
	if(!this->fragment.special)
		return false;
	return (this->fragment.text == token);
}

// TODO: failure of: $t = (thing1; thing2) and
// TODO: func => (thing1; thing2)

// TODO: various debug things in here
// TODO: ternary operators incorrectly get joined into function call
// TODO: arguments. this probably means we need another stack option in fragment
ExpressionTree *ExpressionTree::treeify(ExpressionTree *begin, ExpressionTree *end) {
	static vector<string> assignments = {
			"=", "+=", "-=", "*=", "/=", "%=", "^=", "~=",
			"++", "--"
	};
	static vector<string> fassignments = { 
			"=>", "+=>"
	};
	static vector<vector<pair<string, unsigned>>> precedenceMap = {
		{ { "$", Prefix } },
		{ { "++", Suffix }, { "--", Suffix } },
		{
			{ "!", Prefix },
			{ "++", Prefix }, { "--", Prefix },
			{ "+", Prefix }, { "-", Prefix }
		},
		{ { "^", Binary } },
		{ { "*", Binary }, { "/", Binary }, { "%", Binary } },
		{ { "+", Binary }, { "-", Binary } },
		{ { "~", Binary } },
		{
			{ "<", Binary }, { "<=", Binary },
			{ ">", Binary }, { ">=", Binary }
		},
		{ { "==", Binary }, { "!=", Binary }, { "=~", Binary } },
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
	};

	// TODO: if previous before parenthesis is a function call, use this as
	// arguments? If no args currently?
	// if we've got a parenthized or braced subexpression, simply ditch the
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
	}

	// loop over precedence levels
	for(auto level : precedenceMap) {
		// loop over the expression
		for(ExpressionTree *here = begin->next; here != end; here = here->next) {
			if(!here)
				throw (string)"ExpressionTree::treeify got NULL here? (never hit end)";

			// if it's not a special token, skip it
			if(!here->fragment.special)
				continue;
			// if it's already folded, then skip it
			if(here->folded)
				continue;

			// if we have a function call, parse and bind arguments
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
			}

			// loop over all operators on this level
			for(auto op : level) {
				// if the op doesnt' match, skip it
				if(here->fragment.text != op.first)
					continue;
				// make sure it's the proper type
				switch(op.second) {
					case OperatorType::Binary:
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

						break;
					case OperatorType::Prefix:
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

						break;
					case OperatorType::Suffix:
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

						break;
					default:
						// TODO: include useful information here?
						throw (string)"unkown operator type";
				}
			}
		}
	}

	return dropSemicolons(begin, end);
}

// TODO: some error stuff in here too
ExpressionTree *ExpressionTree::dropSemicolons(
		ExpressionTree *begin, ExpressionTree *end) {
	bool done = false;
	for(ExpressionTree *here = begin; !done;) {
		if(here == end)
			done = true;
		if(here == NULL) {
			if(here == end)
				break;
			if(here == begin)
				throw (string)"ExpressionTree::dropSemicolons: begin is NULL";
			throw (string)"ExpressionTree::dropSemicolons: here is NULL";
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
}

// TODO: put this somewhere usefull
static string escape(string str);
string escape(string str) {
	string res;
	for(char c : str)
		if(c == '\\')
			res += "\\\\";
		else
			res += c;
	return res;
}

// TODO: this is far from done
// TODO: we could have a test suite for this... parse -> toString -> parse,
// TODO: compare? We modify it somewhat, but how badly do we mangle it?
// TODO: this is totally screwed up :( Needs to be parenthisized
string ExpressionTree::toString(bool all) {
	if(this->next && all) {
		vector<string> parts;
		for(ExpressionTree *expr = this; expr; expr = expr->next)
			parts.push_back(expr->toString(false));
		return join(parts, "; ");
	}
	if(!this->fragment.special) {
		// TODO: use type tags to not do this?
		if(this->fragment.isString)
			return this->fragment.sdelim +
				((this->fragment.sdelim == "'") ?
				 this->fragment.text : escape(this->fragment.text)) +
				this->fragment.sdelim;
		return this->fragment.text;
	}

	if(this->isSpecial("()"))
		return "(" + this->child->toString() + ")";

	if(this->isSpecial("$"))
		return "$" + this->rchild->fragment.text;

	if(this->isSpecial("!")) {
		string ret = "!" + this->child->fragment.text;
		for(ExpressionTree *arg = this->rchild; arg; arg = arg->next)
			ret += " " + arg->toString(false);
		return ret;
	}
	string here;
	if(this->child)
		here += this->child->toString() + " ";
	here += this->fragment.text;
	if(this->rchild)
		here += " " + this->rchild->toString();
	return here;
}

// TODO: ability to tag ExpressionTree as various types. string, int,
// TODO: double, variable, function?

// TODO: on throw, rollback changes. This will be a lot of work...

// TODO: better timing control
// TODO: max recursion depth? Just run in thread and abort after x time?
Variable ExpressionTree::evaluate(string nick, bool all) {
	if(this->next && all) {
		vector<Variable> results;
		for(ExpressionTree *expr = this; expr; expr = expr->next)
			results.push_back(expr->evaluate(nick, false));
		return results.back();
	}

	// might have side effects
	// TODO: permissions on creation with = and => (mostly execute and owner)
	if(this->fragment.isSpecial("()")) {
		// TODO
		if(this->child == NULL)
			return Variable("", Permissions(nick));
		return this->child->evaluate(nick);
	}

	if(this->fragment.isSpecial("?")) {
		ExpressionTree *trueTree = this->rchild, *falseTree = NULL;
		if(this->rchild->isSpecial(":")) {
			trueTree = this->rchild->child;
			falseTree = this->rchild->rchild;
		}
		Variable condition = this->child->evaluate(nick);
		ExpressionTree *target = trueTree;
		if(condition.isFalse())
			target = falseTree;
		if(target == NULL) {
			return Variable("", Permissions());
		}

		return target->evaluate(nick);
	}
	if(this->fragment.isSpecial("=")) {
		// left child is $, with right child varname
		string var = this->child->rchild->fragment.text;
		Variable reval = this->rchild->evaluate(nick);
		if(var == (string)"null")
			return reval;
		if(global::vars.find(var) == global::vars.end()) {
			reval.permissions = Permissions(nick);
			global::vars[var] = reval;
			return reval;
		}

		ensurePermission(Permission::Write, nick, var);

		global::vars[var] = reval;
		return reval;
	}
	if(this->fragment.isSpecial("=>") || this->fragment.isSpecial("+=>")) {
		string func = this->child->fragment.text;
		string rtext = this->rchild->toString();
		if(func == (string)"null")
			return Variable(rtext, Permissions(nick));

		if(global::vars.find(func) == global::vars.end()) {
			global::vars[func] = Variable(rtext, Permissions(nick));
			return Variable(rtext, Permissions(nick));
		}

		ensurePermission(Permission::Write, nick, func);

		if(this->fragment.text == "+=>")
			global::vars[func] += "; " + rtext;
		else
			global::vars[func] = rtext;
		return Variable(global::vars[func].toString(), Permissions(nick));
	}
	if(this->fragment.isSpecial("!")) {
		string func = this->child->fragment.text,
				funcBody = global::vars[func].toString();
		// TODO: check for function existence
		ensurePermission(Permission::Execute, nick, func);

		vector<ExpressionTree *> argTrees;
		for(ExpressionTree *arg = this->rchild; arg; arg = arg->next)
			argTrees.push_back(arg);

		// figure out the result of the arguments
		vector<Variable> args;
		for(unsigned i = 0; i < argTrees.size(); ++i) {
			Variable arg = argTrees[i]->evaluate(nick, false);
			args.push_back(arg);
		}

		string argsstr;
		for(auto arg : args)
			argsstr += " " + arg.toString();

		// clear out argument variables
		for(int i = 0; i < 10; ++i)
			global::vars[asString(i + 1)] = "";

		// set the argument values
		global::vars["args"] = argsstr;
		global::vars["0"] = Variable(func, Permissions(nick));
		for(unsigned i = 0; i < args.size(); ++i)
			global::vars[asString(i + 1)] = args[i];

		// a module function
		if(contains(modules::hfmap, func)) {
			modules::Function mfunc = modules::hfmap[func];
			return mfunc(args);
		}

		// user defined function
		if(global::vars.find(func) == global::vars.end())
			throw func + " is undefined";

		ExpressionTree *etree = NULL;
		Variable res;
		try {
			etree = ExpressionTree::parse(funcBody);
			res = etree->evaluate(nick);
		} catch(string &s) {
			delete etree;
			throw s;
		}
		delete etree;

		return res;
	}

	if(this->fragment.isSpecial("++") || this->fragment.isSpecial("--")) {
		string vname = "";
		bool pre = false;
		// TODO: hard coded paths? How about a resolve var name function?
		if(this->child)
			pre = false, vname = this->child->rchild->fragment.text;
		else
			pre = true, vname = this->rchild->rchild->fragment.text;

		// creating it
		if(global::vars.find(vname) == global::vars.end()) {
			if(this->isSpecial("++"))
				global::vars[vname] = Variable("1", Permissions(nick));
			else
				global::vars[vname] = Variable("-1", Permissions(nick));
			if(pre) {
				if(this->isSpecial("++"))
					return Variable("1", Permissions());
				else
					return Variable("-1", Permissions());
			} else
				return Variable("0", Permissions());
		}

		ensurePermission(Permission::Write, nick, vname);

		Variable var = global::vars[vname];
		if((var.type != Type::Double) && (var.type != Type::Integer))
			var = var.asInteger();
		Variable ival = var;
		if(this->isSpecial("++"))
			var.value.l++;
		else
			var.value.l--;
		global::vars[vname] = var;
		if(pre)
			return var;
		else
			return ival;
	}

	// TODO: regex is reinterpreting the argument escapes?
	// TODO: $text =~ ':o/|\o:' behaves like ':o/|o:'
	if(this->fragment.isSpecial("=~") || this->fragment.isSpecial("~")) {
		string text = this->child->evaluate(nick).toString(),
				rstring = this->rchild->evaluate(nick).toString();

		size_t rend = 0;
		char sep = rstring.front();
		rstring = rstring.substr(1);

		if(rstring.front() == sep)
			throw (string)"empty regex not supported";
		for(rend = 1; rend < rstring.length(); ++rend) {
			if(rstring[rend] == '\\')
				rend++;
			else if(rstring[rend] == sep)
				break;
		}
		if(rend == rstring.length())
			throw (string)"unterminated regex";
		string replacement = rstring.substr(rend + 1);
		rstring = rstring.substr(0, rend);

		string flags;
		// if we have a replacement, we may have flags
		if(replacement.length() > 0) {
			if(replacement.front() == sep) {
				flags = replacement.substr(1);
				replacement = "";
			} else {
				for(rend = 1; rend < replacement.length(); ++rend) {
					if(replacement[rend] == '\\')
						rend++;
					else if(replacement[rend] == sep)
						break;
				}
				if(rend != replacement.length()) {
					flags = replacement.substr(rend + 1);
					replacement = replacement.substr(0, rend);
				}
			}
		}

		// TODO: parse flags
		// TODO: group variables, r0, r1, etc

		// note: may throw
		boost::regex rregex(rstring, regex::perl);
		string str = regex_replace(text, rregex, replacement,
				boost::match_default | boost::format_all);
		Variable r_ = Variable(str, Permissions(Permission::Read));
		if(str != text) {
			global::vars["r_"] = r_;
			if(this->fragment.isSpecial("~"))
				return r_;
			return Variable(true, Permissions());
		}

		if(this->fragment.isSpecial("~"))
			return r_;
		return Variable(false, Permissions());
	}

	if(!this->fragment.special) {
		if(this->fragment.isString)
			return Variable(this->fragment.text, Permissions());
		return Variable::parse(this->fragment.text);
	}
	if(this->isSpecial("$")) {
		string var = this->rchild->fragment.text;
		if(global::vars.find(var) == global::vars.end()) {
			global::vars[var] = Variable(0L, Permissions(nick));
		}
		ensurePermission(Permission::Read, nick, var);
		return global::vars[var];
	}
	// TODO: de-int this. double? Only when "appropriate"?
	/*
	if(this->fragment.isSpecial("^")) {
		return asString(pow(fromString<long>(this->child->evaluate(nick)),
				fromString<long>(this->rchild->evaluate(nick))));
	}
	*/

	if(this->isSpecial("+"))
		return this->child->evaluate(nick) + this->rchild->evaluate(nick);
	if(this->isSpecial("-"))
		return this->child->evaluate(nick) - this->rchild->evaluate(nick);
	if(this->isSpecial("*"))
		return this->child->evaluate(nick) * this->rchild->evaluate(nick);
	if(this->isSpecial("/"))
		return this->child->evaluate(nick) / this->rchild->evaluate(nick);
	if(this->isSpecial("%"))
		return this->child->evaluate(nick) % this->rchild->evaluate(nick);

	vector<string> comparisons = { "==", "!=", "<=", ">=", "<", ">" };
	for(auto c : comparisons)
		if(this->fragment.isSpecial(c))
			return this->child->evaluate(nick).compare(this->rchild->evaluate(nick), c);

	// TODO: un-double this? Also, unstring for == and ~=
	if(this->fragment.isSpecial("&&"))
		return this->child->evaluate(nick) & this->rchild->evaluate(nick);
	if(this->fragment.isSpecial("||"))
		return this->child->evaluate(nick) | this->rchild->evaluate(nick);

	vector<string> compoundOpAssigns = { "+", "-", "*", "/", "%", "^", "~" };
	for(auto op : compoundOpAssigns) {
		if(this->fragment.isSpecial(op + "=")) {
			ExpressionTree opET({ op, true }, 0);
			opET.child = this->child;
			opET.rchild = this->rchild;
			Variable res;
			try {
				res = opET.evaluate(nick);
			} catch(string &s) {
				opET.child = opET.rchild = NULL;
				throw s;
			}
			opET.child = opET.rchild = NULL;

			ExpressionTree assign({ "=", true }, 0), rhs({ res.toString() }, 0);
			assign.child = this->child;
			assign.rchild = &rhs;
			try {
				assign.evaluate(nick);
			} catch(string &s) {
				assign.child = assign.rchild = NULL;
				throw s;
			}
			assign.child = assign.rchild = NULL;
			return this->child->evaluate(nick);
		}
	}

	throw (string)"unkown node { \"" + this->fragment.text + "\", " +
		(this->fragment.special ? "" : "not") + " special }, bug " +
		global::vars["bot.owner"].toString() + " to fix";
}

