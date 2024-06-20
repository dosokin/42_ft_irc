#include "UserCommand.h"

irc::Response UserCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters)
{
  (void)server;

  if (client.isInited())
    return IRC_ERR_ALREADYREGISTRED(client.getNickname());

  if (parameters.size() < 4)
    return IRC_ERR_NEEDMOREPARAMS("*", "USER");

  if (!irc::is_user(parameters.at(0)))
    return IRC_ERR_INVALIDUSERNAME("*");

  client.setUsername(parameters[0]);

  if (client.isInited())
  {
    if (server.verifyClient(client))
      return IRC_RPL_WELCOME(client.getNickname());
    client.broadcast(IRC_ERR_PASSWDMISMATCH(client.getNickname()), IRC_SERVER_PREFIX);
    server.destroyClient(&client);
  }
  return irc::Response();
}
