//
// Created by mrouault on 6/10/24.
//

#ifndef KICKCOMMAND_H
#define KICKCOMMAND_H

#include <Client.h>
#include <Server.h>
#include <irc.h>

class KickCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // KICKCOMMAND_H
