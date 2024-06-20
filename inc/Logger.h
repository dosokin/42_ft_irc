#ifndef IRC_LOGGER_H
#define IRC_LOGGER_H

#include <ctime>
#include <iomanip>
#include <iostream>

#define IRC_COLOR_BLACK "\e[0;30m"
#define IRC_COLOR_RED "\e[0;31m"
#define IRC_COLOR_GREEN "\e[0;32m"
#define IRC_COLOR_YELLOW "\e[0;33m"
#define IRC_COLOR_BLUE "\e[0;34m"
#define IRC_COLOR_PURPLE "\e[0;35m"
#define IRC_COLOR_CYAN "\e[0;36m"
#define IRC_COLOR_WHITE "\e[0;37m"
#define IRC_COLOR_GREY "\e[0;90m"

#define IRC_COLOR_BLACK_INTENSE "\e[0;90m"
#define IRC_COLOR_RED_INTENSE "\e[0;91m"
#define IRC_COLOR_GREEN_INTENSE "\e[0;92m"
#define IRC_COLOR_YELLOW_INTENSE "\e[0;93m"
#define IRC_COLOR_BLUE_INTENSE "\e[0;94m"
#define IRC_COLOR_PURPLE_INTENSE "\e[0;95m"
#define IRC_COLOR_CYAN_INTENSE "\e[0;96m"
#define IRC_COLOR_WHITE_INTENSE "\e[0;97m"

#define IRC_COLOR_RESET "\e[0m"

#define __irc_log__(color, level)                                                                            \
  std::cerr << Logger::getTime() << "   " << color << std::left << std::setw(7) << level << IRC_COLOR_RESET  \
            << "    " << IRC_COLOR_GREY << __FILE_NAME__ << ":" << __LINE__ << IRC_COLOR_RESET << "    "

#ifndef IRC_NO_TRACE
#define IRC_LOG_TRACE(args) __irc_log__(IRC_COLOR_CYAN, "TRACE") << args << std::endl
#else
#define IRC_LOG_TRACE(args)
#endif

#define IRC_LOG_INFO(args) __irc_log__(IRC_COLOR_GREEN, "INFO") << args << std::endl
#define IRC_LOG_WARN(args) __irc_log__(IRC_COLOR_YELLOW, "WARN") << args << std::endl
#define IRC_LOG_ERROR(args) __irc_log__(IRC_COLOR_RED_INTENSE, "ERROR") << args << std::endl
#define IRC_LOG_FATAL(args) __irc_log__(IRC_COLOR_PURPLE_INTENSE, "FATAL") << args << std::endl

class Logger
{
public:
  static std::string getTime()
  {
    std::time_t t = std::time(NULL);
    char format[256];
    std::strftime(format, 256, "%T ", std::gmtime(&t));
    return std::string(format);
  }
};

#endif
