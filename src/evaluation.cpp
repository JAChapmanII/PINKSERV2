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
			"++", "--", "<=", ">=", "==", "=~", "+=", "-=",
			"<", ">", "(", ")", "?", ":", ";",
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
};

struct ExpressionTree {
	TokenFragment fragment;
	ExpressionTree *child;
	ExpressionTree *prev, *next;

	ExpressionTree(TokenFragment ifrag) : fragment(ifrag), child(NULL),
			prev(NULL), next(NULL) {
	}

	static ExpressionTree *parse(string statement) {
		vector<TokenFragment> frags = TokenFragment::fragment(statement);

		// starts of sub expressions
		stack<unsigned> sexprStack;
		stack<ExpressionTree *> exprStack;
		ExpressionTree *current = NULL;

		// TODO: parse into a tree

		// make a pass to ensure there aren't any strange mismatched sub
		// expression related tokens
		for(unsigned i = 0; i < frags.size(); ++i) {
			if(frags[i].special) {
				// open sub expressions
				if(frags[i].text == "{")
					sexprStack.push(i);
				if(frags[i].text == "(")
					sexprStack.push(i);

				// close expressions
				if(frags[i].text == "}") {
					if(sexprStack.empty())
						throw (string)"extra " + frags[i].text;
					if(frags[sexprStack.top()].text == "{")
						sexprStack.pop();
					else
						throw (string)"mismatched " + frags[sexprStack.top()].text +
							" with " + frags[i].text;
				}
				if(frags[i].text == ")") {
					if(sexprStack.empty())
						throw (string)"extra " + frags[i].text;
					if(frags[sexprStack.top()].text == "(")
						sexprStack.pop();
					else
						throw (string)"mismatched " + frags[sexprStack.top()].text +
							" with " + frags[i].text;
				}
			}
		}
		if(!sexprStack.empty()) {
			throw asString(sexprStack.size()) + " expressions unclosed";
		}

		return NULL;
	}
	static ExpressionTree parse() {
	}
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
