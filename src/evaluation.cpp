#include <map>
using std::map;
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <algorithm>
using std::transform;
#include <stack>
using std::stack;
#include <cctype>

#include "util.hpp"
using util::split;
using util::trimWhitespace;
using util::contains;
using util::toOrdinal;
using util::startsWith;
using util::asString;

// variable, function map
map<string, string> vars;

// local variable map
map<string, map<string, string>> lvars;


vector<string> getList(string variable);
vector<string> makeList(string lists);

vector<string> makeList(string lists) { // {{{
	vector<string> list = split(lists, ",");
	transform(list.begin(), list.end(), list.begin(), trimWhitespace);
	return list;
} // }}}
vector<string> getList(string variable) { // {{{
	string lists = vars[variable];
	return makeList(lists);
} // }}}


enum Permission { Execute = 0x1, Append = 0x2, Write = 0x4, Modify = 0x8 };
static uint8_t xPermissions = Permission::Execute;
static uint8_t xawPermissions = xPermissions |
	Permission::Append | Permission::Write;
static uint8_t xawmPermissions = xawPermissions | Permission::Modify;

enum PermissionType { Admin, Owner, User, SUser };
struct PermissionFragment {
	PermissionType type;
	string nick;
	uint8_t perms;

	static PermissionFragment parse(string pstr) { // {{{
		PermissionFragment pf = { PermissionType::User, "", 0x0 };

		// split this part on "=",
		vector<string> frags = split(pstr, "=");
		// there need to be two terms
		if(frags.size() != 2)
			throw pstr + " is malformed";
		// trim extra whitespace from all tokens
		transform(frags.begin(), frags.end(), frags.begin(), trimWhitespace);

		// make sure there aren't wonky letters in the perms part
		static string validPermissionCharacters = "xawm0";
		for(auto c : frags[1])
			if(!contains(validPermissionCharacters, (c)))
				throw string(1, c) + " is not a valid permission";

		// determine the actual permissions value
		if(contains(frags[1], 'x'))
			pf.perms |= Permission::Execute;
		if(contains(frags[1], 'a'))
			pf.perms |= Permission::Append;
		if(contains(frags[1], 'w'))
			pf.perms |= Permission::Write;
		if(contains(frags[1], 'm'))
			pf.perms |= Permission::Modify;

		// apply it to the proper thing
		if(frags[0].length() == 1) {
			switch(frags[0][0]) {
				case 'a': pf.type = PermissionType::Admin; break;
				case 'o': pf.type = PermissionType::Owner; break;
				case 'u': pf.type = PermissionType::User; break;
				default:
					throw string(1, frags[0][0]) + " is not valid identifier";
			}
		} else {
			if(!startsWith(frags[0], "n:"))
				throw frags[0] + " is a bad identifier";
			string nick = frags[0].substr(2);
			pf.type = PermissionType::SUser;
			pf.nick = nick;
		}

		// return the built fragment
		return pf;
	} // }}}
};

struct Permissions {
	uint8_t admin;
	string owner_nick;
	uint8_t owner;
	map<string, uint8_t> suser;
	uint8_t user;

	static Permissions parse(string perms) { // {{{
		Permissions p = {
			xawmPermissions, "", xawmPermissions, { }, xPermissions
		};

		// split perms into parts by ",", trim extra whitespace
		vector<string> parts = makeList(perms);
		uint8_t maxUserp = 0x0;
		for(unsigned i = 0; i < parts.size(); ++i) {
			string fragOrdinal = toOrdinal(i + 1) + " fragment";
			PermissionFragment pf = PermissionFragment::parse(parts[i]);
			p.apply(pf);
			if(pf.type == PermissionType::SUser)
				maxUserp |= pf.perms;
		}

		// check for heirarchy conflicts
		if((p.admin | p.owner) != p.admin)
			throw (string)"owner cannot have more powers than admins";
		if((p.owner | p.user) != p.owner)
			throw (string)"users cannot have more powers than the owner";
		if((p.owner | maxUserp) != p.owner)
			throw (string)"no specific user can have more powers than the owner";
		if((p.admin | maxUserp) != p.admin)
			throw (string)"no specific user can have more powers than an admin";

		// return the built permissions object
		return p;
	} // }}}

