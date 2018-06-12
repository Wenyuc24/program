#ifndef CGVP_CORE_MAIN_CONFIGURATION_H_
#define CGVP_CORE_MAIN_CONFIGURATION_H_
#include<cstdint>

#include<string>

namespace cgvp
{
namespace communicate
{
enum class IpProtocol:char {
  IPV4 = 4,
  IPV6 = 6,
};

enum class ErrorLevel :char {
  kIGNORE = 0,
  kWARNING,
  kERROR,
  kSERIOUS,
  kFATAL,
};

enum class Type :char {
  kUdp = 0,
  kTcpClient,
  kTcpServer,
  kHttpClient,
  kHttpServer,
};

struct UdpConfig//:public Config
{
  size_t max_size = 1024;
  bool respond_receive = false;
  std::string respond_body = "received";
  IpProtocol ip_protocol = IpProtocol::IPV4;
  size_t threads = 5;
};

struct TcpClientConfig
{
  size_t max_size = 1024;
  bool send_ping = false;
  std::string ping_body = "\0";
  size_t ping_time = 7;
  IpProtocol ip_protocol = IpProtocol::IPV4;
};

struct TcpServerConfig
{
  size_t max_size = 10240;
  size_t dead_time = 100;
  bool ignort_ping = true;
  std::string ping_body = "\0";

  IpProtocol ip_protocol = IpProtocol::IPV4;
  size_t threads = 20;
  size_t clients = 5;
};

struct HttpClientConfig
{
  float version = 1.0;
  IpProtocol ip_protocol = IpProtocol::IPV4;
};

struct HttpServerConfig
{
  float version = 1.0;
  IpProtocol ip_protocol = IpProtocol::IPV4;
  std::string dor_root = ".";
  size_t max_size = 10240;
};
} // config
}//cgvp
#endif //CGVP_CORE_MAIN_CONFIGURATION_H_