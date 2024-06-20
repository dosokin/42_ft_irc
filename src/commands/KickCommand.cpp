#include "KickCommand.h"
#include <irc.h>

typedef std::pair<std::string, std::string> kick_pair_t;
typedef std::vector<kick_pair_t>::iterator pair_iterator_t;

static std::vector<kick_pair_t> make_kick_pair(const std::vector<std::string> &parameters,
                                               bool &not_enough_params)
{
  std::vector<std::string> channels = irc::strings_split(parameters.at(0), ',');
  std::vector<std::string> nicknames = irc::strings_split(parameters.at(1), ',');
  std::vector<kick_pair_t> pairs;

  for (size_t i = 0; i < channels.size(); ++i)
  {
    if (i > nicknames.size())
    {
      not_enough_params = true;
      return pairs;
    }
    pairs.push_back(std::make_pair(channels.at(i), nicknames.at(i)));
  }

  return pairs;
}

static irc::Response process_kick_pair(Server &server, Client &client, kick_pair_t &pair,
                                       const std::string &reason)
{
  const std::string &channel_name = pair.first;
  const std::string &nickname = pair.second;
  Channel *channel;

  if (!irc::is_channel(channel_name))
    return IRC_ERR_BADCHANMASK(client.getNickname(), channel_name);

  channel = server.getChannelByName(channel_name);
  if (channel == NULL)
    return IRC_ERR_NOSUCHCHANNEL(client.getNickname(), channel_name);
  if (!channel->clientIsOperator(client.getId()))
    return IRC_ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name);

  Client *to_kick = server.getClientByNick(nickname);
  if (to_kick == NULL)
    return IRC_ERR_NOSUCHNICK(client.getNickname(), channel_name);
  if (!channel->clientInChannel(to_kick))
    return IRC_ERR_USERNOTINCHANNEL(client.getNickname(), nickname, channel_name);

  channel->removeClient(to_kick);

  std::string broadcast_message =
      IRC_RES_KICK(client.hostname(), channel_name, nickname);
  if (!reason.empty())
    broadcast_message += " :" + reason;
  to_kick->broadcast(broadcast_message);
  client.broadcast(broadcast_message);

  return irc::Response();
}

irc::Response KickCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;
  if (parameters.size() < 2)
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "KICK");

  bool not_enough_params = false;
  std::vector<kick_pair_t> pairs = make_kick_pair(parameters, not_enough_params);
  if (not_enough_params)
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "KICK");
  std::string reason;
  if (parameters.size() >= 3)
    reason = parameters.at(2);

  for (pair_iterator_t it = pairs.begin(); it != pairs.end(); ++it)
  {
    const irc::Response &response = process_kick_pair(server, client, *it, reason);
    if (!response.nil())
      client.broadcast(response, IRC_SERVER_PREFIX);
  }

  return irc::Response();
}
