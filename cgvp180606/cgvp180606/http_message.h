#ifndef CGVP_CORE_MAIN_HTTP_MESSAGE_H_
#define CGVP_CORE_MAIN_HTTP_MESSAGE_H_

#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "boost/beast.hpp"
namespace cgvp {
namespace communicate{
namespace http{
//using Method = boost::beast::http::verb;
//using Status = boost::beast::http::status;
//using Head_type = boost::beast::http::field;
//using file_body = boost::beast::http::file_body::value_type; 
class Method {
public:
  enum method_t{
    UNKNOWN = 0,
    DELETE_,
    GET,
    HEAD,
    POST,
    PUT,
    CONNECT,
    OPTIONS,
    TRACE,
    // WebDAV
    COPY,
    LOCK,
    MKCOL,
    MOVE,
    PROPFIND,
    PROPPATH,
    SEARCH,
    UNLOCK,
    BIND,
    REBIND,
    UNBIND,
    ACL,

    // RFC-5789
    PATCH,
    PURGE,
    // RFC-2068, section 19.6.1.2
    LINK,
    UNLINK
  };

  static std::string ToString(method_t method);
};

enum Status 
{
  unknown = 0,

  continue_ = 100,
  switching_protocols = 101,
  processing = 102,

  ok = 200,
  created = 201,
  accepted = 202,
  non_authoritative_information = 203,
  no_content = 204,
  reset_content = 205,
  partial_content = 206,
  multi_status = 207,
  already_reported = 208,
  im_used = 226,

  multiple_choices = 300,
  moved_permanently = 301,
  found = 302,
  see_other = 303,
  not_modified = 304,
  use_proxy = 305,
  temporary_redirect = 307,
  permanent_redirect = 308,

  bad_request = 400,
  unauthorized = 401,
  payment_required = 402,
  forbidden = 403,
  not_found = 404,
  method_not_allowed = 405,
  not_acceptable = 406,
  proxy_authentication_required = 407,
  request_timeout = 408,
  conflict = 409,
  gone = 410,
  length_required = 411,
  precondition_failed = 412,
  payload_too_large = 413,
  uri_too_long = 414,
  unsupported_media_type = 415,
  range_not_satisfiable = 416,
  expectation_failed = 417,
  misdirected_request = 421,
  unprocessable_entity = 422,
  locked = 423,
  failed_dependency = 424,
  upgrade_required = 426,
  precondition_required = 428,
  too_many_requests = 429,
  request_header_fields_too_large = 431,
  connection_closed_without_response = 444,
  unavailable_for_legal_reasons = 451,
  client_closed_request = 499,

  internal_server_error = 500,
  not_implemented = 501,
  bad_gateway = 502,
  service_unavailable = 503,
  gateway_timeout = 504,
  http_version_not_supported = 505,
  variant_also_negotiates = 506,
  insufficient_storage = 507,
  loop_detected = 508,
  not_extended = 510,
  network_authentication_required = 511,
  network_connect_timeout_error = 599
};

enum class Head_type 
{
  unknown = 0,

