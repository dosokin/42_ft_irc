#include "Logger.h"
#include <Server.h>
#include <csignal>
#include <sstream>
#include <string>

Server server;

void server_close(int sig)
{
  (void)sig;

  IRC_LOG_INFO("shutting down server");
  server.stop();
}

int getPort(const std::string &s)
{
  int result = 0;

  if (s.length() < 4 || s.length() > 5)
    return -1;
  if (s.find_first_not_of("0123456789") != std::string::npos)
    return -1;
  for (std::string::const_iterator it = s.begin(); it != s.end(); it++){
    if (!isdigit(*it))
      return -1;
    result = result * 10 + (*it - '0');
  }
  if (result < 1024 || result > 65353)
    return -1;
  return result;
}

bool isPasswordValid(const std::string &password)
{
  if (password.length() < 9 || password.length() > 23)
  {
    IRC_LOG_ERROR("password invalid: invalid length [9:23]");
    return false;
  }
  if (!irc::is_key(password))
  {
    IRC_LOG_ERROR("password invalid: whitespaces forbidden");
    return false;
  }
  if (password.find_first_of("~!@#$%^&*()_+{}|:;<>?[]") == std::string::npos)
  {
    IRC_LOG_ERROR("password invalid: special character missing");
    return false;
  }
  if (password.find_first_of("0123456789") == std::string::npos)
  {
    IRC_LOG_ERROR("password invalid: number missing");
    return false;
  }
  if (password.find_first_of("abcdefghijklmnopqrstuvwxyz") == std::string::npos)
  {
    IRC_LOG_ERROR("password invalid: lowercase letter missing");
    return false;
  }
  if (password.find_first_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == std::string::npos)
  {
    IRC_LOG_ERROR("password invalid: capital letter missing");
    return false;
  }
  return true;
}

int main(int argc, char **argv)
{
  int port;

  if (argc != 3)
  {
    IRC_LOG_ERROR("wrong arguments");
    return 1;
  }
  if ((port = getPort(argv[1])) == -1)
  {
    IRC_LOG_ERROR("wrong port");
    return 1;
  }
  if (!isPasswordValid(argv[2]))
    return 1;

  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, server_close);
  signal(SIGTERM, server_close);

  if (!server.initializeServer(port, argv[2]))
    return 1;

  IRC_LOG_INFO("starting server");
  server.start();

  return 0;
}