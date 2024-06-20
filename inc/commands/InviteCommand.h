//
// Created by mrouault on 6/12/24.
//

#ifndef FT_IRC_INVITECOMMAND_H
#define FT_IRC_INVITECOMMAND_H

#include <Client.h>
#include <Server.h>
#include <irc.h>

class InviteCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // FT_IRC_INVITECOMMAND_H
