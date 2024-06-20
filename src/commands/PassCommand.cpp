#include "PassCommand.h"

irc::Response PassCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters){
  if (parameters.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "PASS");
  if (client.isInited())
    return IRC_ERR_ALREADYREGISTRED(client.getNickname());
  if (!server.isPasswordSet())
    return irc::Response();
  client.setEnteredPassword(parameters.at(0));
  return irc::Response();
}