#include <iostream>
using std::cerr;
using std::endl;

#include "ircsocket.hpp"

#include <SFML/System/Sleep.hpp>
using sf::Sleep;

int main(int argc, char **argv) {
	IRCSocket isock("irc.slashnet.org", 6667, "pbrane");

	cerr << "connecting" << endl;
	if(isock.connect() < 0) {
		cerr << "error on connect" << endl;
		return 1;
	}
	cerr << "connected" << endl;

	if(isock.join("#zebra") < 0) {
		cerr << "error on join" << endl;
		return 2;
	}
	cerr << "joined" << endl;

	cerr << "Sending \"Hello!\"" << endl;
	isock.send("Hello!", "#zebra");
	Sleep(1000);
	cerr << ".";
	Sleep(1000);
	cerr << ".";
	Sleep(1000);
	cerr << "." << endl;

	cerr << "quitting" << endl;
	isock.quit();
	return 0;
}
