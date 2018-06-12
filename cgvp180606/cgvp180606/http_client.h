#ifndef CGVP_CORE_MAIN_HTTPCLIENT_H_
#define CGVP_CORE_MAIN_HTTPCLIENT_H_

#include "transport.h"

namespace cgvp {
namespace communicate{

class HttpClient
{
public:
  void Get() {
    
  }
  void Post();
  void Put();
  void Delete();
  void Send();

  void SetHeader();
  void DeleteHeader();
};


} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_HTTPCLIENT_H_