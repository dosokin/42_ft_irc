#include "Logger.h"
#include "commands/InviteCommand.h"
#include "commands/JoinCommand.h"
#include "commands/KickCommand.h"
#include "commands/ModeCommand.h"
#include "commands/NickCommand.h"
#include "commands/PassCommand.h"
#include "commands/PrivMsgCommand.h"
#include "commands/TopicCommand.h"
#include "commands/UserCommand.h"
#include <Channel.h>
#include <Server.h>
#include <cstdio>

Server::Server() : m_initialized(), m_quit(), m_bot_connected() {}

bool Server::initializeServer(int port, const std::string &password)
{
  if (m_initialized)
    return false;
  m_initialized = true;
  m_password = password;
  init(port);
  if (m_quit)
    return false;
  return true;
}

Server::~Server()
{
  for (client_iterator_t it = m_clientMap.begin(); it != m_clientMap.end(); ++it)
    delete it->second;
  for (channel_iterator_t it = m_channelMap.begin(); it != m_channelMap.end(); ++it)
    delete it->second;
  if (m_monitor.epollfd != -1)
    close(m_monitor.epollfd);
  if (m_socket.fd != -1)
  {
    shutdown(m_socket.fd, 0);
    close(m_socket.fd);
  }
}

void Server::init(int port)
{
  m_quit = false;
  m_monitor.epollfd = -1;
  if ((m_socket.fd = socket(DOMAIN_FAMILY, SOCK_STREAM, 0)) < 0)
  {
    IRC_LOG_FATAL("failed to create server socket");
    m_quit = true;
    return;
  }

  int val = 1;
  if (setsockopt(m_socket.fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1)
  {
    IRC_LOG_FATAL("failed to set sock option");
    m_quit = true;
    return;
  }

  m_socket.addr.sin_family = DOMAIN_FAMILY;
  m_socket.addr.sin_addr.s_addr = INADDR_ANY;
  m_socket.addr.sin_port = htons(port == -2 ? DEFAULT_PORT : port);
  if (bind(m_socket.fd, reinterpret_cast<sockaddr *>(&m_socket.addr), sizeof(m_socket.addr)) != 0)
  {
    IRC_LOG_FATAL("failed to bind server socket");
    m_quit = true;
    return;
  }
  initMonitoring();
}

void Server::initMonitoring()
{
  m_monitor.epollfd = epoll_create1(0);
  if (m_monitor.epollfd == -1)
  {
    IRC_LOG_FATAL("epoll_create");
    m_quit = true;
    return;
  }
  struct epoll_event ev;
  ev.events = EPOLLIN;
  ev.data.u32 = 0;
  epoll_ctl(m_monitor.epollfd, EPOLL_CTL_ADD, m_socket.fd, &ev);
  m_monitor.curr_id = 1;
}

void Server::start()
{
  if (m_quit)
    return;
  if (listen(m_socket.fd, MAX_CONNECTIONS) != 0)
  {
    IRC_LOG_FATAL("Listen Failed");
    m_quit = true;
    return;
  }
  setNonBlocking(m_socket.fd);

  // MAIN LOOP
  while (!m_quit)
  {
    monitorConnections();
    while (!m_msgQueue.empty())
    {
      Message msg = m_msgQueue.front();
      processMessage(msg);
      m_msgQueue.pop_front();
    }
  }
}

void Server::monitorConnections()
{
  int nb_fds = epoll_wait(m_monitor.epollfd, m_monitor.events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
  if (nb_fds == -1)
  {
    IRC_LOG_FATAL("epoll_wait");
    m_quit = true;
    return;
  }
  for (int i = 0; i < nb_fds; ++i)
  {
    if (m_monitor.events[i].data.u32 == 0)
      acceptConnection();
    else
    {
      if (m_clientMap.find(m_monitor.events[i].data.u32) == m_clientMap.end())
        continue;
      TcpConn &conn = m_clientMap.at(m_monitor.events[i].data.u32)->getConnection();
      conn.processEvent(m_monitor.events[i]);
      if (!conn.isActive())
        destroyClient(m_clientMap.at(m_monitor.events[i].data.u32));
    }
  }
}

void Server::acceptConnection()
{
  socket_info clientSockInfo;
  uint32_t addr_len = sizeof(clientSockInfo.addr);
  if ((clientSockInfo.fd =
           accept(m_socket.fd, reinterpret_cast<sockaddr *>(&clientSockInfo.addr), &addr_len)) < 0)
  {
    IRC_LOG_ERROR("accept error");
    return;
  }
  setNonBlocking(clientSockInfo.fd);
  struct epoll_event ev;
  uint32_t id = m_monitor.curr_id;
  ++m_monitor.curr_id;
  ev.events = EPOLLIN;
  ev.data.u32 = id;
  if (epoll_ctl(m_monitor.epollfd, EPOLL_CTL_ADD, clientSockInfo.fd, &ev) == -1)
  {
    IRC_LOG_ERROR("epoll_ctl error");
    --m_monitor.curr_id;
    close(clientSockInfo.fd);
    return;
  }
  m_clientMap[id] = new Client(id, clientSockInfo, m_msgQueue, m_monitor.epollfd);
}

void Server::destroyClient(Client *client)
{
  if (client->getIsBot())
    m_bot_connected = false;
  if (client->isInited())
    pruneClient(*client, true);
  m_clientMap.erase(client->getId());
  delete client;
}

Client *Server::getClientByNick(const std::string &nick)
{
  for (client_iterator_t it = m_clientMap.begin(); it != m_clientMap.end(); ++it)
  {
    if (it->second->getNickname() == nick)
      return it->second;
  }

  return NULL;
}

Channel *Server::getChannelByName(const std::string &name)
{
  channel_iterator_t it = m_channelMap.find(name);

  if (it == m_channelMap.end())
    return (NULL);
  return it->second;
}

void Server::processMessage(Message &msg)
{
  irc::Response response;

  if (m_clientMap.find(msg.getId()) == m_clientMap.end())
    return;
  switch (msg.getType())
  {
  case CMD_NICK:
    response = NickCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_USER:
    response = UserCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_MODE:
    response = ModeCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_JOIN:
    response = JoinCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_TOPIC:
    response = TopicCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_INVITE:
    response = InviteCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_KICK:
    response = KickCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_PRIVMSG:
    response = PrivMsgCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  case CMD_PASS:
    response = PassCommand::execute(*this, *this->m_clientMap[msg.getId()], msg.getParameters());
    break;
  default:
    IRC_LOG_WARN("unknown message type");
  }

  if (!response.nil())
    this->m_clientMap[msg.getId()]->broadcast(response, IRC_SERVER_PREFIX);
}

Channel *Server::createChannel(const std::string &channel_name, const std::string &channel_key)
{
  Channel *new_channel = new Channel(channel_name, channel_key);
  m_channelMap[channel_name] = new_channel;
  for (client_iterator_t it = m_clientMap.begin(); it != m_clientMap.end(); it++)
  {
    if (it->second->getIsBot())
      new_channel->addClient(it->second);
  }
  return new_channel;
}

void Server::deleteChannel(Channel *to_remove)
{
  channel_iterator_t it = m_channelMap.find(to_remove->getName());
  if (it != m_channelMap.end())
  {
    m_channelMap.erase(to_remove->getName());
    delete to_remove;
  }
}

void Server::pruneClient(Client &client, bool destroy)
{
  std::vector<Channel *> to_remove;
  for (channel_iterator_t it = this->m_channelMap.begin(); it != this->m_channelMap.end(); it++)
  {
    if (it->second->removeClient(&client))
    {
      std::string message = IRC_RES_PART(client.hostname(), it->second->getName(), "Left");
      if (!destroy)
        client.broadcast(message);
      it->second->broadcast(message, &client);
      if (it->second->getClientCounter() == 0)
        to_remove.push_back(it->second);
    }
  }
  for (std::vector<Channel *>::iterator it = to_remove.begin(); it != to_remove.end(); it++)
    deleteChannel(*it);
}

void Server::stop() { m_quit = true; }

void Server::letHimCook(Client &client)
{
  for (channel_iterator_t it = m_channelMap.begin(); it != m_channelMap.end(); it++)
    it->second->addClient(&client);
}

bool Server::isPasswordSet() { return !m_password.empty(); }

bool Server::verifyClient(Client &client)
{
  if (client.getIsBot())
  {
    if (m_bot_connected)
      return false;
    m_bot_connected = true;
    this->letHimCook(client);
    return true;
  }
  if (m_password.empty())
    return true;
  if (client.getEnteredPassword() == this->m_password)
    return true;
  return false;
}