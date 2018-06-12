#ifndef CGVP_CORE_MAIN_TCPCLIENT_H_
#define CGVP_CORE_MAIN_TCPCLIENT_H_

#include "transport.h"

namespace cgvp {
namespace communicate{
class TcpClient:public Transport
{
class BaseTcpClient:public std::enable_shared_from_this<BaseTcpClient>
{
public:

  BaseTcpClient(TcpClient* tcp_client, boost::asio::io_service& service, size_t max_size);
  BaseTcpClient(const BaseTcpClient&) = delete;
  BaseTcpClient(BaseTcpClient&&) = delete;
  BaseTcpClient& operator=(const BaseTcpClient&) = delete;

  ~BaseTcpClient();

  bool Run(int protocol, tcp::endpoint& dir_endpoint);
  void Stop();

  void DoConnect(tcp::endpoint& endpoint);
  void OnConnect(const std::error_code &error);

  void DoRead();
  void OnRead(const std::error_code &error,size_t bytes);
  void DoWrite(char* write_body, size_t length);
  void OnWrite(char* write_body,const std::error_code &error,size_t bytes);

  int32_t GetPort();
private:
  void BaseError(ErrorLevel level, const char* position, std::error_code error);
  void BaseError(ErrorLevel level, const char* position, const char* error);

  TcpClient* const tcp_client_;

  char* buffer_;
  tcp::socket socket_;
  boost::system::error_code base_error_;

  size_t max_size_;
  bool error_flag_;
};
public:
  TcpClient(id_type id, TcpClientConfig& config);
  TcpClient(const TcpClient& Other) = delete;
  TcpClient(TcpClient&& Other) = delete;
  TcpClient& operator=(TcpClient& Other) = delete;

  ~TcpClient();

  void Connect();
  void Disconnect();

  void BindDir(const char* ip, int32_t port);
  void BindDir(std::string& ip, int32_t port);
  void UnBind();

  void Send(const char* send_str);
  void Send(std::string& send_str);

  void SendPing();

  std::string DirIp();
  int32_t DirPort();

  int32_t port();

  bool IsConnect();


private:
  void RunThread(); 
  void ReSet(TcpClientConfig& config);

  void ConnectSuccess();
  void ReadMessage(char* message);
  void WriteSuccess();
  void ErrorHandle(ErrorLevel level, const char* position, std::error_code error);
  void ErrorHandle(ErrorLevel level, const char* position, const char* error);
  
  boost::asio::io_service service_;
  std::shared_ptr<BaseTcpClient> base_tcp_client_;
  tcp::endpoint dir_endpoint_;
  boost::system::error_code tcp_error_;
  boost::system::error_code ignore_error_;
  boost::asio::deadline_timer timer_;

  time_t now_time_;
  time_t last_time_;
  bool connect_;
  bool bind_flag_ = false;
  bool base_flag_;
  bool threads_flag_;

  TcpClientConfig config_;
};


} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_TCPCLIENT_H_