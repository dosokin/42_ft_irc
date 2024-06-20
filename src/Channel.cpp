#include "Channel.h"

Channel::Channel(const std::string &name, const std::string &key)
    : m_name(name),
      m_key(key),
      m_client_counter(0),
      m_client_limit(UNLIMITED_CLIENTS),
      m_mode(true, false, !key.empty(), false)
{
}

Channel::Channel(const std::string &name)
    : m_name(name), m_client_counter(0), m_client_limit(UNLIMITED_CLIENTS), m_mode(true, false, false, false)
{
}

Channel::~Channel() {}

/* Getters/Setters */

void Channel::setTopic(const std::string &t_topic) { m_topic = t_topic; }
const std::string &Channel::getTopic() const { return m_topic; }
std::string Channel::getSafeTopic() const
{
  if (m_topic.find_first_of("\n\r\t \v") != std::string::npos)
    return ":" + m_topic;
  return m_topic;
}

const std::string &Channel::getName() const { return m_name; }
int64_t Channel::getClientCounter() const { return m_client_counter; }

void Channel::setKey(const std::string &key)
{
  m_key = key;
  m_mode.has_key = (!key.empty());
}
const std::string &Channel::getKey() const { return m_key; }
bool Channel::hasKey() const { return m_mode.has_key; }

void Channel::setInviteOnly(bool invite_only) { m_mode.is_invite_only = invite_only; }
bool Channel::isInviteOnly() const { return m_mode.is_invite_only; }

void Channel::setTopicRestricted(bool restricted) { m_mode.has_topic_restrict = restricted; }
bool Channel::isTopicRestricted() const { return m_mode.has_topic_restrict; }

void Channel::setClientLimit(int64_t limit)
{
  m_client_limit = limit;
  m_mode.has_limit = (limit != UNLIMITED_CLIENTS);
}
int64_t Channel::getClientLimit() const { return m_client_limit; }

/* Channel Logic */

bool Channel::addClient(Client *client)
{
  if (clientInChannel(client))
    return false;
  m_client_map.insert(std::make_pair(client->getId(), client));
  if (!client->getIsBot())
    m_client_counter++;
  return true;
}

bool Channel::removeClient(Client *client)
{
  std::map<uint32_t, Client *>::const_iterator it = m_client_map.find(client->getId());
  if (it != m_client_map.end())
  {
    m_client_map.erase(it->first);
    removeInvite(client->getId());
    removeOperator(client);
    if (!client->getIsBot())
      m_client_counter--;
    return true;
  }
  return false;
}

bool Channel::broadcast(const std::string &message, const Client *ignore)
{
  for (std::map<uint32_t, Client *>::iterator it = m_client_map.begin(); it != m_client_map.end(); ++it)
  {
    if (ignore != NULL && it->first == ignore->getId())
      continue;
    it->second->broadcast(message);
  }
  return true;
}

bool Channel::clientInChannel(const Client *client) const { return clientInChannel(client->getId()); }
bool Channel::clientInChannel(uint32_t client_id) const
{
  return m_client_map.find(client_id) != m_client_map.end();
}

bool Channel::clientIsOperator(uint32_t client_id) const
{
  return m_operators.find(client_id) != m_operators.end();
}

void Channel::addOperator(Client *op) { this->m_operators.insert(op->getId()); }
void Channel::removeOperator(Client *op) { this->m_operators.erase(op->getId()); }
void Channel::pushInvite(uint32_t client_id) { m_invited.insert(client_id); }
bool Channel::clientIsInvited(uint32_t client_id) { return m_invited.find(client_id) != m_invited.end(); }
void Channel::removeInvite(uint32_t client_id) {
  std::set<uint32_t>::iterator it = m_invited.find(client_id);

  if (it != m_invited.end())
    m_invited.erase(it);
}

std::string Channel::namesList()
{
  std::vector<std::string> names;

  for (std::map<uint32_t, Client *>::iterator it = m_client_map.begin(); it != m_client_map.end(); ++it)
  {
    if (this->clientIsOperator(it->first))
      names.push_back("@" + it->second->getNickname());
    else
      names.push_back(it->second->getNickname());
  }

  return irc::strings_join(names, " ");
}

std::string Channel::modeString() const
{
  std::vector<std::string> modes;
  std::vector<std::string> modes_params;

  if (m_mode.has_topic_restrict)
    modes.push_back("t");
  if (m_mode.is_invite_only)
    modes.push_back("i");
  if (m_mode.has_key)
  {
    modes.push_back("k");
    modes_params.push_back(getKey());
  }
  if (m_mode.has_limit)
  {
    modes.push_back("l");
    modes_params.push_back(to_string(getClientLimit()));
  }

  if (modes.empty())
    return "";

  return "+" + irc::strings_join(modes, "") + (modes_params.empty() ? "" : " ") +
         irc::strings_join(modes_params, " ");
}
