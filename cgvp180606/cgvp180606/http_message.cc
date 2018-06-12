#include "http_message.h"


namespace cgvp {
namespace communicate{
namespace http{
#define AddSet(val,str) Setstr(#val,str)  
void Setstr(const char* val, std::string& str) {
  str = val;
}

bool is_char(int c)
{
  return c >= 0 && c <= 127;
}

bool is_ctl(int c)
{
  return (c >= 0 && c <= 31) || (c == 127);
}

bool is_tspecial(int c)
{
  switch (c)
  {
  case '(': case ')': case '<': case '>': case '@':
  case ',': case ';': case ':': case '\\': case '"':
  case '/': case '[': case ']': case '?': case '=':
  case '{': case '}': case ' ': case '\t':
    return true;
  default:
    return false;
  }
}

bool is_digit(int c)
{
  return c >= '0' && c <= '9';
}

std::string Method::ToString(method_t method){
  std::string method_name;
  AddSet(method, method_name);
  return method_name;
}

std::string ToString(Status status) {
  std::string status_name;
  AddSet(status, status_name);
  return status_name;
}

std::string ToString(Head_type header) {
  std::string header_name;
  AddSet(status, header_name);
  return header_name;
}

void HttpMessage::SetHeader(std::string name, std::string value) {
  headers_.emplace_back(name, value);
}
void HttpMessage::Body(std::string& body) {
  body_ = body;
}
std::string& HttpMessage::Body() {
  return body_;
}
void HttpMessage::KeepAlive(bool keep_alive) {
  keep_alive_ = keep_alive;
}
bool HttpMessage::KeepAlive() {
  return keep_alive_;
}
void HttpMessage::Version(std::string& version) {
  version_ = version;
}
std::string& HttpMessage::Version() {
  return version_;
}


int Request::consume(char input) {
  switch (state_)
  {
  case method_start:
    if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return -1;
    }
    else
    {
      state_ = state::method;
      method_.push_back(input);
      return 0;
    }
  case state::method:
    if (input == ' ')
    {
      state_ = uri;
      return 0;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return -1;
    }
    else
    {
      method_.push_back(input);
      return 0;
    }
  case uri:
    if (input == ' ')
    {
      state_ = http_version_h;
      return 0;
    }
    else if (is_ctl(input))
    {
      return -1;
    }
    else
    {
      url_.push_back(input);
      return 0;
    }
  case http_version_h:
    if (input == 'H')
    {
      state_ = http_version_t_1;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_t_1:
    if (input == 'T')
    {
      state_ = http_version_t_2;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_t_2:
    if (input == 'T')
    {
      state_ = http_version_p;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_p:
    if (input == 'P')
    {
      state_ = http_version_slash;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_slash:
    if (input == '/')
    {
      state_ = http_version_major_start;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_major_start:
    if (is_digit(input))
    {
      state_ = http_version_major;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_major:
    if (input == '.')
    {
      state_ = http_version_minor_start;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_minor_start:
    if (is_digit(input))
    {
      if (input == '1')
        version_ = "HTTP/1.1";
      else version_ = "HTTP/1.0";
      state_ = http_version_minor;
      return 0;
    }
    else
    {
      return -1;
    }
  case http_version_minor:
    if (input == '\r')
    {
      state_ = expecting_newline_1;
      return 0;
    }
    else
    {
      return -1;
    }
  case expecting_newline_1:
    if (input == '\n')
    {
      state_ = header_line_start;
      return 0;
    }
    else
    {
      return -1;
    }
  case header_line_start:
    if (input == '\r')
    {
      state_ = expecting_newline_3;
      return 0;
    }
    else if (input == ' ' || input == '\t')
    {
      state_ = header_lws;
      return 0;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return -1;
    }
    else
    {
      headers_.emplace_back(header());
      headers_.back().name.push_back(input);
      state_ = header_name;
      return 0;
    }
  case header_lws:
    if (input == '\r')
    {
      state_ = expecting_newline_2;
      return 0;
    }
    else if (input == ' ' || input == '\t')
    {
      return 0;
    }
    else if (is_ctl(input))
    {
      return -1;
    }
    else
    {
      state_ = header_value;
      headers_.back().value.push_back(input);
      return 0;
    }
  case header_name:
    if (input == ':')
    {
      state_ = space_before_header_value;
      return 0;
    }
    else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
    {
      return -1;
    }
    else
    {
      headers_.back().name.push_back(input);
      return 0;
    }
  case space_before_header_value:
    if (input == ' ')
    {
      state_ = header_value;
      return 0;
    }
    else
    {
      return -1;
    }
  case header_value:
    if (input == '\r')
    {
      state_ = expecting_newline_2;
      return 0;
    }
    else if (is_ctl(input))
    {
      return -1;
    }
    else
    {
      headers_.back().value.push_back(input);
      return 0;
    }
  case expecting_newline_2:
    if (input == '\n')
    {
      state_ = header_line_start;
      return 0;
    }
    else
    {
      return -1;
    }
  case expecting_newline_3:
    return (input == '\n');
  default:
    return -1;
  }
}

Request::Request(char *req_string, size_t bytes) {
  size_t begin = 0;
  int result = 0;
  while (begin != bytes) {
    result = consume(req_string[begin++]);
    if (result == -1) {
      is_request = false;
      return;
    }
    if (result == 1)
      break;
  }
  body_ = req_string + begin;
  is_request = true;
}

void Request::SetMethod(Method::method_t method) {
  method_ = Method::ToString(method);
}
std::string& Request::method() {
  return method_;
}

std::string& Request::url() {
  return url_;
}

void Respond::SetBodyFromFile(std::string file_name) {
  std::ifstream is(file_name.c_str(), std::ios::in | std::ios::binary);
  if (!is) {
    return;
  }
  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0)
    body_.append(buf, is.gcount());
}

void Respond::SetStatus(Status status) {
  status_ = ToString(status);
}

} // http
} // communicate
} // cgvp


