#include <iostream>
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include "ircsocket.hpp"

#include <SFML/System/Time.hpp>
using sf::seconds;
#include <SFML/System/Sleep.hpp>
using sf::sleep;

int main(int argc, char **argv) {
	string server = "irc.slashnet.org";
	if(argc > 1)
		server = argv[1];

	IRCSocket isock(server, 6667, "pbrane");

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
	sleep(seconds(1));
	cerr << ".";
	sleep(seconds(1));
	cerr << ".";
	sleep(seconds(1));
	cerr << "." << endl;

	cerr << "quitting" << endl;
	isock.quit();
	return 0;
}
