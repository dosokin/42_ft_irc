#include "Utils.h"
#include <csignal>
#include <iostream>
#include <vector>

Koakoubot bot;

void signal_handler(int sig)
{
  (void)sig;
  bot.destroy();
  bot.setExit(true);
}

void print_help() { std::cout << "Usage: <address> <port>" << std::endl; }

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

int main(int ac, char **av)
{
  signal(SIGINT, signal_handler);
  signal(SIGPIPE, SIG_IGN);
  if (ac != 3)
  {
    print_help();
    return 1;
  }
  if (getPort(av[2]) == -1)
  {
    std::cerr << "wrong port" << std::endl;
    return 1;
  }
  try
  {
    bot.initKoakoubot(av[1], av[2]);
    while (true)
    {
      bot.receiveMessageFromServ();
      if (bot.getExit())
        return 0;
      while (!bot.getMsgQueue().empty())
      {
        std::string msg = bot.getMsgQueue().front();
        bot.getMsgQueue().pop_front();

        std::vector<std::string> splitted = strings_split(msg, ' ');
        if (splitted.size() >= 3 && splitted.at(1) == "PRIVMSG")
        {
          std::string koi;
          std::string channel = splitted.at(2);
          if (msg.substr(msg.length() - 3) == "?\r\n")
            koi = "feur peut-etre?";
          else if (msg.find("quoi") != std::string::npos)
            koi = "coubeh ? titre : pleure";
          if (!koi.empty())
            bot.sendMessageToServ(PRIVMSG_CMD(channel, koi));
        }
        std::cout << (std::string)msg << std::endl;
      }
    }
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
