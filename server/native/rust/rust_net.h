#ifndef HARMONY_RUST_NET_H
#define HARMONY_RUST_NET_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque socket pointer
typedef struct Socket Socket;

// Protocol families (simplified)
#define AF_INET 2

// Socket types
#define SOCK_STREAM 1
#define SOCK_DGRAM 2

// Protocols
#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

// Initialization
void rust_net_init(void);

// Socket API
Socket* rust_socket_create(int domain, int type, int protocol);
int rust_socket_bind(Socket* socket, uint32_t addr, uint16_t port);
int rust_socket_listen(Socket* socket, int backlog);
int rust_socket_connect(Socket* socket, uint32_t addr, uint16_t port);
ssize_t rust_socket_send(Socket* socket, const void* buf, size_t len);
ssize_t rust_socket_recv(Socket* socket, void* buf, size_t len);
void rust_socket_close(Socket* socket);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_RUST_NET_H
