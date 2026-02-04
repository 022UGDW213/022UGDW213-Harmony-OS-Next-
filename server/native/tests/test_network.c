#include "rust/rust_net.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test_network_stack() {
    printf("🌐 Network Stack Test (Rust)\n");
    printf("===========================\n\n");
    
    // Initialize stack
    rust_net_init();
    printf("✅ Network stack initialized\n");
    
    // Test 1: Create Socket
    printf("\n🧪 Test 1: Create TCP Socket\n");
    printf("-----------------------------\n");
    Socket* sock = rust_socket_create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock) {
        printf("✅ Socket created successfully\n");
    } else {
        printf("❌ Failed to create socket\n");
        return;
    }
    
    // Test 2: Bind
    printf("\n🧪 Test 2: Bind Socket\n");
    printf("-----------------------\n");
    // Bind to 127.0.0.1 (0x7F000001) port 8080
    if (rust_socket_bind(sock, 0x7F000001, 8080) == 0) {
        printf("✅ Socket bound to 127.0.0.1:8080\n");
    } else {
        printf("❌ Failed to bind socket\n");
    }
    
    // Test 3: Connect (Mock)
    printf("\n🧪 Test 3: Connect (Loopback Mock)\n");
    printf("----------------------------------\n");
    // Connect to "remote" 127.0.0.1:9090
    if (rust_socket_connect(sock, 0x7F000001, 9090) == 0) {
        printf("✅ Socket connected\n");
    } else {
        printf("❌ Failed to connect\n");
    }
    
    // Test 4: Send/Recv Loopback
    printf("\n🧪 Test 4: Send/Recv Loopback\n");
    printf("------------------------------\n");
    const char* msg = "Hello Harmony Network!";
    char buf[256] = {0};
    
    printf("Sending: \"%s\"\n", msg);
    ssize_t sent = rust_socket_send(sock, msg, strlen(msg));
    
    if (sent == (ssize_t)strlen(msg)) {
         printf("✅ Sent %zd bytes\n", sent);
    } else {
         printf("❌ Send failed\n");
    }
    
    // Try to receive (loopback logic in Rust should return the same data)
    ssize_t received = rust_socket_recv(sock, buf, sizeof(buf));
    if (received > 0) {
        printf("Received: \"%s\"\n", buf);
        if (strcmp(msg, buf) == 0) {
             printf("✅ Loopback data verified matches\n");
        } else {
             printf("❌ Data mismatch\n");
        }
    } else {
        printf("❌ Recv failed or no data\n");
    }
    
    // Test 5: Close
    printf("\n🧪 Test 5: Close Socket\n");
    printf("------------------------\n");
    rust_socket_close(sock);
    printf("✅ Socket closed\n");
    
    printf("\n✅ All network tests completed!\n");
}

int main() {
    test_network_stack();
    return 0;
}
