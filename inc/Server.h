#ifndef SERVER_H
#define SERVER_H

#include <deque>
#include <iostream>
#include <map>
#include <vector>

#include <Channel.h>

#define DEFAULT_PORT 12834
#define MAX_CONNECTIONS (-1)
#define EPOLL_MAX_EVENTS 10
#define EPOLL_TIMEOUT (-1)
#define DOMAIN_FAMILY AF_INET

typedef std::map<std::string, Channel *>::iterator channel_iterator_t;
typedef std::map<uint32_t, Client *>::iterator client_iterator_t;

struct fd_monitor
{
  int epollfd;
  struct epoll_event events[EPOLL_MAX_EVENTS];
  uint32_t curr_id;
};

class Server
{
public:
  Server();
  ~Server();

  void start();
  void stop();
  Channel *getChannelByName(const std::string &name);
  Client *getClientByNick(const std::string &nick);
  Channel *createChannel(const std::string &channel_name, const std::string &channel_key);
  void deleteChannel(Channel *to_remove);
  void pruneClient(Client &client, bool destroy);
  void letHimCook(Client &client);
  bool initializeServer(int port, const std::string &password);
  bool isPasswordSet();
  void destroyClient(Client *client);
  bool verifyClient(Client &client);

private:
  void init(int port = -2);
  void initMonitoring();

  void monitorConnections();
  void acceptConnection();

  void processMessage(Message &msg);

private:
  bool m_initialized;
  bool m_quit;
  bool m_bot_connected;
  socket_info m_socket;
  fd_monitor m_monitor;

  // Server Data Struct
  std::deque<Message> m_msgQueue;
  std::map<uint32_t, Client *> m_clientMap;
  std::map<std::string, Channel *> m_channelMap;
  std::string m_password;
};

#endif
