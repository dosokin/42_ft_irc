#include "JoinCommand.h"

static irc::Response join_existing_channel(Client &client, Channel *channel,
                                           const std::string &target_channel, const std::string &target_key)
{
  if (channel->hasKey() && channel->getKey() != target_key)
    return IRC_ERR_BADCHANNELKEY(client.getNickname(), target_channel);
  if (channel->isInviteOnly() && !channel->clientIsInvited(client.getId()))
    return IRC_ERR_INVITEONLYCHAN(client.getNickname(), target_channel);
  if (channel->getClientLimit() != Channel::UNLIMITED_CLIENTS && channel->getClientCounter() >= channel->getClientLimit())
    return IRC_ERR_CHANNELISFULL(client.getNickname(), target_channel);

  if (channel->isInviteOnly())
    channel->removeInvite(client.getId());
  channel->addClient(&client);
  channel->broadcast(IRC_RES_JOIN(client.hostname(), target_channel));

  if (!channel->getTopic().empty())
    client.broadcast(IRC_RPL_TOPIC(client.getNickname(), target_channel, channel->getTopic()),
                     IRC_SERVER_PREFIX);

  client.broadcast(IRC_RPL_NAMREPLY(client.getNickname(), target_channel, channel->namesList()),
                   IRC_SERVER_PREFIX);

  return IRC_RPL_ENDOFNAMES(client.getNickname(), target_channel);
}

irc::Response join_channel(Server &server, Client &client, const std::string &target_channel,
                           const std::string &target_key)
{
  Channel *channel = server.getChannelByName(target_channel);
  if (channel)
    return join_existing_channel(client, channel, target_channel, target_key);

  if (!irc::is_channel(target_channel))
    return IRC_ERR_NOSUCHCHANNEL(client.getNickname(), target_channel);

  channel = server.createChannel(target_channel, target_key);
  channel->addClient(&client);
  channel->addOperator(&client);
  channel->broadcast(IRC_RES_JOIN(client.hostname(), target_channel));

  client.broadcast(IRC_RPL_NAMREPLY(client.getNickname(), target_channel, channel->namesList()),
                   IRC_SERVER_PREFIX);
  return (IRC_RPL_ENDOFNAMES(client.getNickname(), target_channel));
}

typedef std::pair<std::string, std::string> join_pair_t;
typedef std::vector<join_pair_t>::iterator pair_iterator_t;

irc::Response JoinCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;

  if (parameters.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "JOIN");

  if (parameters.at(0) == "0")
  {
    server.pruneClient(client, false);
    return irc::Response();
  }

  std::vector<join_pair_t> pairs;
  std::vector<std::string> channels = irc::strings_split(parameters.at(0), ',');
  std::vector<std::string> keys;
  if (parameters.size() > 1)
    keys = irc::strings_split(parameters.at(1), ',');

  for (size_t i = 0; i < channels.size(); i++)
  {
    if (i < keys.size())
      pairs.push_back(std::make_pair(channels[i], keys[i]));
    else
      pairs.push_back(std::make_pair(channels[i], ""));
  }

  for (pair_iterator_t it = pairs.begin(); it != pairs.end(); it++)
    client.broadcast(join_channel(server, client, it->first, it->second), IRC_SERVER_PREFIX);
  return irc::Response();
}
