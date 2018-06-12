#ifndef CGVP_CORE_MAIN_TRANSPORT_H_
#define CGVP_CORE_MAIN_TRANSPORT_H_
#include <cstring>
#include <ctime>

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <regex>
#include <unordered_map>

#include "boost/asio.hpp"
#include "config.h"

namespace cgvp {
namespace communicate {
class TransportManager;
using id_type = int32_t;
using udp = boost::asio::ip::udp;
using tcp = boost::asio::ip::tcp;
using address = boost::asio::ip::address;
class Transport{


public:



  virtual int32_t port() = 0;
  id_type id() { return id_; };
  void SetName(std::string name) {
    name_ = name;
  }

  std::string& name() {
    return name_;
  }
protected:
  Transport(id_type id, int32_t port, Type type) :id_(id), port_(port) 
  {
    switch (type) {
    case Type::kUdp: name_ = "UdpTransport"; break;
    case Type::kTcpClient: name_ = "TcpClient"; break;
    case Type::kTcpServer: name_ = "TcpServer"; break;
    case Type::kHttpClient: name_ = "HttpClient"; break;
    case Type::kHttpServer: name_ = "HttpServer"; break;
    }
  }

  virtual ~Transport() {}
  const id_type id_;
  int32_t port_;
  std::string name_;
  //boost::system::error_code error_code_;
};

} // communicate

} // cgvp

#endif//CGVP_CORE_MAIN_TRANSPORT_H_