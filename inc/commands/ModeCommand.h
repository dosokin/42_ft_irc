//
// Created by mrouault on 6/10/24.
//

#ifndef MODECOMMAND_H
#define MODECOMMAND_H

#include <Client.h>
#include <Server.h>
#include <irc.h>

class ModeCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // MODECOMMAND_H
