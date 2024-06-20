#include "InviteCommand.h"

irc::Response InviteCommand::execute(Server &server, Client &client,
                                     const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;
  if (parameters.size() < 2)
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "INVITE");

  const std::string &nick = parameters.at(0);
  const std::string &channel_name = parameters.at(1);

  Channel *channel = server.getChannelByName(channel_name);
  if (channel == NULL)
    return IRC_ERR_NOSUCHCHANNEL(client.getNickname(), channel_name);

  // User who invites MUST be in the channel
  if (!channel->clientInChannel(&client))
    return IRC_ERR_NOTONCHANNEL(client.getNickname(), channel_name);

  Client *invited = server.getClientByNick(nick);
  if (invited == NULL)
    return IRC_ERR_NOSUCHNICK(client.getNickname(), nick);
  if (channel->clientInChannel(invited))
    return IRC_ERR_USERONCHANNEL(client.getNickname(), nick, channel_name);

  // When channel is invite only, only operators are allowed to send invites
  if (channel->isInviteOnly() && !channel->clientIsOperator(client.getId()))
    return IRC_ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name);

  invited->broadcast(IRC_RES_INVITE(client.hostname(), nick, channel_name));
  channel->pushInvite(invited->getId());

  return IRC_RPL_INVITING(client.getNickname(), channel_name, nick);
}