#include "mach/port.h"
#include "mach/message.h"
#include <stdio.h>
#include <string.h>

void mach_init(void); //Forward declaration

void test_mach_ipc() {
    printf("🍎 Mach IPC Test\n");
    printf("================\n");
    
    mach_init();
    
    // 1. Create Ports
    printf("🧪 Test 1: Port Allocation\n");
    mach_port_t port_a = mach_port_allocate();
    mach_port_t port_b = mach_port_allocate();
    
    if (port_a != MACH_PORT_NULL && port_b != MACH_PORT_NULL && port_a != port_b) {
        printf("✅ Ports allocated: %d, %d\n", port_a, port_b);
    } else {
        printf("❌ Port allocation failed\n");
        return;
    }
    
    // 2. Send Message (A -> B)
    printf("\n🧪 Test 2: Message Passing\n");
    
    struct {
        mach_msg_header_t header;
        char body[64];
    } msg_send;
    
    msg_send.header.msgh_bits = MACH_SEND_MSG;
    msg_send.header.msgh_size = sizeof(msg_send);
    msg_send.header.msgh_remote_port = port_b;
    msg_send.header.msgh_local_port = port_a;
    msg_send.header.msgh_id = 1001;
    strcpy(msg_send.body, "Hello from Mach!");
    
    int err = mach_msg_send(&msg_send.header);
    if (err == MACH_MSG_SUCCESS) {
        printf("✅ Message sent successfully\n");
    } else {
        printf("❌ Message send failed: %d\n", err);
    }
    
    // 3. Receive Message (on B)
    struct {
        mach_msg_header_t header;
        char body[64];
    } msg_recv;
    
    msg_recv.header.msgh_size = sizeof(msg_recv);
    
    err = mach_msg_receive(&msg_recv.header, port_b);
    if (err == MACH_MSG_SUCCESS) {
        printf("✅ Message received\n");
        printf("   ID: %d\n", msg_recv.header.msgh_id);
        printf("   Body: %s\n", msg_recv.body);
        
        if (strcmp(msg_recv.body, "Hello from Mach!") == 0) {
            printf("✅ Data integrity verified\n");
        } else {
            printf("❌ Data corruption\n");
        }
    } else {
        printf("❌ Message receive failed: %d\n", err);
    }
    
    // 4. Cleanup
    mach_port_deallocate(port_a);
    mach_port_deallocate(port_b);
}

int main() {
    test_mach_ipc();
    return 0;
}
