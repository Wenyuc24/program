#include"transport_manager.h"
#include"http_message.h"

std::string str1("0101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101010");
std::string str2("received");
std::string str4(R"(HTTP/1.1 200 OK
  Server: nginx / 1.12.2
  Date : Wed, 06 Jun 2018 05 : 37 : 28 GMT
  Content - Type : text / html
  Content - Length : 612
  Connection : keep - alive
  ETag : "5ad6bb38-264"
  Cache - Control : no - cache
  Access - Control - Allow - Origin : *
  Accept - Ranges : bytes

  <!DOCTYPE html>
  <html>
  <head>
  <title>Welcome to nginx!< / title>
< / head>
</html>
)");
void udp_receive_message(std::string ip, int32_t port, std::string& message,std::function<void(std::string&)> respond) 
{

  std::cout << ip << ":" << port << std::endl;
  std::cout << message << std::endl;

  respond(str1);

  return;
}

void udp_send_success() {
  std::cout << "send ok" << std::endl;
}

void error(const char* opsition, const char* message) {
  //std::cout << opsition << ":" << std::endl;
  //std::cout << message << std::endl;
  
}

void tcp_connect(std::string& ip, int32_t port,std::function<void(std::string&)> send) {
 // std::cout << ip << ":" << port << std::endl;
  send(std::string("connected"));
}
 
void tcp_new_message(std::string& ip, int32_t port, std::string& message, std::function<void(std::string&)> respond) {
 // std::cout << ip << ":" << port << std::endl;
 // std::cout << message << std::endl;

  respond(str4);
}

void tcp_send() {
  //std::cout << "send ok" << std::endl;
}

void http_new_message(const std::string& dor_root,const cgvp::communicate::http::Request& req, cgvp::communicate::http::Respond& res) {
  std::cout << "8888" << std::endl;//aaa
  if (req.Is_request()) {
    res.Body(std::string("wrong"));
    return;
  }
  std::cout << "8888" << std::endl;//aaa
  std::string body(str4);

  res.Body(body);
}


int main() {

  /*cgvp::communicate::TransportManager testt;
  
  auto x = testt.CreateTcpServer(12400);
  
  x->SetConnectHandle(tcp_connect);
  x->SetNewMessageHandle(tcp_new_message);
  x->SetSendHandle(tcp_send);
  x->SetErrorHandle(error);*/

  /*auto y = testt.CreateUdp(12402);
  y->SetNewMessageHandle(udp_receive_message);
  y->SetSendSuccessedHandle(udp_send_success);
  y->SetErrorHandle(error);
  */

  cgvp::communicate::HttpServerConfig config;
  cgvp::communicate::HttpServer test(15, 12441, config);
  test.SetReadHandle(http_new_message);
  test.SetError(error);
  
  std::this_thread::sleep_for(std::chrono::milliseconds(1000000));
  return 0;
}