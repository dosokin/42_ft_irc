#include <Logger.h>
#include "TopicCommand.h"

irc::Response TopicCommand::execute(Server &server, Client &client,
                                    const std::vector<std::string> &parameters)
{
  if (!client.isInited())
    return IRC_ERR_NOTREGISTERED;

  if (parameters.empty())
    return IRC_ERR_NEEDMOREPARAMS(client.getNickname(), "TOPIC");

  const std::string &channel_name = parameters.at(0);
  Channel *channel = server.getChannelByName(channel_name);
  if (channel == NULL || !channel->clientInChannel(&client))
    return IRC_ERR_NOTONCHANNEL(client.getNickname(), channel_name);

  if (parameters.size() < 2)
  {
    if (channel->getTopic().empty())
      return IRC_RPL_NOTOPIC(client.getNickname(), channel_name);
    return IRC_RPL_TOPIC(client.getNickname(), channel_name, channel->getTopic());
  }

  //  Client need to be channel operator in order to change the topic
  if (channel->isTopicRestricted() && !channel->clientIsOperator(client.getId()))
    return IRC_ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name);

  const std::string &topic = parameters.at(1);

  IRC_LOG_TRACE("Setting channel (" << channel_name << ") topic: " << topic);

  channel->setTopic(topic);
  // Indicate the change of topic
  channel->broadcast(IRC_RES_TOPIC(client.hostname(), channel_name, channel->getSafeTopic()));

  return irc::Response();
}
