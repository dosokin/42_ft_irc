#include "Logger.h"
#include <TcpConn.h>
#include <cerrno>

#define socket_errno_ok() (errno == EWOULDBLOCK || errno == EAGAIN)
#define check_socket_error(ret, operation)                                                                   \
  if (ret <= 0 && !socket_errno_ok())                                                                        \
  {                                                                                                          \
    IRC_LOG_ERROR("Connection " << m_id << " " << operation << " Error: Dropping Connection...");            \
    drop();                                                                                                  \
    return;                                                                                                  \
  }

// TcpConn::TcpConn(const std::string addr, const int port, std::deque<Message> &msgQueue)
//     : m_active(false), m_buffer(), m_msgQueue(msgQueue)
//{
//   m_socket.fd = socket(AF_INET, SOCK_STREAM, 0); // AF_UNSPEC
//   if (m_socket.fd < 0)
//     throw std::runtime_error("Socket Creation Failed..."); // Handle error
//
//   m_socket.addr.sin_family = AF_INET; // AF_UNSPEC
//   m_socket.addr.sin_port = htons(port);
//   m_socket.addr.sin_addr.s_addr = inet_addr(addr.c_str());
//
//   if (connect(m_socket.fd, reinterpret_cast<sockaddr *>(&m_socket.addr), sizeof(m_socket.addr)) < 0)
//     throw std::runtime_error("Connection failed..."); // Handle error
//   m_active = true;
// }

TcpConn::TcpConn(uint32_t id, const socket_info &sockInfo, std::deque<Message> &msgQueue, int epollfd)
    : m_id(id), m_active(true), m_epollfd(epollfd), m_buffer(), m_msgQueue(msgQueue), m_outMsgQueue()
{
  m_socket = sockInfo;
}

TcpConn::~TcpConn()
{
  if (m_active)
    drop();
}

void TcpConn::processEvent(struct epoll_event &ev)
{
  if (ev.events & EPOLLIN)
    readRemainingMessages();
  if (ev.events & EPOLLOUT)
    sendRemainingMessages();
}

void TcpConn::readRemainingMessages()
{
  std::vector<uint8_t> buff(RECV_MAX_SIZE);
  const ssize_t ret = recv(m_socket.fd, buff.data(), buff.size(), 0);
  check_socket_error(ret, "Read") m_buffer.reserve(ret);
  m_buffer.insert(m_buffer.end(), buff.begin(), buff.begin() + ret);
  createMessagesFromBuffer();
}

void TcpConn::sendRemainingMessages()
{
  while (!m_outMsgQueue.empty())
  {
    ssize_t ret = send(m_socket.fd, m_outMsgQueue.front().data(), m_outMsgQueue.front().size(), MSG_NOSIGNAL);
    check_socket_error(ret, "Send")

        if (ret == static_cast<ssize_t>(m_outMsgQueue.front().size()))
    {
      m_outMsgQueue.pop_front();
      continue;
    }

    m_outMsgQueue.front().erase(0, ret);
    return;
  }
  enableEpolloutTracking(false);
}

void TcpConn::sendMsg(const std::string &msg)
{
  if (!m_active)
    return;
  std::string new_msg = msg + "\r\n";

  if (!m_outMsgQueue.empty())
  {
    IRC_LOG_TRACE("Send Buffer Busy -> Message: Added To Outgoing Queue!");
    m_outMsgQueue.push_back(new_msg);
    return;
  }

  IRC_LOG_TRACE("Attempting To Send Message ...");
  ssize_t ret = send(m_socket.fd, new_msg.c_str(), new_msg.size(), MSG_NOSIGNAL);
  check_socket_error(ret, "Send") if (ret == static_cast<ssize_t>(new_msg.size()))
  {
    IRC_LOG_INFO("Entire Message Successfully Sent!");
    return;
  }

  IRC_LOG_TRACE("Message Partially Sent -> Adding Rest To Outgoing Queue");
  new_msg.erase(0, ret);
  m_outMsgQueue.push_back(new_msg);
  enableEpolloutTracking(true);
}

void TcpConn::drop()
{
  if (!m_active)
    return;
  m_active = false;
  close(m_socket.fd);
  if (m_epollfd != -1)
    epoll_ctl(m_epollfd, EPOLL_CTL_DEL, m_socket.fd, NULL);
  IRC_LOG_TRACE("closed connection " << m_id);
}

bool TcpConn::isActive() const { return m_active; }

bool TcpConn::createMessageFromBuffer()
{
  for (std::vector<uint8_t>::iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
  {
    if (*it == '\r' && (it + 1) != m_buffer.end() && *(it + 1) == '\n')
    {
      std::string raw_message(m_buffer.begin(), it);

      IRC_LOG_TRACE("received raw message: '" << raw_message << "'");

      Message msg = Message::parse(raw_message);

      m_buffer.erase(m_buffer.begin(), it + 2);
      msg.setId(m_id);
      m_msgQueue.push_back(msg);
      return true;
    }
  }
  return false;
}

void TcpConn::createMessagesFromBuffer()
{
  while (createMessageFromBuffer())
    ;
}
void TcpConn::enableEpolloutTracking(bool value)
{
  struct epoll_event ev;
  ev.data.u32 = m_id;
  ev.events = EPOLLIN;
  if (value)
    ev.events |= EPOLLOUT;

  if (epoll_ctl(m_epollfd, EPOLL_CTL_MOD, m_socket.fd, &ev) == -1)
  {
    IRC_LOG_ERROR("Connection " << m_id << " epoll_ctl failed");
    return;
  }
  if (value)
    IRC_LOG_TRACE("Connection " << m_id << ": EPOLLOUT Tracking Enabled!");
  else
    IRC_LOG_TRACE("Connection " << m_id << ": EPOLLOUT Tracking Disabled!");
}

void setNonBlocking(int socketfd) { fcntl(socketfd, F_SETFL, O_NONBLOCK); }
