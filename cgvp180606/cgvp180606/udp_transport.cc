#include "udp_transport.h"

namespace cgvp {
namespace communicate{

UdpTransport::BaseUdp::BaseUdp(UdpTransport* udp_transport,
    boost::asio::io_service& service, size_t max_size):
    udp_transport_(udp_transport), socket_(service), max_size_(max_size),
    service_(service){
  error_flag_ = true;
  udp_transport_->base_flag_ = true;
  buffer_ = new char[max_size];
}

UdpTransport::BaseUdp::~BaseUdp() {
  delete buffer_;
  udp_transport_->base_flag_ = false;
}

bool UdpTransport::BaseUdp::Open(int protocol, int32_t port) {
  if (protocol == 6) {
    socket_.open(udp::v6());
    socket_.set_option(udp::socket::reuse_address(true));
    socket_.bind(udp::endpoint(boost::asio::ip::address::from_string("0::0"), port), base_error_);
    if (base_error_) {
      base_error_.clear();
      return false;
    }
  }
  else {
    socket_.open(udp::v4());
    socket_.set_option(udp::socket::reuse_address(true));
    socket_.bind(udp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), port), base_error_);
    if (base_error_) {
      base_error_.clear();
      return false;
    }
  }
  DoReceive();
  return true;
}

void UdpTransport::BaseUdp::Close() {
  error_flag_ = false;
  socket_.cancel();
  socket_.close();
}

void UdpTransport::BaseUdp::DoReceive() {
  if (!socket_.is_open())
    return;
  udp::endpoint* tempendpoint = new udp::endpoint();
  socket_.async_receive_from(boost::asio::buffer(buffer_, max_size_),*tempendpoint,std::bind(&UdpTransport::BaseUdp::OnReceive, 
          shared_from_this(), tempendpoint,std::placeholders::_1, std::placeholders::_2));
}


void UdpTransport::BaseUdp::OnReceive(udp::endpoint* receive_endpoint,const std::error_code &error, size_t bytes) {
  if (!error_flag_ || !socket_.is_open())
  {
    delete receive_endpoint;
    return;
  }
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    return DoReceive();
  }
  if (bytes >= max_size_) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, "接收数据超限");
    return DoReceive();
  }
  buffer_[bytes] = '\0';
  std::string str(buffer_);
  DoReceive();
  udp_transport_->ReceiveMessage(receive_endpoint,str);
  delete receive_endpoint;
}


void UdpTransport::BaseUdp::DoSend(udp::endpoint* send_endpoint, char* send_str, size_t length) {
  if (!socket_.is_open())
    return;
  socket_.async_send_to(boost::asio::buffer(send_str, length), *send_endpoint,
      std::bind(&UdpTransport::BaseUdp::OnSend, shared_from_this(), send_endpoint,
          send_str, std::placeholders::_1, std::placeholders::_2));
}


void UdpTransport::BaseUdp::OnSend(udp::endpoint* send_endpoint, char* send_body,
    const std::error_code &error, size_t bytes) {
  if (!error_flag_ || !socket_.is_open())
  {
    delete send_endpoint;
    delete[] send_body;
    return;
  }
  if (error) {
    BaseError(ErrorLevel::kERROR, __FUNCTION__, error);
    return;
  }
  udp_transport_->SendSuccessed();
  delete send_endpoint;
  delete[] send_body;
  return;
}

int32_t UdpTransport::BaseUdp::GetPort() {
  return socket_.local_endpoint().port();
}

void UdpTransport::BaseUdp::BaseError(ErrorLevel level, const char* position, std::error_code error) {
  if (!error_flag_)
    return;
  service_.post(std::bind(static_cast<void(UdpTransport::*)(ErrorLevel,const char*,std::error_code)>(&UdpTransport::ErrorHandle), udp_transport_, level, position, error));
  //udp_transport_->ErrorHandle(level, position, error);
}

void UdpTransport::BaseUdp::BaseError(ErrorLevel level, const char* position, const char* error) {
  if (!error_flag_)
    return;
  service_.post(std::bind(static_cast<void(UdpTransport::*)(ErrorLevel, const char*, const char*)>(&UdpTransport::ErrorHandle), udp_transport_, level, position, error));
  //udp_transport_->ErrorHandle(level, position, error);
}

UdpTransport::UdpTransport(id_type id, int32_t port, UdpConfig& config) :
    Transport(id, port, Type::kUdp), config_(config), base_udp_(nullptr){
  start_ = false;
  bind_flag_ = false;
  base_flag_ = false;
  thread_count_ = 0;
  Start();
}

UdpTransport::~UdpTransport() {
  Stop();
  if (base_flag_ || (thread_count_ > 0))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void UdpTransport::Start(){
  if (start_)
  {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "尝试打开一个打开着的UDP");
    return;
  }
  if (base_flag_ || (thread_count_ > 0))
  {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, "会话未正常析构或线程未正常退出");
    return;
  }
  start_ = true;
  service_.reset();
  base_udp_ = std::make_shared<BaseUdp>(this, service_, config_.max_size);
  if (!base_udp_->Open(config_.ip_protocol == IpProtocol::IPV6 ? 6 : 4, port_)) {
    ErrorHandle(ErrorLevel::kSERIOUS, __FUNCTION__, "端口绑定失败");
    return;
  }
  thread_list_.reserve(config_.threads);
  for (size_t i = 0; i < config_.threads; ++i) {
    thread_list_.emplace_back(std::bind(&UdpTransport::RunThread,this));
    thread_list_[i].detach();
  }
}

