//
// Created by dosokin on 6/10/24
//

#ifndef IRC_JOINCOMMAND_HPP
#define IRC_JOINCOMMAND_HPP

#include <Client.h>
#include <Server.h>
#include <irc.h>

class JoinCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // IRC_JOINCOMMAND_HPP