	void apply(PermissionFragment pfrag) { // {{{
		switch(pfrag.type) {
			case PermissionType::Admin: this->admin = pfrag.perms; break;
			case PermissionType::Owner: this->owner = pfrag.perms; break;
			case PermissionType::User: this->user = pfrag.perms; break;
			case PermissionType::SUser:
				this->suser[pfrag.nick] = pfrag.perms;
				break;
			default:
				throw (string)"invalid permission type";
		}
	} // }}}
	bool allowed(Permission p, string nick, int level = 5) { // {{{
		uint8_t fperms = 0x0;
		int mlevel = 0;

		// either the user is specified directly, or they're just a user
		if(contains(this->suser, nick)) {
			fperms |= this->suser[nick];
		} else {
			fperms |= this->user;
			mlevel = 1;
		}

		// if the user is the owner, add in their permisssions
		if(this->owner_nick == nick) {
			fperms |= this->owner;
			mlevel = 2;
		}

		// if the user is an admin, add in admin permissions
		if(contains(getList("bot.admins"), nick)) {
			fperms |= this->admin;
			mlevel = 3;
		}

		// if it's not a permission issue, don't worry about level
		if(p != Permission::Modify)
			return p & fperms;

		// normal users are not allowed to have modify permissions
		if(level < 2)
			return false;
		// make sure the user has permissions and is at a higher level
		return (p & fperms) && (mlevel > level);
	} // }}}
};

bool hasPermission(Permission p, string nick, string variable, int level = 5);

// variable permission map
map<string, Permissions> vars_perms;

bool hasPermission(Permission p, string nick, string variable, int level) { // {{{
	// the bot owner can do whatever they want
	if(vars["bot.owner"] == nick)
		return true;

	Permissions perms = vars_perms[variable];
	return perms.allowed(p, nick, level);
} // }}}

#include <iostream>
using namespace std;

enum TokenType { Call, SubToken, Argument };
struct Token {
	TokenType type;
	string text;
	Token *sub;
	Token *next;

	Token() : type(TokenType::Argument), text(""), sub(NULL), next(NULL) {
	}
	void print(int ilevel = 0) {
		for(Token *token = this; token; token = token->next) {
			if(token->type == TokenType::SubToken) {
				token->sub->print(ilevel + 1);
			} else {
				cout << string(ilevel, '\t');
				switch(token->type) {
					case TokenType::Call:
						cout << "!" << token->text;
						break;
					case TokenType::Argument:
						cout << token->text;
						break;
				}
			}
			cout << endl;
		}
	}
};

struct TokenFragment {
	bool special;
	string text;

	TokenFragment() : special(false), text() { }
	TokenFragment(string itext, bool ispecial = false) :
		special(ispecial), text(itext) { }
	void clear() { // {{{
		text.clear();
		special = false;
	} // }}}
	static vector<TokenFragment> fragment(string statement) { // {{{
		vector<string> special = {
			"+=>", "=>", "&&", "||",
			"++", "--", "<=", ">=", "==", "=~", "~=",
			"+=", "-=", "*=", "/=", "%=", "^=",
			"=", "<", ">", "(", ")", "?", ":", ";", "[", "]",
			"+", "-", "*", "/", "%", "^", "{", "}", "$", "!", "~"
		}, specialInString = { "$", "{", "}" };

		vector<TokenFragment> stokens;
		bool inString = false;
		char stringType = '\0';
		TokenFragment ctoken;
		while(statement.length() > 0) {
			// if the character is a string edge
			if(statement.front() == '\'' || statement.front() == '"') {
				if(inString) {
					// if we're in a string and it's the end
					if(statement.front() == stringType) {
						// if there's string content, add it
						if(!ctoken.text.empty())
							stokens.push_back(ctoken);
						ctoken.clear();
						// add the special string terminator
						stokens.push_back({ string(1, statement.front()), true });
						statement = statement.substr(1);
						inString = false;
						continue;
					}
				// we're starting a string
				} else {
					// set the string type accordingly
					stringType = statement.front();
					// add the special string starter
					stokens.push_back({ string(1, statement.front()), true });
					statement = statement.substr(1);
					inString = true;
					continue;
				}
			}

			if(inString) {
				// check for special in-string characters
				bool isSpecial = false;
				for(auto specialt : specialInString) {
					// if it is a special token
					if(startsWith(statement, specialt)) {
						// if there is a current token, push it first
						if(!ctoken.text.empty())
							stokens.push_back(ctoken);
						ctoken.clear();
						// push our new special token
						stokens.push_back({ specialt, true });
						statement = statement.substr(specialt.length());
						isSpecial = true;
						break;
					}
				}
				// do all previous tests again on special
				if(isSpecial)
					continue;

				// if we have an escaped character
				if(statement.front() == '\\') {
					// skip the delimiter so we get just the real char
					statement = statement.substr(1);
					// if we escaped the end of the string it's an oopsie
					if(statement.empty()) {
						throw (string)"escaped character at end of string";
					}
				}

				// append current character to current token and advance it string
				ctoken.text += statement.front();
				statement = statement.substr(1);
				continue;
			}

			// check for special tokens
			bool isSpecial = false;
			for(auto specialt : special) {
				// if we're on a special token
				if(startsWith(statement, specialt)) {
					// if there's a current token, end it
					if(!ctoken.text.empty())
						stokens.push_back(ctoken);
					ctoken.clear();
					// add the new special token
					stokens.push_back({ specialt, true });
					statement = statement.substr(specialt.length());
					isSpecial = true;
					break;
				}
			}
			// if we found a special token, we do all previous tests again
			if(isSpecial)
				continue;

			// if it's a space, end of current token and just skip the space
			if(isspace(statement[0])) {
				if(!ctoken.text.empty())
					stokens.push_back(ctoken);
				ctoken.clear();
			// otherwise append to current token
			} else {
				// if we have an escaped character
				if(statement.front() == '\\') {
					// skip the escape character so the real character is added
					statement = statement.substr(1);
					// if there's nothing to escape, error
					if(statement.empty()) {
						throw (string)"escaped character at end of string";
					}
				}

				// append the current character to the text
				ctoken.text += statement[0];
			}
			// advannce in the statement string
			statement = statement.substr(1);
		}

		if(!ctoken.text.empty())
			stokens.push_back(ctoken);
		if(inString)
			throw (string)"string encountered with no end";

		return stokens;
	} // }}}
	bool isSpecial(string token) {
		if(!this->special)
			return false;
		return (text == token);
	}
};

