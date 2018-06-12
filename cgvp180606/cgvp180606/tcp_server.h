#ifndef CGVP_CORE_MAIN_TCPSERVER_H_
#define CGVP_CORE_MAIN_TCPSERVER_H_

#include "transport.h"
#include <mutex>
namespace cgvp {
namespace communicate{

class TcpServer :public Transport
{
  using id_t = int32_t;
  class TcpSession :public std::enable_shared_from_this<TcpSession>
  {
  public:

    TcpSession(TcpServer* tcp_server,id_t this_session,size_t max_size);
    TcpSession(const TcpSession&) = delete;
    TcpSession(TcpSession&&) = delete;
    TcpSession& operator=(const TcpSession&) = delete;

    ~TcpSession();

    bool Run();
    void Stop();

    void DoRead();
    void OnRead(const std::error_code &error, size_t bytes);
    void DoWrite(char* write_body, size_t length);
    void OnWrite(char* write_body, const std::error_code &error, size_t bytes);

    std::string DirIp();
    int32_t DirPort();
    tcp::socket& socket();
    id_t GetId();

  private:
    void BaseError(ErrorLevel level, const char* position, std::error_code error);
    void BaseError(ErrorLevel level, const char* position, const char* error);

    boost::asio::io_service service_;

    char* buffer_;
    tcp::socket socket_;

    boost::system::error_code ignore_error_;

    bool error_flag_;
    size_t max_size_;

    const id_t this_session_;
    TcpServer* tcp_server_;
  };

public:
  TcpServer(id_type id, int32_t port, TcpServerConfig& config);
  TcpServer(const TcpServer& Other) = delete;
  TcpServer(TcpServer&& Other) = delete;
  TcpServer& operator=(TcpServer& Other) = delete;

  ~TcpServer();

  void StartListen();
  void StopListen();

  void DoAccept();
  void OnAccept(std::shared_ptr<TcpSession> new_session,const std::error_code& error);

  void Send(std::string send_ip, int32_t send_port, const char* send_str);
  void Send(std::string send_ip, int32_t send_port, std::string& send_str);
  void DisConnect(std::string ip, int32_t port);

  void SetConnectHandle(const std::function<void(std::string&, int32_t, std::function<void(std::string&)>)> connect_handle);
  void SetNewMessageHandle(const std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)>& new_message_handle);
  void SetSendHandle(const std::function<void()>& send_handle);
  void SetErrorHandle(const std::function<void(const char*, const char*)>& error_handle);

  size_t clients();
  int32_t port();

private:
  void RunThread();
  id_t FindTcpSession(std::string& ip, int32_t port);
  std::unordered_map<id_t, std::shared_ptr<TcpSession> >::iterator DeleteTcpSession(id_t session);

  void ReSet(TcpServerConfig& config);

  void SendTo(id_t session, std::string& send_str);

  void NewConnect(id_t session_id);
  void SendSuccess();
  void NewMessage(id_t session_id, std::string& message);

  void ErrorHandle(id_t session,ErrorLevel level, const char* position, std::error_code error);
  void ErrorHandle(id_t session,ErrorLevel level, const char* position, const char* error);

  std::function<void(std::string&, int32_t, std::function<void(std::string&)>)> connect_handle_;
  std::function<void(std::string&, int32_t, std::string&, std::function<void(std::string&)>)> new_message_handle_;
  std::function<void()> send_handle_;
  std::function<void(const char*, const char*)> error_handle_;

  boost::asio::io_service service_;
  tcp::acceptor *acceptor_;
  std::unordered_map<id_t, std::shared_ptr<TcpSession> > session_map_;

  id_t session_id;
  boost::system::error_code tcp_error_;

  bool listening_;
  std::atomic_int session_count_;

  TcpServerConfig config_;

  std::mutex tcp_mutex;

};


} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_TCPSERVER_H_
