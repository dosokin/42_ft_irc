#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <string>
#include <vector>

enum msg_type
{
  // Server Operations
  CMD_PASS,
  CMD_NICK,
  CMD_USER,
  CMD_SERVER,
  CMD_OPER,
  CMD_QUIT,
  CMD_SQUIT,
  // Channel Operations
  CMD_JOIN,
  CMD_PART,
  CMD_MODE,
  CMD_TOPIC,
  CMD_NAMES,
  CMD_LIST,
  CMD_INVITE,
  CMD_KICK,
  // Server Queries & Cmd
  CMD_VERSION,
  CMD_STATS,
  CMD_LINKS,
  CMD_TIME,
  CMD_CONNECT,
  CMD_TRACE,
  CMD_ADMIN,
  CMD_INFO,
  // Message Sending
  CMD_PRIVMSG,
  CMD_NOTICE,
  // User Based Queries
  CMD_WHO,
  CMD_WHOIS,
  CMD_WHOWAS,
  // Misc
  CMD_KILL,
  CMD_PING,
  CMD_PONG,
  CMD_ERROR,
  // Optional (^^)
  CMD_AWAY,
  CMD_REHASH,
  CMD_RESTART,
  CMD_SUMMON,
  CMD_USERS,
  CMD_WALLOPS,
  CMD_USERHOST,
  CMD_ISON
};

class Message
{
public:
  Message();
  static Message parse(std::string &raw_message);

  msg_type getType() const;
  void setType(msg_type t_type);

  uint32_t getId() const;
  void setId(uint32_t t_id);

  const std::vector<std::string> &getParameters() const;
  void setParameters(const std::vector<std::string> &t_parameters);

private:
  msg_type m_type;
  uint32_t m_id;
  std::vector<std::string> m_parameters;
};

#endif