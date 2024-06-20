#include "Logger.h"
#include <Client.h>
#include <cstdlib>

Client::Client(uint32_t id, const socket_info &sockInfo, std::deque<Message> &msgQueue, int epollfd)
    : m_id(id), m_conn(id, sockInfo, msgQueue, epollfd), m_inited(), m_user_set(), m_nick_set(), m_password_set(), m_bot()
{
}

Client::~Client() {}

uint32_t Client::getId() const { return m_id; }
TcpConn &Client::getConnection() { return m_conn; }
const std::string &Client::getNickname() const { return m_nickname; }
bool Client::isInited() const { return m_inited; }

void Client::setNickname(const std::string &new_nickname)
{
  srand(time(NULL));
  int seconds = rand();
  std::ostringstream convert;
  convert << seconds;
  std::string secondstr = convert.str();
  if (new_nickname == "bot" + secondstr.substr(secondstr.length() - 5, 4))
    m_bot = true;
  m_nickname = new_nickname;
  m_nick_set = true;
  if (m_user_set)
    m_inited = true;
}

void Client::setUsername(const std::string &username)
{
  m_username = username;
  m_user_set = true;
  if (m_nick_set)
    m_inited = true;
}

void Client::broadcast(const std::string &message) {
  IRC_LOG_INFO("client_send: " << m_nickname << " <- '" << message << "'");
  getConnection().sendMsg(message);
}

void Client::broadcast(const irc::Response &response) { broadcast(response.message()); }

void Client::broadcast(const irc::Response &response, const std::string &prefix)
{
  broadcast(response.message(prefix));
}

std::string Client::hostname() const
{
  if (getUsername().empty())
    return getNickname() + IRC_CLIENT_HOST;
  return getNickname() + "!" + getUsername() + IRC_CLIENT_HOST;
}

const std::string &Client::getUsername() const { return m_username; }

bool  Client::getIsBot() const
{return m_bot;}


void Client::setEnteredPassword(const std::string &password)
{
  if (!m_password_set)
  {
    m_password_set = true;
    m_entered_password = password;
  }
}

const std::string &Client::getEnteredPassword() const{
  return m_entered_password;
}

