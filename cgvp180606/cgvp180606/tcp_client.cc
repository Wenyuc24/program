#include "tcp_client.h"

namespace cgvp {
namespace communicate{

TcpClient::BaseTcpClient::BaseTcpClient(TcpClient* tcp_client,
    boost::asio::io_service& service, size_t max_size) :tcp_client_(tcp_client),
    socket_(service), max_size_(max_size){
  error_flag_ = true;
  tcp_client_->base_flag_ = true;
  buffer_ = new char[max_size_];
}

TcpClient::BaseTcpClient::~BaseTcpClient() {
  delete[] buffer_;
  tcp_client_->base_flag_ = false;
}

bool TcpClient::BaseTcpClient::Run(int protocol, tcp::endpoint& dir_endpoint) {
  if (protocol == 6) {
    socket_.open(tcp::v6());
    socket_.set_option(tcp::socket::reuse_address(true));
    socket_.bind(tcp::endpoint(boost::asio::ip::address::from_string("0::0"), 0), base_error_);
    if (base_error_) {
      base_error_.clear();
      return false;
    }
  }
  else {
    socket_.open(tcp::v4());
    socket_.set_option(tcp::socket::reuse_address(true));
    socket_.bind(tcp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), 0), base_error_);
    if (base_error_) {
      base_error_.clear();
      return false;
    }
  }
  DoConnect(dir_endpoint);
  return true;
}
void TcpClient::BaseTcpClient::Stop() {
  error_flag_ = false;
  socket_.cancel();
  socket_.close();
}

void TcpClient::BaseTcpClient::DoConnect(tcp::endpoint& endpoint) {
  std::cout << "DoConnect" << std::endl;//aaa
  if (!socket_.is_open())
    return;
  socket_.async_connect(endpoint, 
      std::bind(&TcpClient::BaseTcpClient::OnConnect, shared_from_this(), std::placeholders::_1));
}
void TcpClient::BaseTcpClient::OnConnect(const std::error_code &error) {
  if (error) {
    BaseError(ErrorLevel::kSERIOUS, __FUNCTION__, error);
    return;
  }
  DoRead();
}

void TcpClient::BaseTcpClient::DoRead() {
  if (!socket_.is_open())
    return;
  socket_.async_read_some(boost::asio::buffer(buffer_, max_size_),
      std::bind(&TcpClient::BaseTcpClient::OnRead, shared_from_this(),
      std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::BaseTcpClient::OnRead(const std::error_code &error, size_t bytes) {
  if (!socket_.is_open())
    return;
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    return;
  }
  if (bytes >= max_size_) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, "接收数据超限");
    return;
  }
  buffer_[bytes] = '\0';
  tcp_client_->ReadMessage(buffer_);
  DoRead();
}

void TcpClient::BaseTcpClient::DoWrite(char* write_body, size_t length) {
  if (!socket_.is_open())
    return;
  socket_.async_write_some(boost::asio::buffer(write_body, length), 
      std::bind(&TcpClient::BaseTcpClient::OnWrite, shared_from_this(), 
          write_body, std::placeholders::_1, std::placeholders::_2));
}

void TcpClient::BaseTcpClient::OnWrite(char* write_body, const std::error_code &error, size_t bytes) {
  std::cout << __FUNCTION__ << std::endl;//aaa
  delete[] write_body;
  if (!error_flag_ || !socket_.is_open())
  {
    return;
  }
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    return;
  }
  tcp_client_->WriteSuccess();
  return;
}

int32_t TcpClient::BaseTcpClient::GetPort() {
  return socket_.local_endpoint().port();
}

void TcpClient::BaseTcpClient::BaseError(ErrorLevel level, const char* position, std::error_code error) {
  if (!error_flag_)
    return;
  tcp_client_->ErrorHandle(level, position, error);
}

void TcpClient::BaseTcpClient::BaseError(ErrorLevel level, const char* position, const char* error) {
  if (!error_flag_)
    return;
  tcp_client_->ErrorHandle(level, position, error);
}

TcpClient::TcpClient(id_type id, TcpClientConfig& config) :
    Transport(id, 0, Type::kTcpClient), config_(config), timer_(service_){
  connect_ = false;
  bind_flag_ = false;
  base_flag_ = false;
  threads_flag_ = false;
}


