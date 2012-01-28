#include "ircsocket.hpp"
using std::string;
using std::vector;
using std::map;
using sf::Socket;

#include <iostream>
using std::cerr;
using std::endl;

#include <SFML/Network/IpAddress.hpp>
#include <SFML/System/Sleep.hpp>
using sf::Sleep;

const unsigned bufferSize = 4096;

IRCSocket::IRCSocket(string host, unsigned short port, string nick) : // {{{
		m_host(host), m_port(port), m_nick(nick), m_isConnected(false),
		m_channelNicks(), m_socket(), m_recvBuffer() {
} // }}}
IRCSocket::~IRCSocket() { // {{{
	if(this->m_isConnected)
		this->quit();
} // }}}

int IRCSocket::connect() { // {{{
	if(this->m_isConnected)
		return -1;

	if(this->m_socket.Connect(this->m_host, this->m_port) == Socket::Error) {
		cerr << "IRCSocket::connect: could not connect to host" << endl;
		return -2;
	}
	this->m_isConnected = true;

	string nickCommand = "NICK " + this->m_nick;
	string userCommand = "USER " + this->m_nick + " j j :" + this->m_nick;

	this->write(nickCommand);
	Sleep(1000/10);

	this->write(userCommand);
	Sleep(1000/10);

	bool done = false;
	while(!done) {
		string str = this->read();
		if(str.empty()) {
			Sleep(1000/100);
		}
		if((str[0] == 'P') && (str[1] == 'I') &&
			(str[2] == 'N') && (str[3] == 'G')) {
			string pong = "PONG";
			if(str.length() > 4)
				pong += str.substr(4);
			this->write(pong);
		} else if(str.find(" 432 ") != string::npos) {
			cerr << "IRCSocket::connect: Nickname contains erroneous characters"
				<< endl;
			return 432;
		} else if(str.find(" 376 ") != string::npos) {
			vector<string> evec;
			return 0;
		}
	}

	return -3;
} // }}}
int IRCSocket::join(string channel) { // {{{
	if(this->m_channelNicks.find(channel) != this->m_channelNicks.end()) {
		return -1;
	}

	string joinCommand = "JOIN " + channel;
	this->write(joinCommand);
	Sleep(1000/10);

	bool done = false;
	while(!done) {
		string str = this->read();
		if(str.empty()) {
			Sleep(1000/100);
			continue;
		}
		if((str[0] == 'P') && (str[1] == 'I') &&
			(str[2] == 'N') && (str[3] == 'G')) {
			string pong = "PONG";
			if(str.length() > 4)
				pong += str.substr(4);
			this->write(pong);
		} else if(str.find(" 433 ") != string::npos) {
			cerr << "IRCSocket::join: nick already taken" << endl;
			return 433;
		} else if((str.find(" 332 ") != string::npos) ||
				(str.find(" JOIN ") != string::npos)) {
			vector<string> evec;
			this->m_channelNicks[channel] = evec;
			return 0;
		}
	}
	return -2;
} // }}}
int IRCSocket::quit() { // {{{
	string quitCommand = "QUIT";
	if(this->write(quitCommand) == (ssize_t)quitCommand.length())
		return 0;
	return 1;
} // }}}

ssize_t IRCSocket::write(const char *data, size_t count) { // {{{
	if(!this->m_isConnected)
		return -1;
	switch(this->m_socket.Send(data, count)) {
		case Socket::Disconnected:
			cerr << "IRCSocket::write: disconnected" << endl;
			this->m_isConnected = false;
			return -2;
		case Socket::Error:
			cerr << "IRCSocket::write: error" << endl;
			this->m_isConnected = false;
			return -3;
		case Socket::Done:
			break;
		case Socket::NotReady: default:
			cerr << "IRCSocket:: impossible response code" << endl;
			this->m_isConnected = false;
			return -4;
	}
	switch(this->m_socket.Send("\r\n", 2)) {
		case Socket::Disconnected:
			cerr << "IRCSocket::write: disconnected (2)" << endl;
			this->m_isConnected = false;
			return -2;
		case Socket::Error:
			cerr << "IRCSocket::write: error (2)" << endl;
			this->m_isConnected = false;
			return -3;
		case Socket::Done:
			break;
		case Socket::NotReady: default:
			cerr << "IRCSocket:: impossible response code (2)" << endl;
			this->m_isConnected = false;
			return -4;
	}
	return count;
} // }}}
ssize_t IRCSocket::write(std::string str) { // {{{
	return this->write((const char *)str.c_str(), (size_t)str.length());
} // }}}
ssize_t IRCSocket::send(string str, string target) { // {{{
	if((str.length() == 0) || (target.length() == 0))
		return 0;
	string pmsgCommand = "PRIVMSG " + target + " :" + str;
	ssize_t res = this->write(pmsgCommand);
	if(res < 1)
		return res;
	return str.length() + target.length();
} // }}}

string IRCSocket::read() { // {{{
	string str = this->fetch();
	if(!str.empty())
		return str;

	char buffer[bufferSize];
	size_t received = 0;
	this->m_socket.Receive(buffer, bufferSize, received);
	if(received > 0)
		this->m_recvBuffer += string(buffer, received);

	str = this->fetch();
	if(!str.empty())
		return str;
	return "";
} // }}}
string IRCSocket::fetch() { // {{{
	size_t rnLocation = this->m_recvBuffer.find("\r\n");
	if(rnLocation != string::npos) {
		string first = this->m_recvBuffer.substr(0, rnLocation),
				second = this->m_recvBuffer.substr(rnLocation + 2);
		this->m_recvBuffer = second;
		return first;
	} else
		return "";
} // }}}

