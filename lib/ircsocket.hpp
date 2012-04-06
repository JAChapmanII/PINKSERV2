#ifndef IRC_SOCKET_HPP
#define IRC_SOCKET_HPP

#include <string>
#include <vector>
#include <map>
#include <SFML/Network/TcpSocket.hpp>

class IRCSocket {
	public:
		IRCSocket(std::string host, unsigned short port, std::string nick);
		~IRCSocket();

		int connect();
		int join(std::string channel);
		int quit();

		ssize_t write(const char *data, size_t count);
		ssize_t write(std::string str);
		ssize_t send(std::string str, std::string target);
		std::string read();

	protected:
		std::string fetch();

		std::string m_host;
		unsigned short m_port;
		std::string m_nick;

		bool m_isConnected;

		std::map<std::string, std::vector<std::string>> m_channelNicks;

		sf::TcpSocket m_socket;
		std::string m_recvBuffer;
};

#endif // IRC_SOCKET_HPP
