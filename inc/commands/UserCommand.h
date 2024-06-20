//
// Created by dosokin on 6/10/24.
//

#ifndef USERCOMMAND_HPP
#define USERCOMMAND_HPP

#include <Client.h>
#include <Server.h>
#include <irc.h>

class UserCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif
