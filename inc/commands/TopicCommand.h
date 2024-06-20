//
// Created by mrouault on 6/10/24.
//

#ifndef TOPICCOMMAND_H
#define TOPICCOMMAND_H

#include <Client.h>
#include <Server.h>
#include <irc.h>

class TopicCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // TOPICCOMMAND_H
