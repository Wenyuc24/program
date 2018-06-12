#ifndef CGVP_CORE_MAIN_TRANSPORTMANAGER_H_
#define CGVP_CORE_MAIN_TRANSPORTMANAGER_H_

#include<cstdint>
#include<memory>
#include<map>
#include<algorithm>

#include"udp_transport.h"
#include"tcp_client.h"
#include"tcp_server.h"
#include"http_server.h"

namespace cgvp {
namespace communicate {

using transport_ptr = std::shared_ptr<Transport>;
using udp_ptr = std::shared_ptr<UdpTransport>;
using tcp_client_ptr = std::shared_ptr<TcpClient>;
using tcp_server_ptr = std::shared_ptr<TcpServer>;
class TransportManager
{

public:
  id_type AddTransport(transport_ptr);
  bool HasTransport(id_type);
  void DeleteTransport(id_type);
  void ClearTransport();

  udp_ptr CreateUdp(int32_t port = 0);
  udp_ptr CreateUdp(UdpConfig &config, int32_t port = 0);
  id_type CreateUdpInline(int32_t port = 0);
  id_type CreateUdpInline(UdpConfig &config, int32_t port = 0);
  udp_ptr GetUdpServer(id_type);

  tcp_client_ptr CreateTcpClient();
  tcp_client_ptr CreateTcpClient(TcpClientConfig& config);
  id_type CreateTcpClientInline();
  id_type CreateTcpClientInline(TcpClientConfig& config);
  tcp_client_ptr GetTcpClient(id_type);

  tcp_server_ptr CreateTcpServer(int32_t port = 0);
  tcp_server_ptr CreateTcpServer(TcpServerConfig&, int32_t port = 0);
  id_type CreateTcpServerInline(int32_t port = 0);
  id_type CreateTcpServerInline(TcpServerConfig&, int32_t port = 0);
  tcp_server_ptr GetTcpServer(id_type);

  inline UdpConfig DefaultUdp();
  inline TcpClientConfig DefaultTcpClient();
  inline TcpServerConfig DefaultTcpServer();
  inline HttpClientConfig DefaultHttpClient();
  inline HttpServerConfig DefaultHttpServer();

  size_t count() {
    return count_;
  }

private:
  std::map<id_type, transport_ptr> transport_map_;
  static id_type last_id_;
  size_t count_ = 0;

  UdpConfig default_udp_;
  TcpClientConfig default_tcp_client_;
  TcpServerConfig default_tcp_server_;
  HttpClientConfig default_http_client_;
  HttpServerConfig default_http_server_;
};

} // communicate


} // cgvp
#endif//CGVP_CORE_MAIN_TRANSPORTMANAGER_H_