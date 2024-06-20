//
// Created by dosokin on 6/10/24.
//

#ifndef NICKCOMMAND_H
#define NICKCOMMAND_H

#include <Client.h>
#include <Server.h>
#include <irc.h>

class NickCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif
