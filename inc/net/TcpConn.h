#ifndef TCP_CONN_H
#define TCP_CONN_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <Message.h>
#include <deque>

#define RECV_MAX_SIZE 1024

enum connection_type
{
  TCP_SERVER = 0,
  TCP_CLIENT
};

struct socket_info
{
  int fd;
  struct sockaddr_in addr;
};

class TcpConn
{
public:
//  TcpConn(const std::string addr, const int port, std::deque<Message> &msgQueue);
  TcpConn(uint32_t id, const socket_info &sockInfo, std::deque<Message> &msgQueue,
          int epollfd = -1);
  ~TcpConn();

  void processEvent(struct epoll_event &ev);
  void readRemainingMessages();
  void sendRemainingMessages();

  void sendMsg(const std::string &msg);

  void drop();
  bool isActive() const;

private:
  uint32_t m_id;
  bool m_active;

  socket_info m_socket;
  int m_epollfd;
  std::vector<uint8_t> m_buffer;
  std::deque<Message> &m_msgQueue;
  std::deque<std::string> m_outMsgQueue;

private:
  bool createMessageFromBuffer();
  void createMessagesFromBuffer();

  void enableEpolloutTracking(bool value);
};

void setNonBlocking(int socketfd);

#endif