  a_im,
  accept,
  accept_additions,
  accept_charset,
  accept_datetime,
  accept_encoding,
  accept_features,
  accept_language,
  accept_patch,
  accept_post,
  accept_ranges,
  access_control,
  access_control_allow_credentials,
  access_control_allow_headers,
  access_control_allow_methods,
  access_control_allow_origin,
  access_control_max_age,
  access_control_request_headers,
  access_control_request_method,
  age,
  allow,
  alpn,
  also_control,
  alt_svc,
  alt_used,
  alternate_recipient,
  alternates,
  apparently_to,
  apply_to_redirect_ref,
  approved,
  archive,
  archived_at,
  article_names,
  article_updates,
  authentication_control,
  authentication_info,
  authentication_results,
  authorization,
  auto_submitted,
  autoforwarded,
  autosubmitted,
  base,
  bcc,
  body,
  c_ext,
  c_man,
  c_opt,
  c_pep,
  c_pep_info,
  cache_control,
  caldav_timezones,
  cancel_key,
  cancel_lock,
  cc,
  close,
  comments,
  compliance,
  connection,
  content_alternative,
  content_base,
  content_description,
  content_disposition,
  content_duration,
  content_encoding,
  content_features,
  content_id,
  content_identifier,
  content_language,
  content_length,
  content_location,
  content_md5,
  content_range,
  content_return,
  content_script_type,
  content_style_type,
  content_transfer_encoding,
  content_type,
  content_version,
  control,
  conversion,
  conversion_with_loss,
  cookie,
  cookie2,
  cost,
  dasl,
  date,
  date_received,
  dav,
  default_style,
  deferred_delivery,
  delivery_date,
  delta_base,
  depth,
  derived_from,
  destination,
  differential_id,
  digest,
  discarded_x400_ipms_extensions,
  discarded_x400_mts_extensions,
  disclose_recipients,
  disposition_notification_options,
  disposition_notification_to,
  distribution,
  dkim_signature,
  dl_expansion_history,
  downgraded_bcc,
  downgraded_cc,
  downgraded_disposition_notification_to,
  downgraded_final_recipient,
  downgraded_from,
  downgraded_in_reply_to,
  downgraded_mail_from,
  downgraded_message_id,
  downgraded_original_recipient,
  downgraded_rcpt_to,
  downgraded_references,
  downgraded_reply_to,
  downgraded_resent_bcc,
  downgraded_resent_cc,
  downgraded_resent_from,
  downgraded_resent_reply_to,
  downgraded_resent_sender,
  downgraded_resent_to,
  downgraded_return_path,
  downgraded_sender,
  downgraded_to,
  ediint_features,
  eesst_version,
  encoding,
  encrypted,
  errors_to,
  etag,
  expect,
  expires,
  expiry_date,
  ext,
  followup_to,
  forwarded,
  from,
  generate_delivery_report,
  getprofile,
  hobareg,
  host,
  http2_settings,
  if_,
  if_match,
  if_modified_since,
  if_none_match,
  if_range,
  if_schedule_tag_match,
  if_unmodified_since,
  im,
  importance,
  in_reply_to,
  incomplete_copy,
  injection_date,
  injection_info,
  jabber_id,
  keep_alive,
  keywords,
  label,
  language,
  last_modified,
  latest_delivery_time,
  lines,
  link,
  list_archive,
  list_help,
  list_id,
  list_owner,
  list_post,
  list_subscribe,
  list_unsubscribe,
  list_unsubscribe_post,
  location,
  lock_token,
  man,
  max_forwards,
  memento_datetime,
  message_context,
  message_id,
  message_type,
  meter,
  method_check,
  method_check_expires,
  mime_version,
  mmhs_acp127_message_identifier,
  mmhs_authorizing_users,
  mmhs_codress_message_indicator,
  mmhs_copy_precedence,
  mmhs_exempted_address,
  mmhs_extended_authorisation_info,
  mmhs_handling_instructions,
  mmhs_message_instructions,
  mmhs_message_type,
  mmhs_originator_plad,
  mmhs_originator_reference,
  mmhs_other_recipients_indicator_cc,
  mmhs_other_recipients_indicator_to,
  mmhs_primary_precedence,
  mmhs_subject_indicator_codes,
  mt_priority,
  negotiate,
  newsgroups,
  nntp_posting_date,
  nntp_posting_host,
  non_compliance,
  obsoletes,
  opt,
  optional,
  optional_www_authenticate,
  ordering_type,
  organization,
  origin,
  original_encoded_information_types,
  original_from,
  original_message_id,
  original_recipient,
  original_sender,
  original_subject,
  originator_return_address,
  overwrite,
  p3p,
  path,
  pep,
  pep_info,
  pics_label,
  position,
  posting_version,
  pragma,
  prefer,
  preference_applied,
  prevent_nondelivery_report,
  priority,
  privicon,
  profileobject,
  protocol,
  protocol_info,
  protocol_query,
  protocol_request,
  proxy_authenticate,
  proxy_authentication_info,
  proxy_authorization,
  proxy_connection,
  proxy_features,
  proxy_instruction,
  public_,
  public_key_pins,
  public_key_pins_report_only,
  range,
  received,
  received_spf,
  redirect_ref,
  references,
  referer,
  referer_root,
  relay_version,
  reply_by,
  reply_to,
  require_recipient_valid_since,
  resent_bcc,
  resent_cc,
  resent_date,
  resent_from,
  resent_message_id,
  resent_reply_to,
  resent_sender,
  resent_to,
  resolution_hint,
  resolver_location,
  retry_after,
  return_path,
  safe,
  schedule_reply,
  schedule_tag,
  sec_websocket_accept,
  sec_websocket_extensions,
  sec_websocket_key,
  sec_websocket_protocol,
  sec_websocket_version,
  security_scheme,
  see_also,
  sender,
  sensitivity,
  server,
  set_cookie,
  set_cookie2,
  setprofile,
  sio_label,
  sio_label_history,
  slug,
  soapaction,
  solicitation,
  status_uri,
  strict_transport_security,
  subject,
  subok,
  subst,
  summary,
  supersedes,
  surrogate_capability,
  surrogate_control,
  tcn,
  te,
  timeout,
  title,
  to,
  topic,
  trailer,
  transfer_encoding,
  ttl,
  ua_color,
  ua_media,
  ua_pixels,
  ua_resolution,
  ua_windowpixels,
  upgrade,
  urgency,
  uri,
  user_agent,
  variant_vary,
  vary,
  vbr_info,
  version,
  via,
  want_digest,
  warning,
  www_authenticate,
  x_archived_at,
  x_device_accept,
  x_device_accept_charset,
  x_device_accept_encoding,
  x_device_accept_language,
  x_device_user_agent,
  x_frame_options,
  x_mittente,
  x_pgp_sig,
  x_ricevuta,
  x_riferimento_message_id,
  x_tiporicevuta,
  x_trasporto,
  x_verificasicurezza,
  x400_content_identifier,
  x400_content_return,
  x400_content_type,
  x400_mts_identifier,
  x400_originator,
  x400_received,
  x400_recipients,
  x400_trace,
  xref,
};

enum state
{
  method_start,
  method,
  uri,
  http_version_h,
  http_version_t_1,
  http_version_t_2,
  http_version_p,
  http_version_slash,
  http_version_major_start,
  http_version_major,
  http_version_minor_start,
  http_version_minor,
  expecting_newline_1,
  header_line_start,
  header_lws,
  header_name,
  space_before_header_value,
  header_value,
  expecting_newline_2,
  expecting_newline_3
};

class HttpMessage 
{
public:
  
