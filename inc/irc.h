//
// Created by mrouault on 5/30/24.
//

#ifndef IRC_H
#define IRC_H

#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

#define IRC_CLIENT_HOST "@zangoledozo.42.irc"
#define IRC_SERVER_PREFIX "icecream-racing-cats.42.fr"

#define IRC_RES(client, cmd) (":" + client + " " + cmd + " ")
#define IRC_RES_INVITE(client, invited, channel) (IRC_RES(client, "INVITE") + invited + " " + channel)
#define IRC_RES_TOPIC(client, channel, topic) (IRC_RES(client, "TOPIC") + channel + " " + topic)
#define IRC_RES_KICK(client, channel, kicked)                                                      \
  (IRC_RES(client, "KICK") + channel + " " + kicked)
#define IRC_RES_PRIVMSG(client, recipient, message) (IRC_RES(client, "PRIVMSG") + recipient + " :" + message)
#define IRC_RES_JOIN(client, channel) (IRC_RES(client, "JOIN") + channel)
#define IRC_RES_PART(client, channel, message) (IRC_RES(client, "PART") + channel + " :" + message)
#define IRC_RES_MODE(client, channel, message) (IRC_RES(client, "MODE") + channel + " " + message)

#define IRC_ERR_NEEDMOREPARAMS(client, command) irc::Response(451, client, command, ":Not enough parameters")
#define IRC_ERR_NONICKNAMEGIVEN(client) irc::Response(431, client, ":No nickname given")
#define IRC_ERR_ERRONEUSNICKNAME(client, nick) irc::Response(432, client, nick, ":Erroneous nickname")
#define IRC_ERR_NICKNAMEINUSE(client, nick) irc::Response(433, client, nick, ":Nickname is already in use")
#define IRC_ERR_BADCHANMASK(client, channel) irc::Response(476, client, channel, ":Bad Channel Mask")
#define IRC_ERR_NOSUCHCHANNEL(client, channel) irc::Response(403, client, channel, ":No such channel")
#define IRC_ERR_NOSUCHNICK(client, nick) irc::Response(401, client, nick, ":No such nick/channel")
#define IRC_ERR_USERNOTINCHANNEL(client, nick, channel)                                                      \
  irc::Response(441, client, nick, channel, ":They aren't on that channel")
#define IRC_ERR_NOTONCHANNEL(client, channel)                                                                \
  irc::Response(442, client, channel, ":You're not on that channel")
#define IRC_ERR_CHANOPRIVSNEEDED(client, channel)                                                            \
  irc::Response(482, client, channel, ":You're not channel operator")
#define IRC_ERR_USERONCHANNEL(client, user, channel)                                                         \
  irc::Response(443, client, user, channel, ":is already on channel")
#define IRC_ERR_KEYSET(client, channel) irc::Response(467, client, channel, ":Channel key already set")
#define IRC_ERR_UNKNOWNMODE(client, mode, channel)                                                           \
  irc::Response(472, client, mode, ":is unknown mode char to me for " + channel)
#define IRC_ERR_BADCHANNELKEY(client, channel)                                                               \
  irc::Response(475, client, channel, ":Cannot join channel (+k)")
#define IRC_ERR_NOTREGISTERED irc::Response(451, ":You have not registered")
#define IRC_ERR_ALREADYREGISTRED(client) irc::Response(462, client, ":You may not reregister")
#define IRC_ERR_CHANNELISFULL(client, channel)                                                               \
  irc::Response(471, client, channel, ":Cannot join channel (+l)")
#define IRC_ERR_INVITEONLYCHAN(client, channel)                                                              \
  irc::Response(473, client, channel, ":Cannot join channel (+i)")
#define IRC_ERR_CANNOTSENDTOCHAN(client, channel)                                                            \
  irc::Response(404, client, channel, ":Cannot send to channel")
#define IRC_ERR_NOTEXTTOSEND(client) irc::Response(412, client, ":No text to send")
#define IRC_ERR_NORECIPIENT(client, command) irc::Response(411, client, ":No recipient given (" command ")")
#define IRC_ERR_INVALIDUSERNAME(client) irc::Response(468, client, ":Malformed username")
#define IRC_RPL_NOTOPIC(client, channel) irc::Response(331, client, channel, ":No topic is set")
#define IRC_RPL_TOPIC(client, channel, topic) irc::Response(332, client, channel, ":" + topic)
#define IRC_RPL_INVITING(client, channel, nick) irc::Response(341, client, channel, nick)
#define IRC_RPL_NAMREPLY(client, channel, nicknames)                                                         \
  irc::Response(353, client + " =", channel, ":" + nicknames)
#define IRC_RPL_CHANNELMODEIS(client, channel, mode) irc::Response(324, client, channel, mode)
#define IRC_RPL_ENDOFNAMES(client, channel) irc::Response(366, client, channel, ":End of NAMES list")
#define IRC_RPL_WELCOME(client) irc::Response(1, client, ":Welcome to the Internet Relay Network", client)
#define IRC_ERR_PASSWDMISMATCH(client) irc::Response(464, client, ":Password incorrect")

namespace irc
{
class Response
{
public:
  Response();
  Response(uint16_t code, const std::string &message);
  Response(uint16_t code, const std::string &param, const std::string &message);
  Response(uint16_t code, const std::string &p1, const std::string &p2, const std::string &message);
  Response(uint16_t code, const std::string &p1, const std::string &p2, const std::string &p3,
           const std::string &message);

public:
  std::string message() const;
  std::string message(const std::string &server_prefix) const;
  bool nil() const;

private:
  bool m_nil;
  std::string m_message;
  uint16_t m_code;
};

bool is_target(const std::string &value);
bool is_msg_target(const std::string &value);
bool is_msg_to(const std::string &value);
bool is_channel(const std::string &value);
bool is_servername(const std::string &value);
bool is_host(const std::string &value);
bool is_hostname(const std::string &value);
bool is_shortname(const std::string &value);
bool is_hostaddr(const std::string &value);
bool is_ip4addr(const std::string &value);
bool is_ip6addr(const std::string &value);
bool is_targetmask(const std::string &value);
bool is_chanstring(const std::string &value);
bool is_channel_id(const std::string &value);
bool is_user(const std::string &value);
bool is_key(const std::string &value);
bool is_special(char c);
bool is_nickname(const std::string &value);
bool is_mask(const std::string &value);

std::vector<std::string> strings_split(const std::string &s, char c);
std::string strings_join(const std::vector<std::string> &s, const std::string &separator);

}

template <typename T> std::string to_string(T n)
{
  std::ostringstream ss;
  ss << n;
  return ss.str();
}

#endif
