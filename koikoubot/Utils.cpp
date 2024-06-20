#include "Utils.h"

std::vector<std::string> strings_split(const std::string &s, char c)
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

std::string ftItoa(int i)
{
  std::ostringstream convert;
  convert << i;
  std::string s = convert.str();
  return (s);
}