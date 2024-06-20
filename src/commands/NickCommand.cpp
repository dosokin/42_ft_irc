#include "NickCommand.h"

irc::Response NickCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters)
{
  std::string current_nick = "*";
  if (client.isInited())
    current_nick = client.getNickname();
  if (parameters.empty())
    return IRC_ERR_NONICKNAMEGIVEN(current_nick);

  const std::string &nick = parameters.at(0);
  if (!irc::is_nickname(nick))
    return IRC_ERR_ERRONEUSNICKNAME(current_nick, nick);

  if (server.getClientByNick(nick) != NULL)
    return IRC_ERR_NICKNAMEINUSE(current_nick, nick);

  client.setNickname(nick);
  if (client.isInited())
  {
    if (server.verifyClient(client))
      return IRC_RPL_WELCOME(client.getNickname());
    client.broadcast(IRC_ERR_PASSWDMISMATCH(client.getNickname()), IRC_SERVER_PREFIX);
    server.destroyClient(&client);
  }
  return irc::Response();
}
