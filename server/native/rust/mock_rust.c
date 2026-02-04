#include "rust/rust_fs.h"
#include "rust/rust_net.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Mock FS Implementation ---
int rust_fs_init(void) {
    printf("   [Rust Mock] FS Initialized (RamFS)\n");
    return 0;
}

int rust_fs_open(const char* path, int flags) {
    (void)flags;
    printf("   [Rust Mock] open(%s) -> fd 100\n", path);
    return 100;
}

ssize_t rust_fs_read(int fd, void* buf, size_t len) {
    printf("   [Rust Mock] read(%d, %zu)\n", fd, len);
    const char* data = "Mock Data";
    size_t copy_len = len < strlen(data) ? len : strlen(data);
    memcpy(buf, data, copy_len);
    return copy_len;
}

ssize_t rust_fs_write(int fd, const void* buf, size_t len) {
    (void)buf;
    printf("   [Rust Mock] write(%d, %zu)\n", fd, len);
    return len;
}

int rust_fs_close(int fd) {
    printf("   [Rust Mock] close(%d)\n", fd);
    return 0;
}

int rust_fs_ls(const char* path) {
    printf("   [Rust Mock] ls %s\n", path);
    return 0;
}

// --- Mock Net Implementation ---
void rust_net_init(void) {
    printf("   [Rust Mock] Net Initialized (Stack)\n");
}

Socket* rust_socket_create(int domain, int type, int protocol) {
    (void)domain; (void)type; (void)protocol;
    return (Socket*)0xDEADBEEF;
}

int rust_socket_bind(Socket* socket, uint32_t addr, uint16_t port) {
    (void)socket; (void)addr; (void)port;
    return 0;
}

int rust_socket_listen(Socket* socket, int backlog) {
    (void)socket; (void)backlog;
    return 0;
}

int rust_socket_connect(Socket* socket, uint32_t addr, uint16_t port) {
    (void)socket; (void)addr; (void)port;
    return 0;
}

ssize_t rust_socket_send(Socket* socket, const void* buf, size_t len) {
    (void)socket; (void)buf;
    return len;
}

ssize_t rust_socket_recv(Socket* socket, void* buf, size_t len) {
    (void)socket; (void)buf;
    return 0;
}

void rust_socket_close(Socket* socket) {
    (void)socket;
}