TcpClient::~TcpClient() {
  Disconnect();
  if (base_flag_ || threads_flag_)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void TcpClient::Connect() {
  if (connect_)
  {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "客户端已经在连接状态");
    return;
  }
  if (base_flag_ ||threads_flag_)
  {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, "连接未正常断开或线程未正常退出");
    return;
  }
  if (!bind_flag_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "未绑定目的地址");
    return;
  }
  service_.reset();
  base_tcp_client_ = std::make_shared<BaseTcpClient>(this, service_, config_.max_size);
  
  connect_ = true;
  if (!base_tcp_client_->Run(config_.ip_protocol == IpProtocol::IPV6 ? 6 : 4, dir_endpoint_)) {
    ErrorHandle(ErrorLevel::kSERIOUS, __FUNCTION__, "端口绑定失败");
    return;
  }
  std::thread run_thread(std::bind(&TcpClient::RunThread, this));
  run_thread.detach();
  if (config_.send_ping) {
    time(&last_time_);
    timer_.expires_from_now(boost::posix_time::milliseconds(config_.ping_time * 1000));
    timer_.async_wait(std::bind(&TcpClient::SendPing, this));
  }
}

void TcpClient::Disconnect() {
  if (!connect_)
    return;
  connect_ = false;
  base_tcp_client_->Stop();
  base_tcp_client_.reset();
  base_tcp_client_ = nullptr;
  timer_.cancel(ignore_error_);
}

void TcpClient::BindDir(const char* ip, int32_t port) {
  if ((port > 65535) || (port<1)) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "端口不合法");
    return;
  }
  auto bind_address = address::from_string(ip, tcp_error_);
  if (tcp_error_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, tcp_error_);
    tcp_error_.clear();
    return;
  }
  dir_endpoint_ = tcp::endpoint(bind_address, port);
  bind_flag_ = true;
}
void TcpClient::BindDir(std::string& ip, int32_t port) {
  BindDir(ip.c_str(), port);
}
void TcpClient::UnBind() {
  bind_flag_ = false;
}

void TcpClient::Send(const char* send_str) {
  if (!connect_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "未连接");
    return;
  }
  size_t length = strlen(send_str);
  char* send_body = new char[length + 10];
  strcpy_s(send_body, length + 1, send_str);
  base_tcp_client_->DoWrite(send_body, length);
  if (config_.send_ping) {
    time(&last_time_);
    timer_.expires_from_now(boost::posix_time::milliseconds(config_.ping_time * 1000));
    timer_.async_wait(std::bind(&TcpClient::SendPing, this));
  }
  return;
}
void TcpClient::Send(std::string& send_str) {
  Send(send_str.c_str());
}

void TcpClient::SendPing() {
  time(&now_time_);
  if (now_time_ - last_time_ >= config_.ping_time) {
    Send(config_.ping_body);
  }
}

std::string TcpClient::DirIp() {
  return dir_endpoint_.address().to_string();
}
int32_t TcpClient::DirPort() {
  return dir_endpoint_.port();
}

int32_t TcpClient::port() {
  if (connect_) 
    return base_tcp_client_->GetPort();
  else return port_;
}

bool TcpClient::IsConnect() {
  return connect_;
}

void TcpClient::RunThread() {
  //std::cout << "xiancheng" << std::endl;
  threads_flag_ = true;
  service_.run(tcp_error_);
  if (tcp_error_) {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, tcp_error_);
    tcp_error_.clear();
  }
  threads_flag_ = false;
}

void TcpClient::ReSet(TcpClientConfig& config) {
  config_ = config;
}

void TcpClient::ConnectSuccess() {
  std::cout << "connect success" << std::endl;
}

void TcpClient::ReadMessage(char* message) {
  std::cout << message << std::endl;
}

void TcpClient::WriteSuccess() {
  std::cout << "send ok" << std::endl;
}
void TcpClient::ErrorHandle(ErrorLevel level, const char* position, std::error_code error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
    std::cout << position << error.message() << std::endl;
    break;
  }
  case ErrorLevel::kSERIOUS: {
    std::cout << position << error.message() << std::endl;
    Disconnect();
    break;
  }
  case ErrorLevel::kFATAL: {
    std::cout << position << error.message() << std::endl;
    std::cout << "发生严重错误，建议重新启动整个系统" << std::endl;
    break;
  }
  }
  //记录日志
}
void TcpClient::ErrorHandle(ErrorLevel level, const char* position, const char* error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
    std::cout << position << error << std::endl;
    break;
  }
  case ErrorLevel::kSERIOUS: {
    std::cout << position << error << std::endl;
    Disconnect();
    break;
  }
  case ErrorLevel::kFATAL: {
    std::cout << position << error << std::endl;
    std::cout << "发生严重错误，建议重新启动整个系统" << std::endl;
    break;
  }
  }
  //记录日志
}

} // communicate
} // cgvp