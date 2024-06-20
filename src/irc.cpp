//
// Created by mrouault on 5/30/24.
//

#include <cctype>
#include <irc.h>

static bool string_starts_with_any(const std::string &str, const std::string &chars)
{
  return (chars.find_first_of(str[0]) != std::string::npos);
}

static bool char_is_any(char c, const std::string &chars)
{
  return chars.find_first_of(c) != std::string::npos;
}

std::vector<std::string> irc::strings_split(const std::string &s, char c)
{
  std::vector<std::string> splitted;

  for (size_t i = 0, j = 0; j != s.length();)
  {
    j = s.find(c, i);

    if (j == std::string::npos)
      j = s.length();

    std::string newPart = s.substr(i, j - i);
    splitted.push_back(newPart);
    i = j + 1;
  }

  return splitted;
}

std::string irc::strings_join(const std::vector<std::string> &s, const std::string &separator)
{
  std::string result;

  if (s.empty())
    return "";
  if (s.size() == 1)
    return s[0];

  std::vector<std::string>::const_iterator it = s.begin();
  for (; it < s.end() - 1; it++)
    result.append(*it + separator);
  result.append(*it);

  return result;
}

bool irc::is_target(const std::string &value)
{
  return (irc::is_nickname(value) || irc::is_servername(value));
}

bool irc::is_msg_target(const std::string &value)
{
  if (value.empty())
    return false;

  if (value.at(0) == ',')
    return false;

  std::vector<std::string> splited;
  splited = strings_split(value, ',');

  for (std::vector<std::string>::const_iterator it = splited.begin(); it != splited.end(); it++)
  {
    if (it->empty() || !irc::is_msg_to(value))
      return false;
  }

  return true;
}

bool irc::is_msg_to(const std::string &value)
{
  size_t i, j;
  if (irc::is_channel(value) || irc::is_nickname(value) || irc::is_targetmask(value))
    return true;

  if (value.find_first_of('!') != std::string::npos)
  {
    i = value.find_first_of('!');
    j = value.find_first_of('@');
    return (irc::is_nickname(value.substr(0, i)) && irc::is_user(value.substr(i + 1, i - j - 1)) &&
            irc::is_host(value.substr(j + 1)));
  }
  std::vector<std::string> splited = strings_split(value, '@');
  if (splited.size() == 1)
  {
    i = value.find_first_of('%');
    return (irc::is_nickname(value.substr(0, i)) && irc::is_host(value.substr(i + 1)));
  }
  if (!is_servername(splited.at(1)))
    return false;
  i = splited.at(0).find_first_of('%');
  if (i == std::string::npos)
    return (irc::is_user(splited.at(0)));
  return (irc::is_user(value.substr(0, i)) && irc::is_host(value.substr(i + 1)));
}

bool irc::is_channel(const std::string &value)
{
  std::string new_value;
  if (string_starts_with_any(value, "!"))
  {
    if (!irc::is_channel_id(value.substr(1, 5)))
      return false;
    new_value = value.substr(6);
  }
  else
  {
    if (!string_starts_with_any(value, "#+&"))
      return false;
    new_value = value.substr(1);
  }

  std::vector<std::string> splited;
  splited = strings_split(new_value, ':');

  for (std::vector<std::string>::const_iterator it = splited.begin(); it != splited.end(); it++)
  {
    if (it->empty() || !irc::is_chanstring(value))
      return false;
  }

  return true;
}

bool irc::is_servername(const std::string &value) { return (irc::is_hostname(value)); }

bool irc::is_host(const std::string &value) { return (irc::is_hostname(value) || irc::is_hostaddr(value)); }

bool irc::is_hostname(const std::string &value)
{
  std::vector<std::string> splited;
  splited = strings_split(value, '.');

  for (std::vector<std::string>::const_iterator it = splited.begin(); it != splited.end(); it++)
  {
    if (it->empty() || !irc::is_shortname(value))
      return false;
  }

  return true;
}

bool irc::is_shortname(const std::string &value)
{
  if (!(isalnum(value.at(0)) && isalnum(value.at(value.length() - 1))))
    return false;
  for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
  {
    if (!(isalnum(*it) || *it == '-'))
      return false;
  }
  return true;
}

bool irc::is_hostaddr(const std::string &value) { return (irc::is_ip4addr(value) || irc::is_ip6addr(value)); }

bool irc::is_ip4addr(const std::string &value)
{
  std::vector<std::string> splited;
  splited = strings_split(value, '.');

  for (std::vector<std::string>::const_iterator it = splited.begin(); it != splited.end(); it++)
  {
    if (it->length() > 3 || it->empty())
      return false;

    if (value.find_first_not_of("0123456789") != std::string::npos)
      return false;
  }

  return true;
}

