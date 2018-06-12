#include "tcp_server.h"

namespace cgvp {
namespace communicate{


TcpServer::TcpSession::TcpSession(TcpServer* tcp_server, id_t this_session, size_t max_size):
    tcp_server_(tcp_server),this_session_(this_session),max_size_(max_size),socket_(service_)
{
  ++(tcp_server_->session_count_);
  buffer_ = new char[max_size];
  error_flag_ = true;
}


TcpServer::TcpSession::~TcpSession() {
  delete buffer_;
  --(tcp_server_->session_count_);
}

bool TcpServer::TcpSession::Run() {
  DoRead();
  tcp_server_->NewConnect(this_session_);
  service_.run();
  return true;
}

void TcpServer::TcpSession::Stop() {
  error_flag_ = false;
  socket_.cancel(ignore_error_);
  socket_.close(ignore_error_);
}

void TcpServer::TcpSession::DoRead() {
  if (!socket_.is_open())
    return;
  socket_.async_read_some(boost::asio::buffer(buffer_, max_size_),
    std::bind(&TcpSession::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::TcpSession::OnRead(const std::error_code &error, size_t bytes) {
  if ((!socket_.is_open())||(!error_flag_))
    return ;
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    return DoRead();
  }
  if (bytes >= max_size_) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, "接收数据超限");
    return DoRead();
  }
  buffer_[bytes] = '\0';
  std::string str(buffer_);
  DoRead();
  tcp_server_->NewMessage(this_session_, str);
}

void TcpServer::TcpSession::DoWrite(char* write_body, size_t length) {
  std::cout << __FUNCTION__ << std::endl;//aaa
  if (!socket_.is_open())
    return;
  socket_.async_write_some(boost::asio::buffer(write_body, length),
    std::bind(&TcpServer::TcpSession::OnWrite, shared_from_this(),
      write_body, std::placeholders::_1, std::placeholders::_2));
}

void TcpServer::TcpSession::OnWrite(char* write_body, const std::error_code &error, size_t bytes) {
  delete[] write_body;
  if (!error_flag_ || !socket_.is_open())
  {
    return;
  }
  if (error) {
    return;
  }
  tcp_server_->SendSuccess();
  return;
}

std::string TcpServer::TcpSession::DirIp() {
  return socket_.remote_endpoint().address().to_string();
}

int32_t TcpServer::TcpSession::DirPort() {
  return socket_.remote_endpoint().port();
}

tcp::socket& TcpServer::TcpSession::socket() {
  return socket_;
}

TcpServer::id_t TcpServer::TcpSession::GetId() {
  return this_session_;
}

void TcpServer::TcpSession::BaseError(ErrorLevel level, const char* position, std::error_code error) {
  if (!error_flag_)
    return;
  tcp_server_->ErrorHandle(this_session_,level, position, error);
}

void TcpServer::TcpSession::BaseError(ErrorLevel level, const char* position, const char* error) {
  if (!error_flag_)
    return;
  tcp_server_->ErrorHandle(this_session_,level, position, error);
}

TcpServer::TcpServer(id_type id, int32_t port, TcpServerConfig& config) :
    Transport(id,port,Type::kTcpServer),config_(config)
{
  listening_ = false;
  session_count_ = 0;
  StartListen();
  session_id = 15;
}


TcpServer::~TcpServer() {
  StopListen();
}

void TcpServer::StartListen() {
  if (listening_) {
    return;
  }
  if (session_count_ != 0) {
    ErrorHandle(0,ErrorLevel::kFATAL, __FUNCTION__, "客户端未正确关闭或线程未正确退出");
  }
  service_.reset();
  acceptor_ = new tcp::acceptor(service_,
    tcp::endpoint(config_.ip_protocol == IpProtocol::IPV6 ? tcp::v6() : tcp::v4(), port_));
  DoAccept();
  std::thread running(std::bind(&TcpServer::RunThread, this));
  running.detach();
  listening_ = true;
}

void TcpServer::StopListen() {
  if (!listening_)
    return;
  listening_ = false;
  for (auto iter = session_map_.begin(); iter != session_map_.end();) {
    iter = DeleteTcpSession(iter->first);
  }

  acceptor_->cancel();
  acceptor_->close();
  delete acceptor_;
  session_map_.clear();
}

void TcpServer::DoAccept() {
  auto temp_session = std::make_shared<TcpSession>(this, ++session_id,config_.max_size);
  acceptor_->async_accept(temp_session->socket(),
    std::bind(&TcpServer::OnAccept, this, temp_session,std::placeholders::_1));
  
}
void TcpServer::OnAccept(std::shared_ptr<TcpSession> new_session, const std::error_code& error) {
  std::cout << "444" << std::endl;//aaa
  if (!(acceptor_->is_open()))
    return;
  if (!listening_)
    return;
  if (error) {
    ErrorHandle(0,ErrorLevel::kSERIOUS, __FUNCTION__, error);
  }
  session_map_[new_session->GetId()] = new_session;
  std::thread temp(std::bind(&TcpSession::Run, new_session));
  temp.detach();
  DoAccept();

}

void TcpServer::Send(std::string send_ip, int32_t send_port, const char* send_str) {
  id_t id = FindTcpSession(send_ip, send_port);
  if (id == 0) {
    ErrorHandle(0,ErrorLevel::kERROR, __FUNCTION__, "不存在的客户端");
    return;
  }
  SendTo(id, std::string(send_str));
}

void TcpServer::Send(std::string send_ip, int32_t send_port, std::string& send_str) {
  auto id = FindTcpSession(send_ip, send_port);
  if (id == 0) {
    ErrorHandle(0,ErrorLevel::kERROR, __FUNCTION__, "不存在的客户端");
    return;
  }
  SendTo(id, send_str);
}
void TcpServer::DisConnect(std::string ip, int32_t port) {
  auto id = FindTcpSession(ip, port);
  if (id == 0)
    return;
  DeleteTcpSession(id);
}

void TcpServer::SetConnectHandle(const std::function<void(std::string&, int32_t, std::function<void(std::string&)>)> connect_handle) {
  connect_handle_ = connect_handle;
}
void TcpServer::SetNewMessageHandle(const std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)>& new_message_handle) {
  new_message_handle_ = new_message_handle;
}
void TcpServer::SetSendHandle(const std::function<void()>& send_handle) {
  send_handle_ = send_handle;
}
void TcpServer::SetErrorHandle(const std::function<void(const char*, const char*)>& error_handle) {
  error_handle_ = error_handle;
}

size_t TcpServer::clients() {
  return session_map_.size();
}

int32_t TcpServer::port() {
  if (listening_) {
    return acceptor_->local_endpoint().port();
  }
  else return port_;
}

void TcpServer::RunThread() {
  service_.run(tcp_error_);
  if (tcp_error_) {
    ErrorHandle(0,ErrorLevel::kFATAL, __FUNCTION__, tcp_error_);
    tcp_error_.clear();
  }
}

void TcpServer::ReSet(TcpServerConfig& config) {
  config_ = config;
}

void TcpServer::SendTo(id_t id, std::string& send_str) {
  size_t length = send_str.size();
  char* send_body = new char[length + 10];
  strcpy(send_body, send_str.c_str());
  session_map_[id]->DoWrite(send_body, length);
  
}

void TcpServer::NewConnect(id_t session_id) {
  connect_handle_(session_map_[session_id]->DirIp(), session_map_[session_id]->DirPort(),
    std::bind(&TcpServer::SendTo, this, session_id, std::placeholders::_1)
  );
}

void TcpServer::SendSuccess() {
  send_handle_();
}

void TcpServer::NewMessage(id_t session_id, std::string& message) {
  new_message_handle_(session_map_[session_id]->DirIp(), session_map_[session_id]->DirPort(), message,
    std::bind(&TcpServer::SendTo, this, session_id, std::placeholders::_1)
  );
}

TcpServer::id_t TcpServer::FindTcpSession(std::string& ip, int32_t port) {
  for (auto iter = session_map_.begin(); iter != session_map_.end(); ++iter) {
    if ((iter->second->DirIp() == ip) && (iter->second->DirPort() == port)){
      return iter->first;
    }
  }
  return 0;
}

std::unordered_map<TcpServer::id_t, std::shared_ptr<TcpServer::TcpSession> >::iterator TcpServer::DeleteTcpSession(id_t session_id) {
  session_map_[session_id]->Stop();
  session_map_[session_id].reset();
  auto iter = session_map_.find(session_id);
  return session_map_.erase(iter);
}

void TcpServer::ErrorHandle(id_t session, ErrorLevel level, const char* position, std::error_code error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
    if(session!=0)
    this->DeleteTcpSession(session);
    break;
  }
  case ErrorLevel::kSERIOUS: {
    StopListen();
    break;
  }
  case ErrorLevel::kFATAL: {
    std::cout << "发生严重错误，建议重新启动整个系统" << std::endl;
    break;
  }
  }
  error_handle_(position, error.message().c_str());
  //记录日志
}

void TcpServer::ErrorHandle(id_t session, ErrorLevel level, const char* position, const char* error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
    if (session != 0)
    this->DeleteTcpSession(session);
    break;
  }
  case ErrorLevel::kSERIOUS: {
    StopListen();
    break;
  }
  case ErrorLevel::kFATAL: {
    std::cout << "发生严重错误，建议重新启动整个系统" << std::endl;
    break;
  }
  }
  error_handle_(position, error);
  //记录日志
}

} // communicate
} // cgvp
