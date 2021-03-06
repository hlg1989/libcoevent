// file encoding: UTF-8

#ifndef __CO_EVENT_ITNL_H__
#define __CO_EVENT_ITNL_H__

#include "co_routine_inner.h"
#include "co_routine.h"
#include "cpp_tools.h"
#include "coevent.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <stdint.h>

namespace andrewmc {
namespace libcoevent {

// forever
#define FOREVER_SECONDS         (3600*24*365*10)

// print()
#define CFG_PRINT_BUFF_SIZE     (1024)
ssize_t print(int fd, const char *format, ...);

#if DEBUG_FLAG
#define DEBUG(fmt, args...)   print(1, "DEBUG - %s, %s(), %d: "fmt, __FILE__, __func__, __LINE__, ##args)
#define INFO(fmt, args...)    print(1, "INFO  - %s, %s(), %d: "fmt, __FILE__, __func__, __LINE__, ##args)
#define ERROR(fmt, args...)   print(2, "ERROR - %s, %s(), %d: "fmt, __FILE__, __func__, __LINE__, ##args)
#else
#define DEBUG(fmt, args...)
#define INFO(fmt, args...)    print(1, fmt, ##args)
#define ERROR(fmt, args...)   print(2, fmt, ##args)
#endif

// to_timeval()
struct timeval to_timeval(double seconds);
struct timeval to_timeval(float seconds);
struct timeval to_timeval(int seconds);
struct timeval to_timeval(long seconds);
struct timeval to_timeval_from_milisecs(unsigned milisecs);

// to_double()
double to_double(struct timeval &time);

// libco ext
BOOL is_coroutine_end(const struct stCoRoutine_t *routine);
BOOL is_coroutine_started(const struct stCoRoutine_t *routine);

// fd settings
int set_fd_nonblock(int fd);
int set_fd_reuseaddr(int fd);

// recvfrom()
ssize_t recv_from(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

// libevent flag check
BOOL event_is_timeout(uint32_t libevent_what);
BOOL event_readable(uint32_t libevent_what);
BOOL event_writable(uint32_t libevent_what);
BOOL event_got_signal(uint32_t libevent_what);

// struct sockaddr conversion
void convert_str_to_sockaddr_in(const std::string &str, unsigned port, struct sockaddr_in *addr_out);
void convert_str_to_sockaddr_in6(const std::string &str, unsigned port, struct sockaddr_in6 *addr_out);
void convert_str_to_sockaddr_un(const std::string &str, struct sockaddr_un *addr_out);

// sockaddr to string
std::string str_from_sin_addr(const struct in_addr *addr);
std::string str_from_sin6_addr(const struct in6_addr *addr6);

// Actual implementation of UDPClient
class UDPItnlClient : public UDPClient
{
protected:
    void            *_event_arg;
    int             _fd_ipv4;
    int             _fd_ipv6;
    int             _fd_unix;
    int             _fd;
    uint32_t        *_libevent_what_storage;    // ensure that this is assigned in heap instead of stack
    struct sockaddr_in  _remote_addr_ipv4;
    struct sockaddr_in6 _remote_addr_ipv6;
    struct sockaddr_un  _remote_addr_unix;
    socklen_t       _remote_addr_len;
    Procedure       *_owner_server;

public:
    UDPItnlClient();
    virtual ~UDPItnlClient();

    struct Error init(Procedure *server, struct stCoRoutine_t *coroutine, NetType_t network_type, void *user_arg = NULL);
    NetType_t network_type();

    struct Error send(const void *data, const size_t data_len, size_t *send_len_out_nullable, const struct sockaddr *addr, socklen_t addr_len);
    struct Error send(const void *data, const size_t data_len, size_t *send_len_out_nullable, const std::string &target_address, unsigned target_port = 80);
    struct Error send(const void *data, const size_t data_len, size_t *send_len_out_nullable, const char *target_address = "", unsigned target_port = 80);
    struct Error reply(const void *data, const size_t data_len, size_t *send_len_out_nullable = NULL);

    struct Error recv(void *data_out, const size_t len_limit, size_t *len_out_nullable, double timeout_seconds = 0);
    struct Error recv_in_timeval(void *data_out, const size_t len_limit, size_t *len_out_nullable, const struct timeval &timeout);
    struct Error recv_in_mimlisecs(void *data_out, const size_t len_limit, size_t *len_out_nullable, unsigned timeout_milisecs);

    std::string default_dns_server(size_t index = 0);

    std::string remote_addr();    // valid in IPv4 or IPv6 type
    unsigned remote_port();       // valid in IPv4 or IPv6 type
    void copy_remote_addr(struct sockaddr *addr_out, socklen_t addr_len);

    Procedure *owner_server();

private:
    void _init();
    void _clear();

    struct sockaddr *_remote_addr();
};


// Actual implementation of DNSClient
class DNSItnlClient : public DNSClient {
protected:
    std::map<std::string, DNSResult *>  _dns_result;
    UDPItnlClient                       *_udp_client;
    static uint16_t                     _transaction_ID;

public:
    // construct and descruct functions
    DNSItnlClient();
    virtual ~DNSItnlClient();
    struct Error init(Procedure *server, struct stCoRoutine_t *coroutine, NetType_t network_type, void *user_arg = NULL);

    // send and receive DNS request
    NetType_t network_type();
    struct Error resolve(const std::string &domain_name, double timeout_seconds = 0, const std::string &dns_server_ip = "");
    struct Error resolve_in_timeval(const std::string &domain_name, const struct timeval &timeout, const std::string &dns_server_ip = "");
    struct Error resolve_in_milisecs(const std::string &domain_name, unsigned timeout_milisecs, const std::string &dns_server_ip = "");

    // read default DNS server configured in syste
    std::string default_dns_server(size_t index = 0, NetType_t *network_type_out = NULL);

    // misc functions
    const DNSResult *dns_result(const std::string &domain_name);
    std::string quick_resolve(const std::string &domain_name, double timeout_seconds = 0, const std::string &dns_server_ip = "");
    Procedure *owner_server();

    // remote addr
    std::string remote_addr();    // valid in IPv4 or IPv6 type
    unsigned remote_port();       // valid in IPv4 or IPv6 type
    void copy_remote_addr(struct sockaddr *addr_out, socklen_t addr_len);

private:
    void _init();
    struct Error _send_dns_request_for(const char *c_domain_name, const struct sockaddr *addr, socklen_t addr_len);
    void _parse_dns_response(const uint8_t *c_data, size_t data_len);
};


// Actual implementation of UDPSession
class UDPItnlSession : public UDPSession {
protected:
    void        *_event_arg;
    int         _fd;
    struct sockaddr_storage _self_addr;
    struct sockaddr_storage _remote_addr;
    socklen_t   _remote_addr_len;
    uint32_t    *_libevent_what_storage;
    unsigned    _port;
    int         _server_fd;
    UDPServer   *_server;

    ::andrewmc::cpptools::Data _data_buff;
    size_t      _data_offset;
    size_t      _data_len_to_read;

    void _clear();

public:
    UDPItnlSession();
    virtual ~UDPItnlSession();

    NetType_t network_type();

    struct Error init(UDPServer *server, int server_fd, WorkerFunc func, const struct sockaddr *remote_addr, socklen_t addr_len, void *user_arg);   // auto_free is TRUE

    struct Error reply(const void *data, const size_t data_len, size_t *send_len_out_nullable = NULL);
    struct Error recv(void *data_out, const size_t len_limit, size_t *len_out_nullable, double timeout_seconds = 0);
    struct Error recv_in_timeval(void *data_out, const size_t len_limit, size_t *len_out_nullable, const struct timeval &timeout);
    struct Error recv_in_mimlisecs(void *data_out, const size_t len_limit, size_t *len_out_nullable, unsigned timeout_milisecs);

    struct Error forward_incoming_data(const void *c_data, size_t data_len);

    struct Error sleep(double seconds);
    struct Error sleep(struct timeval &sleep_time);
    struct Error sleep_milisecs(unsigned mili_secs);

    std::string remote_addr();      // valid in IPv4 or IPv6 type
    unsigned remote_port();         // valid in IPv4 or IPv6 type
    void copy_remote_addr(struct sockaddr *addr_out, socklen_t addr_len);

    UDPServer *server();

public:
    int port() const;

protected:
    struct stCoRoutine_t *_coroutine();
};


// TCP session
class TCPItnlSession : public TCPSession {
protected:
    int                     _fd;
    struct sockaddr_storage _remote_addr;
    socklen_t               _addr_len;

    TCPServer               *_server;
    uint32_t                *_libevent_what_storage;

    void                    *_event_arg;

public:
    TCPItnlSession();
    virtual ~TCPItnlSession();

    NetType_t network_type();

    struct Error init(TCPServer *server, int fd, WorkerFunc func, const struct sockaddr *remote_addr, socklen_t addr_len, void *user_arg);   // auto_free is TRUE

    struct Error reply(const void *data, const size_t data_len, size_t *send_len_out_nullable = NULL);
    struct Error recv(void *data_out, const size_t len_limit, size_t *len_out_nullable, double timeout_seconds = 0);
    struct Error recv_in_timeval(void *data_out, const size_t len_limit, size_t *len_out_nullable, const struct timeval &timeout);
    struct Error recv_in_mimlisecs(void *data_out, const size_t len_limit, size_t *len_out_nullable, unsigned timeout_milisecs);

    struct Error disconnect(void);

    struct Error sleep(double seconds);
    struct Error sleep(struct timeval &sleep_time);
    struct Error sleep_milisecs(unsigned mili_secs);

    std::string remote_addr();      // valid in IPv4 or IPv6 type
    unsigned remote_port();         // valid in IPv4 or IPv6 type
    void copy_remote_addr(struct sockaddr *addr_out, socklen_t addr_len);

    TCPServer *server();
    int file_descriptor();

private:
    void _clear();
};


// TCP client
class TCPItnlClient : public TCPClient {
protected:
    void            *_event_arg;
    int             _fd;
    struct sockaddr_storage _self_addr;
    struct sockaddr_storage _remote_addr;
    socklen_t       _addr_len;
    BOOL            _is_connected;

    Procedure       *_owner_server;
    uint32_t        *_libevent_what_storage;

public:
    TCPItnlClient();
    virtual ~TCPItnlClient();

    NetType_t network_type();

    struct Error init(Procedure *server, struct stCoRoutine_t *coroutine, NetType_t network_type, void *user_arg = NULL);

    struct Error connect_to_server(const struct sockaddr *addr, socklen_t addr_len, double timeout_seconds = 0);
    struct Error connect_to_server(const std::string &target_address = "", unsigned target_port = 80, double timeout_seconds = 0);
    struct Error connect_to_server(const char *target_address = "", unsigned target_port = 80, double timeout_seconds = 0);

    struct Error connect_in_timeval(const struct sockaddr *addr, socklen_t addr_len, const struct timeval &timeout);
    struct Error connect_in_timeval(const std::string &target_address, unsigned target_port, const struct timeval &timeout);
    struct Error connect_in_timeval(const char *target_address, unsigned target_port, const struct timeval &timeout);

    struct Error connect_in_mimlisecs(const struct sockaddr *addr, socklen_t addr_len, unsigned timeout_milisecs);
    struct Error connect_in_mimlisecs(const std::string &target_address, unsigned target_port, unsigned timeout_milisecs);
    struct Error connect_in_mimlisecs(const char *target_address, unsigned target_port, unsigned timeout_milisecs);

    struct Error send(const void *data, const size_t data_len, size_t *send_len_out_nullable = NULL);

    struct Error recv(void *data_out, const size_t len_limit, size_t *len_out_nullable, double timeout_seconds);
    struct Error recv_in_timeval(void *data_out, const size_t len_limit, size_t *len_out_nullable, const struct timeval &timeout);
    struct Error recv_in_mimlisecs(void *data_out, const size_t len_limit, size_t *len_out_nullable, unsigned timeout_milisecs);

    std::string remote_addr();    // valid in IPv4 or IPv6 type
    unsigned remote_port();       // valid in IPv4 or IPv6 type
    void copy_remote_addr(struct sockaddr *addr_out, socklen_t addr_len);

    Procedure *owner_server();

private:
    void _clear();
};

}   // end of namespace libcoevent
}   // end of namespace andrewmc
#endif  // EOF