bool is_special_ip6addr(const std::string &value)
{
  size_t i;
  if (value.length() < 19)
    return false;
  if (!(value.compare(0, 10, "0:0:0:0:0:")))
    return false;
  i = value.compare(10, 2, "0:") == '0' ? 12 : 15;
  if (!(i == 12 || value.compare(10, 5, "FFFF:")))
    return false;
  return irc::is_ip4addr(value.substr(i));
}

bool irc::is_ip6addr(const std::string &value)
{
  if (!(isxdigit(value.at(0)) && isxdigit(value.at(value.length() - 1))))
    return false;

  std::vector<std::string> splited;
  splited = strings_split(value, ':');
  if (splited.size() != 8)
    return is_special_ip6addr(value);
  for (std::vector<std::string>::const_iterator it = splited.begin(); it + 1 != splited.end(); it++)
  {
    if (it->empty())
      return false;
    if (value.find_first_not_of("0123456789ABCDEFabcdef") != std::string::npos)
      return is_special_ip6addr(value);
  }
  return true;
}

bool irc::is_nickname(const std::string &value)
{
  if (value.size() > 9 || value.empty())
  {
    return false;
  }

  if (!(is_special(value.at(0)) || isalpha(value.at(0))))
  {
    return false;
  }

  for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
  {
    if (isalnum(*it) || char_is_any(*it, "[]\\`_^{|}-"))
      continue;
    return false;
  }
  return true;
}

// The mask MUST
//         have at least 1 (one) "." in it and no wildcards following the last
//     ".".  This requirement exists to prevent people sending messages to
//     "#*" or "$*", which would broadcast to all users.  Wildcards are the
//         '*' and '?'  characters.  This extension to the PRIVMSG command is
//         only available to operators.

bool irc::is_mask(const std::string &value)
{
  size_t point;
  point = value.find_last_of('.');
  if (point == std::string::npos || char_is_any(value.at(point + 1), "*?"))
    return false;
  return true;
}

bool irc::is_targetmask(const std::string &value)
{
  return (string_starts_with_any(value, "$#") && irc::is_mask(value.substr(1)));
}

bool irc::is_chanstring(const std::string &value)
{
  for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
  {
    if (!(*it) || char_is_any(*it, "\a\r\n ,:"))
      return false;
  }
  return true;
}

bool irc::is_channel_id(const std::string &value)
{
  if (value.length() != 5)
    return false;
  for (int i = 0; i < 5; i++)
  {
    if (!char_is_any(value.at(i), "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
      return false;
  }
  return true;
}

bool irc::is_user(const std::string &value)
{
  for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
  {
    if (!(*it) || char_is_any(*it, "\r\n @"))
    {
      return false;
    }
  }
  return true;
}

bool irc::is_key(const std::string &value)
{
  for (std::string::const_iterator it = value.begin(); it != value.end(); it++)
  {
    if (!(*it) || char_is_any(*it, "\r\n\f\t\v "))
    {
      return false;
    }
  }
  return true;
}

bool irc::is_special(const char c) { return char_is_any(c, "[]\\`_^{|}"); }

std::string pad_string(const std::string &str, size_t n, char c)
{
  if (str.size() >= n)
    return str;
  std::string pad(n - str.size(), c);
  return pad + str;
}

std::string irc::Response::message() const { return m_message; }

std::string irc::Response::message(const std::string &server_prefix) const
{
  return ":" + server_prefix + " " + pad_string(to_string(m_code), 3, '0') + " " + m_message;
}

irc::Response::Response() : m_nil(true), m_code(0) {}

irc::Response::Response(uint16_t code, const std::string &message)
    : m_nil(false), m_message(message), m_code(code)
{
}

irc::Response::Response(uint16_t code, const std::string &param, const std::string &message)
    : m_nil(false), m_message(param + " " + message), m_code(code)
{
}

irc::Response::Response(uint16_t code, const std::string &p1, const std::string &p2,
                        const std::string &message)
    : m_nil(false), m_message(p1 + " " + p2 + " " + message), m_code(code)
{
}

bool irc::Response::nil() const { return m_nil; }
irc::Response::Response(uint16_t code, const std::string &p1, const std::string &p2, const std::string &p3,
                        const std::string &message)
    : m_nil(false), m_message(p1 + " " + p2 + " " + p3 + " " + message), m_code(code)
{
}
