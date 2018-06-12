#ifndef CGVP_CORE_MAIN_HTTPSERVER_H_
#define CGVP_CORE_MAIN_HTTPSERVER_H_

#include "transport.h"
#include "http_message.h"

namespace cgvp {
namespace communicate{

class HttpServer:public Transport
{
  class HttpSession;
  using Session_iter = std::list<std::shared_ptr<HttpSession> >::iterator;
  class HttpSession:public std::enable_shared_from_this<HttpSession>
  {
  public:
    HttpSession(HttpServer* http_server,boost::asio::io_service& super_http_service,size_t max_size);

    void Run(Session_iter this_session);
    void Stop();

    void DoRead();
    void OnRead(const std::error_code &error, size_t bytes);
    void OnWrite(http::Respond* res, const std::error_code &error, size_t bytes);

    tcp::socket& socket();
    boost::asio::io_service service_;
  private:
    void DeleteThis();
    void BaseError(ErrorLevel level, const char* position, std::error_code error);
    void BaseError(ErrorLevel level, const char* position, const char* error);

    HttpServer* http_server_;
    bool error_flag_;

    
    boost::asio::io_service& super_http_service_;

    char* buffer_;
    tcp::socket socket_;
    size_t max_size_;
    Session_iter this_session_;
  };
public:
  HttpServer(id_type id,int32_t port,HttpServerConfig& config);
  void StartListen();
  void StopListen();

  void DoAccept();
  void OnAccept(std::shared_ptr<HttpSession> new_session,const std::error_code& error);

  void SetReadHandle(const std::function<void(const std::string&, const http::Request&, http::Respond&)>& read_handle);
  void SetError(const std::function<void(const char*, const char*)>& error_handle);

  int32_t port();
private:
  void RunThread();
  Session_iter DeleteSession(Session_iter session);
  void NewRequest(http::Request& req, http::Respond& res);
  void ErrorHandle(ErrorLevel level, const char* position, std::error_code error);
  void ErrorHandle(ErrorLevel level, const char* position, const char* error);

  boost::asio::io_service service_;
  tcp::acceptor* acceptor_;
  std::list<std::shared_ptr<HttpSession> > Session_list_;

  std::function<void(const std::string&, const http::Request&, http::Respond&)> read_handle_;
  std::function<void(const char*, const char*)> error_handle_;

  boost::system::error_code http_error_;
  boost::system::error_code ignore_error_;

  bool listening_;
  size_t session_count_;
  HttpServerConfig config_;
};

} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_HTTPSERVER_H_