  void SetHeader(std::string name, std::string value);
  void Body(std::string& body);
  std::string& Body();
  void KeepAlive(bool keep_alive);
  bool KeepAlive();
  void Version(std::string& version);
  std::string& Version();
  
protected:
  HttpMessage() = default;
  struct header
  {
    header() = default;
    header(std::string& _name, std::string& _value):name(_name),value(_value){}
    std::string name;
    std::string value;
  };
  std::string version_;
  std::vector<header> headers_;
  std::string body_;
  bool keep_alive_;
};

class Request :public HttpMessage
{
public:
  Request() = default;
  Request(char *req_string, size_t bytes);
  void SetMethod(Method::method_t method);
  std::string& method();
  std::string& url();
  bool Is_request() const{
    return is_request;
  }
protected:
  bool is_request;
  std::string method_;
  int consume(char input);
  state state_;
  std::string url_;

};

class Respond :public HttpMessage
{
public:
  void SetBodyFromFile(std::string file_name);
  void SetStatus(Status status);
  std::vector < boost::asio::const_buffer> to_buffers() {
    std::cout << "body:" << body_ << std::endl;
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(body_));
    return buffers;
  }
protected:
  std::string status_;
  
};


} // http
} // communicate
} // cgvp

#endif//CGVP_CORE_MAIN_HTTP_MESSAGE_H_