void printexprends(pair<unsigned, unsigned> sexpr, vector<TokenFragment> frags) {
	vector<unsigned> fstarts;
	string expr;
	for(auto frag : frags) {
		fstarts.push_back(expr.length());
		expr += frag.text + " ";
	}

	int first = fstarts[sexpr.first], second = fstarts[sexpr.second];
	//cout << first << ", " << second << endl;
	cout << expr << endl;
	cout << string(first, ' ') + '^' +
		string(second - first - 1, '~') + '^' << endl;
}
void printexprends(vector<pair<unsigned, unsigned>> sexpr, vector<TokenFragment> frags) {
	for(auto s : sexpr)
		printexprends(s, frags);
}

struct ExpressionTree {
	bool folded;
	TokenFragment fragment;
	ExpressionTree *child, *rchild;
	ExpressionTree *prev, *next;

	ExpressionTree(TokenFragment ifrag) : folded(false), fragment(ifrag),
			child(NULL), rchild(NULL), prev(NULL), next(NULL) {
	}

	static vector<pair<unsigned, unsigned>> delimitExpressions( // {{{
			vector<TokenFragment> &fragments) {
		static vector<string> startSubCharacters = { "(", "{", "[" },
			stopSubCharacters = { ")", "}", "]" };
		stack<unsigned> sexprStack;
		vector<pair<unsigned, unsigned>> sexprlist;

		// insert a semicolon at the front and back to ensure correct parsing
		fragments.insert(fragments.begin(), TokenFragment(";", true));
		fragments.push_back(TokenFragment(";", true));

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
							!contains(stopSubCharacters, fragments[i + 1].text)) {
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
	static ExpressionTree *parse(string statement) {
		// turn string into a set of fragments
		vector<TokenFragment> frags = TokenFragment::fragment(statement);

		// find ends of expressions
		vector<pair<unsigned, unsigned>> sexprlist = delimitExpressions(frags);
		printexprends(sexprlist, frags);

		// parse into a tree
		vector<ExpressionTree *> exprs;
		for(unsigned i = 0; i < frags.size(); ++i) {
			ExpressionTree *here = new ExpressionTree(frags[i]);
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
			cout << "treeifying" << endl;
			printexprends(sexprlist[i], frags);
			last = treeify(exprEnds[i].first, exprEnds[i].second);
			if(last != NULL) {
				cout << "last returned frome treeify: " << endl;
				last->print();
			}
		}

		ExpressionTree *start = last, *end = NULL;
		for(; start->prev; start = start->prev)
			;// "spin"
		for(end = start; end->next; end = end->next)
			;// "spin"

		//dropSemicolons(start, end);

		cout << endl << endl;
		cout << "final: " << endl;
		start->print();

		return NULL;
	}

	void print(int level = 0) { // {{{
		if(level == 0)
			cout << this->fragment.text << endl;

		if(this->child) {
			cout << string(level * 2 + 2, ' ') << "l: " << this->child->fragment.text << endl;
			this->child->print(level + 1);
		}

		if(this->rchild) {
			cout << string(level * 2 + 2, ' ') << "r: " << this->rchild->fragment.text << endl;
			this->rchild->print(level + 1);
		}

		if(this->next)
			this->next->print(level);
	} // }}}

	bool validAssignmentOperand() { // {{{
		if(!this->isSpecial("$"))
			return false;
		if(this->rchild && this->rchild->validOperand())
			return true;
		return false;
	} // }}}
	bool validOperand() { // {{{
		if(this->fragment.special && !this->folded)
			return false;
		return true;
	} // }}}
	bool validUrnaryOperand() { // {{{
		return false;
	} // }}}

	bool validIdentifier() { // {{{
		if(this->fragment.special)
			return false;
		char f = this->fragment.text.front();
		if(!isalpha(f) && f != '_')
			return false;
		for(unsigned i = 1; i < this->fragment.text.length(); ++i)
			if(!isalnum(this->fragment.text[i]))
				return false;
		return true;
	} // }}}

	bool isSpecial(string token) { // {{{
		if(!this->fragment.special)
			return false;
		return (this->fragment.text == token);
	} // }}}

	enum OperatorType { Binary, Prefix, Suffix };
	static ExpressionTree *treeify(ExpressionTree *begin, ExpressionTree *end) {
		static vector<string> assignments = { // {{{
				"=", "+=", "-=", "*=", "/=", "%=", "^=",
				"++", "--"
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
			{ { "==", Binary }, { "~=", Binary } },
			{ { "&&", Binary } },
			{ { "||", Binary } },
			{
				{ "?", Binary }, { ":", Binary },
				{ "=", Binary },
				{ "+=", Binary }, { "-=", Binary },
				{ "*=", Binary }, { "/=", Binary }, { "%=", Binary },
				{ "^=", Binary },
			}
		}; // }}}

		// if we've got a parenthized subexpression, simply ditch the {{{
		// parenthesis and hook the contents up directly
		if(begin->isSpecial("(") && end->isSpecial(")")) {
			cout << "parenthised expressiond" << endl;
			ExpressionTree *newBegin = begin->prev;
			begin->prev->next = begin->next;
			begin->next->prev = begin->prev;
			delete begin;
			end->prev->next = end->next;
			end->next->prev = end->prev;
			delete end;
			return newBegin->next;
		} // }}}
		// if we've got a braced subexpression, simply ditch the {{{
		// braces and hook the contents up directly
		if(begin->isSpecial("{") && end->isSpecial("}")) {
			cout << "braced expressiond" << endl;
			ExpressionTree *newBegin = begin->prev;
			begin->prev->next = begin->next;
			begin->next->prev = begin->prev;
			delete begin;
			end->prev->next = end->next;
			end->next->prev = end->prev;
			delete end;
			return newBegin->next;
		} // }}}

		// loop over precedence levels
		for(auto level : precedenceMap) { // {{{
			// loop over the expression
			for(ExpressionTree *here = begin->next; here != end; here = here->next) {
				if(!here) {
					cout << "oops" << endl;
					break;
				}
				// if it's not a special token, skip it
				if(!here->fragment.special)
					continue;
				// if it's already folded, then skip it
				if(here->folded)
					continue;
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
								if(!here->prev->validAssignmentOperand())
									throw (string)"assignment with invalid lhs";
							} else {
								if(!here->prev->validOperand()) {
									cout << "prev: " << here->prev->fragment.text << endl;
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

							//cout << "binary" << endl;
							//cout << "  left: " << here->prev->fragment.text << endl;
							//cout << " right: " << here->next->fragment.text << endl;

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

							//here->print();
							break; // }}}
						case OperatorType::Prefix: // {{{
							// TODO: unspecial case?
							if(op.first == "$" || op.first == "!") {
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

							//cout << "prefix" << endl;
							//here->print();
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

							cout << "suffix" << endl;
							//here->print();
							break; // }}}
					}
				}
			}
		} // }}}

		return dropSemicolons(begin, end);
	}

	static ExpressionTree *dropSemicolons( // {{{
			ExpressionTree *begin, ExpressionTree *end) {
		//cout << "REMOVING SEMICOLONS: " << begin->fragment.text << "
		bool done = false;
		for(ExpressionTree *here = begin; !done;) {
			if(here == end)
				done = true;
			if(here == NULL) {
				if(here == end)
					break;
				if(here == begin)
					cout << "begin is NULL? wat" << endl;
				cout << "ERRORRRRR" << endl;
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

			delete tmp;
		}

		return begin;
	} // }}}
};

void execute(string statement) {
	// tmp variable map
	map<string, string> tvars;
}

int main(int argc, char **argv) {
	for(int i = 1; i < argc; ++i) {
		cout << i << ": " << argv[i] << endl;
		try {
			//Permissions p = Permissions::parse(argv[i]);
			//vector<TokenFragment> tfv = TokenFragment::fragment(argv[i]);
			ExpressionTree *etree = ExpressionTree::parse(argv[i]);
		} catch(string &s) {
			cout << "\t: " << s << endl;
		}
	}
	return 0;
}

