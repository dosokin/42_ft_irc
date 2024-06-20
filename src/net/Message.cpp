#include "Logger.h"
#include "irc.h"
#include <Message.h>
#include <iostream>

// static bool is_valid_parameter_char(char c)
//{
//   return (c != '\x00' && c != '\n' && c != '\r' && c != ':' && c != ' ');
// }

static msg_type msg_type_from_string(const std::string &type)
{
  if (type == "PASS")
    return CMD_PASS;
  if (type == "NICK")
    return CMD_NICK;
  if (type == "USER")
    return CMD_USER;
  if (type == "OPER")
    return CMD_OPER;
  if (type == "MODE")
    return CMD_MODE;
  if (type == "QUIT")
    return CMD_QUIT;
  if (type == "JOIN")
    return CMD_JOIN;
  if (type == "TOPIC")
    return CMD_TOPIC;
  if (type == "INVITE")
    return CMD_INVITE;
  if (type == "KICK")
    return CMD_KICK;
  if (type == "PRIVMSG")
    return CMD_PRIVMSG;
  return CMD_ERROR; // TODO: throw exception?
}

Message::Message() : m_type(CMD_ERROR), m_id() {}

Message Message::parse(std::string &raw_message)
{
  typedef std::string::iterator iter_s;

  Message message;
  std::vector<std::string> parameters;
  iter_s it = raw_message.begin();

  std::string type;
  for (; it < raw_message.end() && *it != ' '; ++it)
    type += *it;

  std::string parameter;

  size_t pos = raw_message.find(" :");
  if (pos == std::string::npos)
    pos = raw_message.size();
  iter_s stop = raw_message.begin() + pos;
  for (; it < stop; ++it)
  {
    parameter = "";
    while (it < stop && *it == ' ')
      ++it;

    while (it < stop && *it != ' ')
      parameter += *(it++);
    parameters.push_back(parameter);
  }
  if (stop != raw_message.end())
    parameters.push_back(raw_message.substr(pos + 2));
  message.setType(msg_type_from_string(type));
  message.setParameters(parameters);

  return message;
}

msg_type Message::getType() const { return m_type; }

uint32_t Message::getId() const { return m_id; }

void Message::setId(uint32_t t_id) { m_id = t_id; }

const std::vector<std::string> &Message::getParameters() const { return m_parameters; }

void Message::setParameters(const std::vector<std::string> &t_parameters) { m_parameters = t_parameters; }

void Message::setType(msg_type t_type) { m_type = t_type; }
