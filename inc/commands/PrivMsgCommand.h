//
// Created by dosokin on 6/10/24.
//

#ifndef IRC_PRIVMSGCOMMAND_HPP
#define IRC_PRIVMSGCOMMAND_HPP

#include <Client.h>
#include <Server.h>
#include <irc.h>

class PrivMsgCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif // IRC_PRIVMSGCOMMAND_HPP
