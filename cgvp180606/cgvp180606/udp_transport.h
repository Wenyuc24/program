#ifndef CGVP_CORE_MAIN_UDPTRANSPORT_H_
#define CGVP_CORE_MAIN_UDPTRANSPORT_H_

#include "transport.h"

namespace cgvp {
namespace communicate{

class UdpTransport:public Transport
{
class BaseUdp:public std::enable_shared_from_this<UdpTransport::BaseUdp>
{
public:
  BaseUdp(UdpTransport* udp_transport, boost::asio::io_service& service,
      size_t max_size); 
  ~BaseUdp();

  BaseUdp(const BaseUdp&) = delete;
  BaseUdp(BaseUdp&&) = delete;
  BaseUdp& operator=(const BaseUdp&) = delete;
  
  bool Open(int protocol, int32_t port);
  void Close();

  void DoReceive();
  void OnReceive(udp::endpoint* receive_endpoint,const std::error_code &error, size_t bytes);
  void DoSend(udp::endpoint* send_endpoint, char* send_str, size_t length);
  void OnSend(udp::endpoint* send_endpoint, char* send_body,
    const std::error_code &error, size_t bytes);

  int32_t GetPort();

private:
  void BaseError(ErrorLevel level, const char* position, std::error_code error);
  void BaseError(ErrorLevel level, const char* position, const char* error);
  
  UdpTransport* udp_transport_;
  
  char* buffer_;
  boost::asio::io_service& service_;
  udp::socket socket_;
  udp::endpoint default_endpoint_;
  std::mutex mutex_udp;
  boost::system::error_code base_error_;

  size_t max_size_;
  bool error_flag_;
};
public:
  UdpTransport(id_type id, int32_t port, UdpConfig& config);

  UdpTransport(const UdpTransport& Other) = delete;
  UdpTransport(UdpTransport&& Other) = delete;
  UdpTransport& operator=(UdpTransport& Other) = delete;

  ~UdpTransport();

  void Start();
  void Stop();
  void ReStart();
  
  void BindDir(const char* ip, int32_t port);
  void BindDir(std::string& ip, int32_t port);
  void UnBind();

  void Send(const char* send_str);
  void Send(std::string& send_str);

  void SendTo(const char* send_ip, int32_t send_port,const char* send_str);
  void SendTo(std::string& send_ip, int32_t send_port, const char* send_str);
  void SendTo(const char* send_ip, int32_t send_port, std::string& send_str);
  void SendTo(std::string& send_ip, int32_t send_port, std::string& send_str);

  void SetNewMessageHandle(const std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)>& new_message_handle);
  void SetSendSuccessedHandle(const std::function<void()>& send_successed_handle);
  void SetErrorHandle(const std::function<void(const char*, const char*)>& error_handle_);

  std::string DirIp();
  int32_t DirPort();
  
  int32_t port();
  bool IsStart();

private:
  friend class TransportManager;
  friend class BaseUdp;

  void RunThread();
  void ReSet(UdpConfig& config);

  void ReceiveMessage(udp::endpoint* receive_endpoint,std::string& message);
  void SendSuccessed();
  void ErrorHandle(ErrorLevel level, const char* position, std::error_code error);
  void ErrorHandle(ErrorLevel level, const char* position, const char* error);

  std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)> new_message_handle_;
  std::function<void()> send_successed_handle_;
  std::function<void(const char*, const char*)> error_handle_;

  std::shared_ptr<BaseUdp> base_udp_;
  boost::asio::io_service service_;
  udp::endpoint dir_endpoint_;
  std::vector<std::thread> thread_list_;
  boost::system::error_code udp_error_;

  bool start_;
  bool bind_flag_;
  bool base_flag_;
  int thread_count_;

  UdpConfig config_;
};
} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_UDPTRANSPORT_H_