#include "Koakoubot.h"
#include "Utils.h"

Koakoubot::Koakoubot() : m_server_addrData(NULL),  m_exit() {}

void Koakoubot::initKoakoubot(const std::string &addr, const std::string &port)
{
  m_server_addrData = setAddrInfo(addr.c_str(), port.c_str());
  m_server_sockfd =
      socket(m_server_addrData->ai_family, m_server_addrData->ai_socktype, m_server_addrData->ai_protocol);
  if (m_server_sockfd == -1)
  {
    std::cerr << "error socket" << std::endl;
    throw std::logic_error(strerror(errno));
  }
  if (connect(m_server_sockfd, m_server_addrData->ai_addr, m_server_addrData->ai_addrlen) == -1)
  {
    std::cerr << "error connect" << std::endl;
    throw std::logic_error(strerror(errno));
  }
  struct sockaddr_in local_addr;
  socklen_t addr_len = sizeof(local_addr);
  if (getsockname(m_server_sockfd, (struct sockaddr *)&local_addr, &addr_len) < 0)
  {
    std::cerr << "Error getting socket name" << std::endl;
    close(m_server_sockfd);
    return;
  }
  std::string localport = ftItoa(static_cast<int>(ntohs(local_addr.sin_port)));
  servConnect();
}

void Koakoubot::servConnect()
{
  sleep(3); // RM
  setNickname();
  sendMessageToServ(NICK_CMD(m_nickname));
  sleep(3); // RM
  sendMessageToServ(USER_CMD);
}

addrinfo *Koakoubot::setAddrInfo(const char *addr, const char *port)
{
  addrinfo hints, *result;
  int status;

  std::memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(addr, port, &hints, &result)) != 0)
  {
    std::cerr << "error get addr info" << std::endl;
    throw std::logic_error(gai_strerror(status));
  }
  return (result);
}

void Koakoubot::setNickname()
{
  std::srand(time(NULL));
  std::string s = ftItoa(std::rand());
  m_nickname = s.substr(s.length() - 5, 4);
}

int Koakoubot::getServerSockfd() const { return m_server_sockfd; }

const addrinfo &Koakoubot::getServerAddrInfo() const { return *(m_server_addrData); }

std::deque<std::string> &Koakoubot::getMsgQueue() { return m_msgQueue; }

void Koakoubot::sendMessageToServ(const std::string &message)
{
  std::cout << message << std::endl;
  ssize_t ret = send(m_server_sockfd, message.c_str(), message.size(), 0);
  check_errno(ret);
}

void Koakoubot::receiveMessageFromServ()
{
  std::vector<char> buffer(2048);
  ssize_t size = recv(m_server_sockfd, buffer.data(), buffer.size(), 0);
  check_errno(size);
  if (m_exit)
    return;
  m_buffer.reserve(size);
  m_buffer.insert(m_buffer.end(), buffer.begin(), buffer.begin() + size);

  while (createMessageFromBuffer())
    ;
}

bool Koakoubot::createMessageFromBuffer()
{
  for (std::vector<char>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
  {
    if (*it == '\r' && (it + 1) != m_buffer.end() && *(it + 1) == '\n')
    {
      std::string raw_message(m_buffer.begin(), it + 2);

      m_buffer.erase(m_buffer.begin(), it + 2);
      m_msgQueue.push_back(raw_message);
      return true;
    }
  }
  return false;
}

bool Koakoubot::getExit() { return m_exit; }

void Koakoubot::setExit(bool new_status) { m_exit = new_status; }

Koakoubot::~Koakoubot() { destroy(); }
void Koakoubot::destroy()
{
  if (m_server_addrData != NULL)
  {
    freeaddrinfo(m_server_addrData);
    m_server_addrData = NULL;
  }
  if (m_server_sockfd != -1)
  {
    close(m_server_sockfd);
    m_server_sockfd = -1;
  }
}
void Koakoubot::check_errno(ssize_t ret)
{
  if (ret <= 0 && errno != EAGAIN)
  {
    destroy();
    m_exit = true;
    std::cerr << "Socket Fatal Error..." << std::endl;
  }
}
