#include "PrivMsgCommand.h"

struct Target
{
  enum target_type
  {
    CHANNEL,
    CLIENT,
  };
  target_type type;
  std::string name;
};

std::vector<Target> getTarget(const std::string &targets)
{
  std::vector<Target> targetArray;
  std::vector<std::string> recipients = irc::strings_split(targets, ',');

  for (std::vector<std::string>::iterator it = recipients.begin(); it != recipients.end(); it++)
  {
    Target newTarget;
    if (it->empty())
      continue;
    if (it->at(0) == '&' || it->at(0) == '#')
      newTarget.type = Target::CHANNEL;
    else
      newTarget.type = Target::CLIENT;
    newTarget.name = *it;
    targetArray.push_back(newTarget);
  }
  return targetArray;
}

irc::Response PrivMsgCommand::execute(Server &server, Client &client,
                                      const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;

  if (parameters.empty())
    return IRC_ERR_NORECIPIENT(client.getNickname(), "PRIVMSG");

  if (parameters.size() < 2)
    return IRC_ERR_NOTEXTTOSEND(client.getNickname());

  std::vector<Target> target = getTarget(parameters.at(0));
  for (std::vector<Target>::iterator it = target.begin(); it != target.end(); it++)
  {
    if (it->type == Target::CLIENT)
    {
      Client *recipient = server.getClientByNick(it->name);
      if (recipient == NULL)
        return IRC_ERR_NOSUCHNICK(client.getNickname(), it->name);
      recipient->broadcast(IRC_RES_PRIVMSG(client.hostname(), it->name, parameters.at(1)));
    }
    else if (it->type == Target::CHANNEL)
    {
      Channel *recipient = server.getChannelByName(it->name);
      if (recipient == NULL || !recipient->clientInChannel(&client))
        return IRC_ERR_CANNOTSENDTOCHAN(client.getNickname(), it->name);
      recipient->broadcast(IRC_RES_PRIVMSG(client.hostname(), it->name, parameters.at(1)), &client);
    }
  }

  return irc::Response();
}
