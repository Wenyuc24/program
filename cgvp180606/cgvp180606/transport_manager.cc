#include"transport_manager.h"

namespace cgvp {
  
namespace  communicate {

id_type TransportManager::AddTransport(transport_ptr transport) {
  if (transport) {
    transport_map_[transport->id()] = transport;
    ++count_;
    return transport->id();
  }
  else return 0;
}

bool TransportManager::HasTransport(id_type id) {
  auto iter = transport_map_.find(id);
  if (iter == transport_map_.end())
    return false;
  return true;
}

void TransportManager::DeleteTransport(id_type id) {
  auto iter = transport_map_.find(id);
  if (iter != transport_map_.end()){
    transport_map_.erase(iter);
    --count_;
  }  
}

void TransportManager::ClearTransport() {
  for (auto iter = transport_map_.begin(); 
    iter != transport_map_.end(); 
    iter++) 
  {
    transport_map_.erase(iter);
  }
  count_ = 0;
}

udp_ptr TransportManager::CreateUdp(int32_t port) {
  return CreateUdp(default_udp_, port);
}


udp_ptr TransportManager::CreateUdp(UdpConfig &config,int32_t port ) {
  id_type id = ((++last_id_)%10000)*5;

  config.max_size = std::max<size_t>(1, config.max_size);
  config.threads = std::max<size_t>(1, config.threads);
  if (port < 0 || port>65535)
    port = 0;
  udp_ptr udp_transport= std::make_shared<UdpTransport>(id, port, config);
  return udp_transport;
}

id_type TransportManager::CreateUdpInline(int32_t port) {
  return CreateUdpInline(default_udp_, port);
}

id_type TransportManager::CreateUdpInline(UdpConfig &config, int32_t port){
  udp_ptr udp_server=CreateUdp(config,port);
  return AddTransport(udp_server);
}

udp_ptr TransportManager::GetUdpServer(id_type id) {
  if ((id % 5) != 0) {
    return nullptr;
  }
  auto iter = transport_map_.find(id);
  if (iter == transport_map_.end()) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<UdpTransport>(iter->second);
}

tcp_client_ptr TransportManager::CreateTcpClient() {
  return CreateTcpClient(default_tcp_client_);
}

tcp_client_ptr TransportManager::CreateTcpClient(TcpClientConfig& config) 
{
  id_type id = ((++last_id_) % 10000) * 5+1;
  config.max_size = std::max<size_t>(1, config.max_size);
  if (config.ping_body.empty())
    config.ping_body = "\0";
  config.ping_time = std::max<int32_t>(2, config.ping_time);
  tcp_client_ptr tcp_client = std::make_shared<TcpClient>(id,config);
  return tcp_client;
}

id_type TransportManager::CreateTcpClientInline() {
  return CreateTcpClientInline(default_tcp_client_);
}

id_type TransportManager::CreateTcpClientInline(TcpClientConfig& config){
  auto tcp_client = CreateTcpClient(config);
  return AddTransport(tcp_client);
}

tcp_client_ptr TransportManager::GetTcpClient(id_type id) {
  if ((id % 5) != 1) {
    return nullptr;
  }
  auto iter = transport_map_.find(id);
  if (iter == transport_map_.end()) {
    return nullptr;
  }
  return std::dynamic_pointer_cast<TcpClient>(iter->second);
}


tcp_server_ptr TransportManager::CreateTcpServer(int32_t port) {
  return CreateTcpServer(default_tcp_server_, port);
}

tcp_server_ptr TransportManager::CreateTcpServer(TcpServerConfig& config, 
    int32_t port) {
  id_type id = ((++last_id_) % 10000) * 5 + 2;
  config.max_size = std::max<size_t>(1, config.max_size);
  if (config.ping_body.empty())
    config.ping_body = "\0";
  if (port < 0 || port>65535)
    port = 0;
  config.dead_time = std::max<size_t>(2, config.dead_time);
  auto tcp_server = std::make_shared<TcpServer>(id, port, config);
  return tcp_server;
}

id_type TransportManager::CreateTcpServerInline(int32_t port) {
  return CreateTcpServerInline(default_tcp_server_, port);
}

id_type TransportManager::CreateTcpServerInline(
    TcpServerConfig& config,int32_t port) 
{
  auto tcp_server = CreateTcpServer(config, port);
  return AddTransport(tcp_server);
}

tcp_server_ptr TransportManager::GetTcpServer(id_type id) {
  if ((id % 5) != 2) {
    //记录日志
    return nullptr;
  }
  auto iter = transport_map_.find(id);
  if (iter == transport_map_.end()) {
    //记录日志
    return nullptr;
  }
  return std::dynamic_pointer_cast<TcpServer>(iter->second);
}

inline UdpConfig TransportManager::DefaultUdp() {
  return default_udp_;
}

inline TcpClientConfig TransportManager::DefaultTcpClient() {
  return default_tcp_client_;
}

inline TcpServerConfig TransportManager::DefaultTcpServer() {
  return default_tcp_server_;
}

inline HttpClientConfig TransportManager::DefaultHttpClient() {
  return default_http_client_;
}

inline HttpServerConfig TransportManager::DefaultHttpServer() {
  return default_http_server_;
}

id_type TransportManager::last_id_ = 365;
} //  communicate


} // cgvp