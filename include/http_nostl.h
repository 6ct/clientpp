#pragma once
#include <Windows.h>

#ifndef WH_USE_WININET
#define WH_INTERNET(X) WinHttp##X
#define WH_INTERNETW(X) WinHttp##X
#define WH_HTTP(X) WinHttp##X
#define WH_HTTPW(X) WinHttp##X
#define WH_INTERNET_CONST(X) WINHTTP_##X
#define WH_HTTP_CONST(X) WINHTTP_##X
#define WH_WININET_ARGS(...)
#define WH_WINHTTP_ARGS(...) , __VA_ARGS__
#pragma comment(lib, "winhttp.lib")
#include <winhttp.h>
#else
#define WH_INTERNET(X) Internet##X
#define WH_INTERNETW(X) Internet##X##W
#define WH_HTTP(X) Http##X
#define WH_HTTPW(X) Http##X##W
#define WH_INTERNET_CONST(X) INTERNET_##X
#define WH_HTTP_CONST(X) HTTP_##X
#define WH_WININET_ARGS(...) , __VA_ARGS__
#define WH_WINHTTP_ARGS(...)
#pragma comment(lib, "wininet.lib")
#include <WinInet.h>
#endif

namespace http {
namespace nostl {

class request_t;
class response_t;

void safe_free(void *s);
void safe_delete(void *s);

char *format_last_error(const char *msg);

enum option_t {
  option_allow_unknown_cert_authority = 0,
  option_allow_invalid_cert_name,
  option_allow_invalid_cert_date
};

class handle_manager_t {
public:
  handle_manager_t();
  handle_manager_t(HINTERNET h);
  handle_manager_t(const handle_manager_t &other) = delete;
  virtual ~handle_manager_t();
  inline HINTERNET handle() const { return handle_; };
  inline void set_handle(HINTERNET h) { handle_ = h; };
  inline operator HINTERNET() const { return handle_; };

protected:
  HINTERNET handle_;
};

class error_handler_t {
public:
  error_handler_t() : ok_(true), error_(nullptr) {}
  virtual ~error_handler_t() { safe_free(error_); }
  inline bool ok() const { return ok_; }
  inline const char *error() const { return error_; }
  inline void set_error(const char *msg) {
    safe_free(error_);
    error_ = _strdup(msg);
  }

protected:
  bool ok_;
  char *error_;
};

class session_t : public handle_manager_t, public error_handler_t {
public:
  session_t(const char *user_agent);
  ~session_t();
};

class connection_t : public handle_manager_t, public error_handler_t {
public:
  connection_t(const session_t &sess, const char *host);
  virtual ~connection_t();
  response_t send(const request_t &req);
  unsigned int flags() const { return flags_; }
  inline unsigned int timeout() const { return timeout_; }
  void set_option(option_t opt, bool on);
  inline void set_timeout(unsigned int seconds) { timeout_ = seconds; }

private:
  wchar_t *host_;
  URL_COMPONENTSW components_;
  unsigned int flags_;
  unsigned int timeout_;
};

class request_t {
  friend class connection_t;

public:
  struct header_line {
    header_line(wchar_t *line);
    ~header_line();
    header_line *next_;
    wchar_t *line_;
  };

  request_t(const char *method, const char *url);
  virtual ~request_t();
  void set_body(const char *data, size_t length);
  void add_header(const char *line);
  void set_option(option_t opt, bool on);

private:
  wchar_t *method_;
  wchar_t *url_;
  char *body_;
  size_t body_length_;
  header_line *headers_head_;
  header_line *headers_tail_;
  unsigned int flags_;
};

class response_t : public handle_manager_t, public error_handler_t {
  friend class connection_t;

private:
  response_t(HINTERNET request_t);

public:
  response_t(const response_t &other) = delete;
  response_t(response_t &&other);
  virtual ~response_t();
  inline const response_t &operator=(const response_t &other) = delete;
  inline const response_t &operator=(const response_t &&other) = delete;
  inline int status() const { return status_; }
  inline bool succeeded() const { return status_ >= 200 && status_ < 300; }
  inline bool failed() const { return !succeeded(); }
  bool read(char *buffer, size_t count, size_t *bytes_read);

private:
  int status_;
};

} // namespace nostl

} // namespace http
