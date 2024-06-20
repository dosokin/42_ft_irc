#include "ModeCommand.h"
#include "Logger.h"
#include <cstdlib>

struct ModeDescriptor
{
  char mode;
  bool set;
  ModeDescriptor(char _mode, bool _set) : mode(_mode), set(_set) {}
};

typedef std::pair<ModeDescriptor, std::string> mode_pair_t;

static irc::Response mode_push_pair(Client &client, Channel *channel, std::vector<mode_pair_t> &pairs,
                                    char mode, bool set, const std::vector<std::string> &parameters,
                                    size_t &opt_param_idx)
{
  switch (mode)
  {
  case 'i':
  case 't':
    pairs.push_back(std::make_pair(ModeDescriptor(mode, set), ""));
    break;
  case 'k':
  case 'l':
    if (set && opt_param_idx >= parameters.size())
      return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
    pairs.push_back(std::make_pair(ModeDescriptor(mode, set), set ? parameters.at(opt_param_idx) : ""));
    if (set)
      opt_param_idx++;
    break;
  case 'o':
    if (opt_param_idx >= parameters.size())
      return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
    pairs.push_back(std::make_pair(ModeDescriptor(mode, set), parameters.at(opt_param_idx)));
    opt_param_idx++;
    break;
  default:
    IRC_LOG_WARN("unknown mode passed: '" << mode << "'");
    return IRC_ERR_UNKNOWNMODE(client.getNickname(), std::string(1, mode), channel->getName());
  }

  return irc::Response();
}

static std::vector<mode_pair_t> mode_create_pairs(Client &client, Channel *channel,
                                                  const std::vector<std::string> &parameters,
                                                  irc::Response &response)
{
  size_t i = 2;
  std::vector<mode_pair_t> pairs;

  for (std::vector<std::string>::const_iterator it = parameters.begin() + 1; it < parameters.end(); ++it, ++i)
  {
    const std::string &modes = *it;
    IRC_LOG_TRACE(*it);
    if (modes.size() < 2)
    {
      response = IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
      return pairs;
    }
    if (modes.at(0) != '+' && modes.at(0) != '-')
    {
      response = IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
      return pairs;
    }

    size_t old = i;
    const bool set = modes.at(0) == '+';
    for (std::string::const_iterator sit = modes.begin() + 1; sit != modes.end(); ++sit)
    {
      response = mode_push_pair(client, channel, pairs, *sit, set, parameters, i);
      if (!response.nil())
        return pairs;
    }

    it += (i - old);
  }

  return pairs;
}

static void process_mode_invite(Channel *channel, bool set) { channel->setInviteOnly(set); }

static void process_mode_topic(Channel *channel, bool set) { channel->setTopicRestricted(set); }

static irc::Response process_mode_key(Channel *channel, Client &client, bool set, const std::string &key)
{
  if (set && key.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
  if (set && channel->hasKey())
    return IRC_ERR_KEYSET(client.getNickname(), channel->getName());
  channel->setKey(set ? key : "");
  return irc::Response();
}

static irc::Response process_mode_operator(Channel *channel, Server &server, Client &client, bool set,
                                           const std::string &nick)
{
  if (nick.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
  Client *op = server.getClientByNick(nick);
  if (op == NULL || !channel->clientInChannel(op))
    return IRC_ERR_USERNOTINCHANNEL(client.getNickname(), nick, channel->getName());
  if (set)
    channel->addOperator(op);
  else
    channel->removeOperator(op);
  return irc::Response();
}

static irc::Response process_mode_limit(Channel *channel, Client &client, bool set, const std::string &limit)
{
  int64_t client_limit = Channel::UNLIMITED_CLIENTS;
  if (set && limit.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
  if (set && limit.find_first_not_of("0123456789") != std::string::npos)
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");
  if (set)
    client_limit = std::atoi(limit.c_str());
  channel->setClientLimit(client_limit);
  return irc::Response();
}

static irc::Response mode_pair_process(Server &server, Client &client, Channel *channel,
                                       const mode_pair_t &pair)
{
  const bool set = pair.first.set;
  const char mode = pair.first.mode;

  switch (mode)
  {
  case 'i':
    process_mode_invite(channel, set);
    break;
  case 't':
    process_mode_topic(channel, set);
    break;
  case 'k':
    return process_mode_key(channel, client, set, pair.second);
  case 'o':
    return process_mode_operator(channel, server, client, set, pair.second);
  case 'l':
    return process_mode_limit(channel, client, set, pair.second);
  default:
    IRC_LOG_WARN("unknown mode passed: '" << mode << "'");
    return IRC_ERR_UNKNOWNMODE(client.getNickname(), std::string(1, mode), channel->getName());
  }

  return irc::Response();
}

irc::Response ModeCommand::execute(Server &server, Client &client, const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;
  if (parameters.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "MODE");

  const std::string &channel_name = parameters.at(0);
  if (!irc::is_channel(channel_name))
    return IRC_ERR_NOTONCHANNEL(client.getNickname(), channel_name);

  Channel *channel = server.getChannelByName(channel_name);
  if (channel == NULL)
    return IRC_ERR_NOTONCHANNEL(client.getNickname(), channel_name);

  if (parameters.size() < 2)
    return IRC_RPL_CHANNELMODEIS(client.getNickname(), channel_name, channel->modeString());

  if (!channel->clientIsOperator(client.getId()))
    return IRC_ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name);

  irc::Response response;
  std::vector<mode_pair_t> pairs = mode_create_pairs(client, channel, parameters, response);
  if (!response.nil())
    return response;

  for (std::vector<mode_pair_t>::iterator it = pairs.begin(); it != pairs.end(); ++it)
  {
    IRC_LOG_TRACE("setting mode: '" << it->first.mode << "' arg='" << it->second << "'");
    if (!(response = mode_pair_process(server, client, channel, *it)).nil())
      return response;
  }

  std::string buffer;
  for (std::vector<mode_pair_t>::iterator it = pairs.begin(); it != pairs.end(); ++it)
  {
    const bool set = it->first.set;
    const char mode = it->first.mode;
    const std::string message =
        (set ? "+" : "-") + std::string(1, mode) + (it->second.empty() ? "" : " :" + it->second);
    channel->broadcast(IRC_RES_MODE(client.hostname(), channel_name, message));
  }

  return irc::Response();
}
