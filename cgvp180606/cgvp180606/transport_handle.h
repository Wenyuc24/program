#ifndef CGVP_CORE_MAIN_TRANSPORTHANDLE_H_
#define CGVP_CORE_MAIN_TRANSPORTHANDLE_H_
#include<cstdint>

#include<string>

#include "config.h"

namespace cgvp
{
namespace communicate
{

class TransportHandle
{
public:
  virtual void ErrorHandle(ErrorLevel level, std::string& postion, std::string& error_message) {}
};

class UdpHandle:public TransportHandle
{
public:
  virtual void ReceiveHandle() {}
  virtual void SendHandle() {}
};

class TcpClientHandle :public TransportHandle
{
public:
  virtual void ReceiveHandle() {}
  virtual void SendHandle() {}
};

class TcpServerHandle :public TransportHandle
{
public:
  virtual void NewConnect() {}
  virtual void ReceiveHandle() {}
  virtual void SendHandle() {}
};

class HttpClientHandle :public TransportHandle
{
public:
  virtual void ReceiveHandle() {}
  virtual void SendHandle() {}
};

class HttpServerHandle :public TransportHandle
{
public:
  virtual void ReceiveHandle() {}
  virtual void SendHandle() {}
};
} // config
}//cgvp
#endif //CGVP_CORE_MAIN_TRANSPORTHANDLE_H_