void UdpTransport::Stop() {
  if (!start_)
    return;
  start_ = false;
  base_udp_->Close();
  base_udp_.reset();
  base_udp_ = nullptr;
  thread_list_.clear();
}

void UdpTransport::ReStart() {
  Stop();
  if (base_flag_ || (thread_count_ > 0))
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  Start();
}

void UdpTransport::BindDir(const char* ip, int32_t port) {
  if ((port > 65535) || (port<1)) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "端口不合法");
    return;
  }
  auto bind_address = address::from_string(ip, udp_error_);
  if (udp_error_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, udp_error_);
    udp_error_.clear();
    return;
  }
  dir_endpoint_ = udp::endpoint(bind_address, port);
  bind_flag_ = true;
}

void UdpTransport::BindDir(std::string& ip, int32_t port) {
  BindDir(ip .c_str(), port);
}

void UdpTransport::UnBind() {
  bind_flag_ = false;
}

void UdpTransport::Send(const char* send_str) {
  if (!start_) { 
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "系统未打开");
    return;
  }
  if (!bind_flag_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "未绑定默认发送端口");
    return;
  }
  size_t length = strlen(send_str);
  char* send_body = new char[length + 10];
  strcpy_s(send_body, length + 1, send_str);
  auto send_endpoint = new udp::endpoint(dir_endpoint_);
  base_udp_->DoSend(send_endpoint, send_body, length);
}

void UdpTransport::Send(std::string& send_str) {
  Send(send_str.c_str());
}

void UdpTransport::SendTo(const char* send_ip, int32_t send_port, const char* send_str) {
  if (!start_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "系统未打开");
    return;
  }
  if ((send_port > 65535) || (send_port<1)) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, "端口不合法");
    return;
  }
  auto send_address = address::from_string(send_ip, udp_error_);
  if (udp_error_) {
    ErrorHandle(ErrorLevel::kERROR, __FUNCTION__, udp_error_);
    udp_error_.clear();
    return;
  }
  size_t length = strlen(send_str);
  char* send_body = new char[length + 10];
  strcpy_s(send_body, length + 1, send_str);
  udp::endpoint* send_endpoint = new udp::endpoint(send_address, send_port);
  base_udp_->DoSend(send_endpoint, send_body, length);
  return;
}

void UdpTransport::SendTo(std::string& send_ip, int32_t send_port, const char* send_str) {
  return SendTo(send_ip.c_str(), send_port, send_str);
}

void UdpTransport::SendTo(const char* send_ip, int32_t send_port, std::string& send_str) {
  return SendTo(send_ip, send_port, send_str.c_str());
}

void UdpTransport::SendTo(std::string& send_ip, int32_t send_port, std::string& send_str) {
  return SendTo(send_ip.c_str(), send_port, send_str);
}

void UdpTransport::SetNewMessageHandle(const std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)>& new_message_handle) {
  new_message_handle_ = new_message_handle;
}
void UdpTransport::SetSendSuccessedHandle(const std::function<void()>& send_successed_handle) {
  send_successed_handle_ = send_successed_handle;
}
void UdpTransport::SetErrorHandle(const std::function<void(const char*, const char*)>& error_handle) {
  error_handle_ = error_handle;
}

std::string UdpTransport::DirIp() {
  return dir_endpoint_.address().to_string();
}

int32_t UdpTransport::DirPort() {
  return dir_endpoint_.port();
}

int32_t UdpTransport::port() {
  if (start_)
    return base_udp_->GetPort();
  else return port_;
}

bool UdpTransport::IsStart() {
  return start_;
}

void UdpTransport::RunThread() {
  ++thread_count_;
  service_.run(udp_error_);
  if (udp_error_) {
    ErrorHandle(ErrorLevel::kFATAL, __FUNCTION__, udp_error_);
    udp_error_.clear();
  }
  --thread_count_;
}

void UdpTransport::ReSet(UdpConfig& config) {
  config_ = config;
  ReStart();
}

void UdpTransport::ReceiveMessage(udp::endpoint* receive_endpoint, std::string& message) {
  std::string str = receive_endpoint->address().to_string();
  new_message_handle_(str, receive_endpoint->port(), message,
    [&](std::string& respond) {
    this->SendTo(receive_endpoint->address().to_string().c_str(), receive_endpoint->port(), respond);
  });
}
void UdpTransport::SendSuccessed() {
  std::cout << std::this_thread::get_id()<< std::endl;
  send_successed_handle_();
}



void UdpTransport::ErrorHandle(ErrorLevel level, const char* position,
    std::error_code error) {
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR:{
    break;
  }
  case ErrorLevel::kSERIOUS: {
    Stop();
    break;
  }
  case ErrorLevel::kFATAL:{
    std::cout << "发生严重错误，建议重新启动整个系统" << std::endl;
    break;
  }
  }
  error_handle_(position, error.message().c_str());
  //记录日志
}
void UdpTransport::ErrorHandle(ErrorLevel level,const char* position,const char* error){
  switch (level)
  {
  case ErrorLevel::kIGNORE:
  case ErrorLevel::kWARNING:
  case ErrorLevel::kERROR: {
    break;
  }
  case ErrorLevel::kSERIOUS: {
    Stop();
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