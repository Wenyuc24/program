#include "http_server.h"

namespace cgvp {
namespace communicate{

HttpServer::HttpSession::HttpSession(HttpServer* http_server, boost::asio::io_service& super_http_service, size_t max_size):
    http_server_(http_server),super_http_service_(super_http_service),socket_(service_),max_size_(max_size)
{
  buffer_ = new char[max_size];
  error_flag_ = true;
}

void HttpServer::HttpSession::Run(Session_iter this_session) {
  this_session_ = this_session;
  DoRead();
  std::thread runs(
    [this] {
    this->service_.run();
  }
  );
  runs.detach();
}

void HttpServer::HttpSession::Stop() {
  error_flag_ = false;
}

void HttpServer::HttpSession::DoRead() {
  if (!socket_.is_open())
    return;
  socket_.async_read_some(boost::asio::buffer(buffer_, max_size_), std::bind(&HttpSession::OnRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
}

void HttpServer::HttpSession::OnRead(const std::error_code &error, size_t bytes) {
  if ((!socket_.is_open())||!error_flag_)
    return;
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    DeleteThis();
    return;
  }
  if (bytes >= max_size_) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, "接收数据超限");
    DeleteThis();
    return;
  }
  buffer_[bytes] = '\0';
  char *req_string = new char[bytes + 10];
  strcpy(req_string, buffer_);
  http::Request *req = new http::Request(req_string, bytes);
  http::Respond *res = new http::Respond;
  res->KeepAlive(req->KeepAlive());
  http_server_->NewRequest(*req, *res);
  socket_.async_write_some(res->to_buffers(), std::bind(&HttpSession::OnWrite, shared_from_this(), res, std::placeholders::_1, std::placeholders::_2));
}
void HttpServer::HttpSession::OnWrite(http::Respond* res, const std::error_code &error, size_t bytes) {
  bool flag = res->KeepAlive();
  delete res;
  if (!flag)
    DeleteThis();
  else DoRead();
}

tcp::socket& HttpServer::HttpSession::socket() {
  return socket_;
}

void HttpServer::HttpSession::DeleteThis() {
  super_http_service_.post(std::bind(&HttpServer::DeleteSession, http_server_, this_session_));
}

void HttpServer::HttpSession::BaseError(ErrorLevel level, const char* position, std::error_code error) {
  if (!error_flag_)
    return;
  http_server_->ErrorHandle(level, position, error);
}

void HttpServer::HttpSession::BaseError(ErrorLevel level, const char* position, const char* error) {
  if (!error_flag_)
    return;
  http_server_->ErrorHandle(level, position, error);
}

HttpServer::HttpServer(id_type id, int32_t port, HttpServerConfig& config):
    Transport(id,port,Type::kHttpServer),config_(config)
{
  listening_ = false;
  session_count_ = 0;
  StartListen();
}

void HttpServer::StartListen() {
  if (listening_) {
    return;
  }
  listening_ = true;
  if (session_count_ != 0) {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, "客户端未正确关闭或线程未正确退出");
  }
  service_.reset();
  acceptor_ = new tcp::acceptor(service_,
    tcp::endpoint(config_.ip_protocol == IpProtocol::IPV6 ? tcp::v6() : tcp::v4(), port_));
  DoAccept();
  std::thread run_thread(std::bind(&HttpServer::RunThread, this));
  run_thread.detach();
}

void HttpServer::StopListen() {
  if (!listening_)
    return;
  listening_ = false;
  acceptor_->cancel();
  acceptor_->close();
  delete acceptor_;
}

void HttpServer::DoAccept() {
  auto temp_session = std::make_shared<HttpSession>(this, service_,config_.max_size);
  acceptor_->async_accept(temp_session->socket(),
    std::bind(&HttpServer::OnAccept, this, temp_session, std::placeholders::_1));
}

void HttpServer::OnAccept(std::shared_ptr<HttpSession> new_session, const std::error_code& error) {
  if (!listening_)
    return;
  DoAccept();
  if (error) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, error);
  }
  Session_iter x = Session_list_.insert(Session_list_.end(), new_session);
  new_session->Run(x);
}

void HttpServer::SetReadHandle(const std::function<void(const std::string&, const http::Request&, http::Respond&)>& read_handle) {
  read_handle_ = read_handle;
}

void HttpServer::SetError(const std::function<void(const char*, const char*)>& error_handle) {
  error_handle_ = error_handle;
}

void HttpServer::RunThread() {
  service_.run(http_error_);
  if (http_error_) {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, http_error_);
    http_error_.clear();
  }
}

HttpServer::Session_iter HttpServer::DeleteSession(Session_iter session){
  if (session != Session_list_.end()) {
    (*session)->Stop();
    return Session_list_.erase(session);
  }
  return Session_list_.end();
}

void HttpServer::NewRequest(http::Request& req, http::Respond& res) {
  read_handle_(config_.dor_root, req, res);
}

int32_t HttpServer::port() {
  if (listening_) {
    return acceptor_->local_endpoint().port();
  }
  else return port_;
}

void HttpServer::ErrorHandle(ErrorLevel level, const char* position, std::error_code error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: break;
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
void HttpServer::ErrorHandle(ErrorLevel level, const char* position, const char* error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
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