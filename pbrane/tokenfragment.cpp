#include "tokenfragment.hpp"
using std::string;
using std::vector;

#include "util.hpp"
using util::startsWith;

vector<TokenFragment> TokenFragment::fragment(string statement) {
	vector<string> special = {
		"+=>", "=>", "&&", "||",
		"++", "--", "<=", ">=", "==", "!=", "=~",
		"+=", "-=", "*=", "/=", "%=", "^=", "~=",
		"=", "<", ">", "(", ")", "?", ":", ";", "[", "]",
		"+", "-", "*", "/", "%", "^", "{", "}", "$", "!", "~"
	};

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
			// TODO: Allow expressions inside of strings?

			// if we have an escaped character
			if(stringType == '"' && statement.front() == '\\') {
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
}
bool TokenFragment::validIdentifier(string str) {
	char f = str.front();
	if(str.length() == 1 && isdigit(f))
		return true;
	if(!isalpha(f) && f != '_')
		return false;
	for(unsigned i = 1; i < str.length(); ++i)
		// TODO: this needs to be standardized better
		if(!isalnum(str[i]) && (str[i] != '.') && (str[i] != '_'))
			return false;
	return true;
}

TokenFragment::TokenFragment() : special(false), text() {
}
TokenFragment::TokenFragment(string itext, bool ispecial) :
		special(ispecial), text(itext) {
}


void TokenFragment::clear() {
	text.clear();
	special = false;
}

bool TokenFragment::isSpecial(string token) {
	if(!this->special)
		return false;
	return (text == token);
}

bool TokenFragment::validIdentifier() {
	return validIdentifier(this->text);
}

