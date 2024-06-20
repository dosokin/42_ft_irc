#ifndef IRC_PASSCOMMAND_HPP
#define IRC_PASSCOMMAND_HPP

#include "irc.h"
#include "Server.h"
#include "Client.h"

class PassCommand
{
public:
  static irc::Response execute(Server &server, Client &client, const std::vector<std::string> &parameters);
};

#endif
