#ifndef CLIENT_H
#define CLIENT_H

#include <TcpConn.h>
#include <deque>
#include <irc.h>

class Client
{
public:
  Client(uint32_t id, const socket_info &sockInfo, std::deque<Message> &msgQueue, int epollfd = -1);
  ~Client();

  uint32_t getId() const;
  TcpConn &getConnection();
  bool    getIsBot() const;

  const std::string &getNickname() const;
  void setNickname(const std::string &new_nickname);

  void setUsername(const std::string &username);
  const std::string &getUsername() const;
  bool isInited() const;

  std::string hostname() const;
  void broadcast(const std::string &message);
  void broadcast(const irc::Response &response);
  void broadcast(const irc::Response &response, const std::string &prefix);

  void        setEnteredPassword(const std::string &password);
  const std::string &getEnteredPassword() const;

private:
  uint32_t m_id;
  TcpConn m_conn;
  std::string m_username;
  std::string m_nickname;
  bool m_inited;
  bool m_user_set;
  bool m_nick_set;
  bool m_password_set;
  std::string m_entered_password;
  bool m_bot;
  // TODO: m_mode;
};

#endif