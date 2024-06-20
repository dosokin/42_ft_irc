//
// Created by dosokin on 6/17/24.
//

#ifndef BOT_IRC_BOT_HPP
#define BOT_IRC_BOT_HPP

#include <arpa/inet.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define USER_CMD "USER BOT * 0 : boot\r\n"
#define NICK_CMD(nick) "NICK bot" + nick + "\r\n"
#define PRIVMSG_CMD(recipient, msg) "PRIVMSG " + recipient + " :" + msg + "\r\n"

class Koakoubot
{

public:
  Koakoubot();
  ~Koakoubot();

  int getServerSockfd() const;
  const addrinfo &getServerAddrInfo() const;
  std::deque<std::string> &getMsgQueue();

  void sendMessageToServ(const std::string &message);
  void receiveMessageFromServ();
  void initKoakoubot(const std::string &addr, const std::string &port);
  bool getExit();
  void setExit(bool new_status);

  void check_errno(ssize_t ret);
  void destroy();

private:
  void setNickname();
  void servConnect();
  addrinfo *setAddrInfo(const char *addr, const char *port);
  bool createMessageFromBuffer();

private:
  int m_server_sockfd;
  addrinfo *m_server_addrData;
  std::string m_nickname;
  std::vector<char> m_buffer;
  std::deque<std::string> m_msgQueue;
  bool m_exit;
};

#endif
