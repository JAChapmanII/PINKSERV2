#ifndef TOKENFRAGMENT_HPP
#define TOKENFRAGMENT_HPP

#include <string>
#include <vector>

struct TokenFragment {
	bool special;
	std::string text;

	static std::vector<TokenFragment> fragment(std::string statement);
	static bool validIdentifier(std::string str);

	TokenFragment();
	TokenFragment(std::string itext, bool ispecial = false);

	void clear();
	bool isSpecial(std::string token);
	bool validIdentifier();
};

#endif // TOKENFRAGMENT_HPP
