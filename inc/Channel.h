#ifndef CHANNEL_H
#define CHANNEL_H

#include <map>
#include <set>
#include <string>

#include "Client.h"
#include "irc.h"

class Channel
{
public:
  static const int64_t UNLIMITED_CLIENTS = -1;

public:
  Channel(const std::string &name, const std::string &key);
  Channel(const std::string &name);
  ~Channel();

public:
  void setTopic(const std::string &t_topic);
  const std::string &getTopic() const;
  std::string getSafeTopic() const;

  const std::string &getName() const;
  int64_t getClientCounter() const;

  void setKey(const std::string &key);
  const std::string &getKey() const;
  bool hasKey() const;

  void setInviteOnly(bool invite_only);
  bool isInviteOnly() const;

  void setTopicRestricted(bool restricted);
  bool isTopicRestricted() const;

  void setClientLimit(int64_t limit);
  int64_t getClientLimit() const;

public:
  bool addClient(Client *client);
  bool removeClient(Client *client);
  bool broadcast(const std::string &message, const Client *ignore = NULL);

  bool clientInChannel(const Client *client) const;
  bool clientInChannel(uint32_t client_id) const;
  bool clientIsOperator(uint32_t client_id) const;

  void pushInvite(uint32_t client_id);
  void removeInvite(uint32_t client_id);
  bool clientIsInvited(uint32_t client_id);

  void addOperator(Client *op);
  void removeOperator(Client *op);
  std::string namesList();
  std::string modeString() const;

private:
  struct Mode
  {
  public:
    Mode(bool topic_restrict, bool limit, bool key, bool invite_only)
        : has_topic_restrict(topic_restrict), has_limit(limit), has_key(key), is_invite_only(invite_only)
    {
    }
    bool has_topic_restrict : 1;
    bool has_limit : 1;
    bool has_key : 1;
    bool is_invite_only : 1;
  };

  std::string m_name;
  std::string m_key;
  std::string m_topic;
  int64_t m_client_counter;
  int64_t m_client_limit;
  std::set<uint32_t> m_invited;
  std::set<uint32_t> m_operators;
  std::map<uint32_t, Client *> m_client_map;
  Mode m_mode;
};

#endif
