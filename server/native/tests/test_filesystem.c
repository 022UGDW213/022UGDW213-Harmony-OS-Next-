#include "rust/rust_fs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void test_filesystem() {
    printf("📂 Filesystem Test (RamFS in Rust)\n");
    printf("==================================\n\n");
    
    rust_fs_init();
    
    // Test 1: Create/Write File
    printf("🧪 Test 1: Write File 'hello.txt'\n");
    printf("----------------------------------\n");
    
    int fd = rust_fs_open("/hello.txt", O_CREAT | O_WRONLY);
    if (fd > 0) {
        printf("✅ Opened file 'hello.txt', fd=%d\n", fd);
        const char* data = "Hello, Virtual Filesystem!";
        ssize_t written = rust_fs_write(fd, data, strlen(data));
        if (written == (ssize_t)strlen(data)) {
            printf("✅ Wrote %zd bytes\n", written);
        } else {
            printf("❌ Write failed\n");
        }
        rust_fs_close(fd);
    } else {
        printf("❌ Failed to open file\n");
    }
    
    // Test 2: List Directory
    printf("\n🧪 Test 2: Directory Listing\n");
    printf("-----------------------------\n");
    rust_fs_ls("/");
    
    // Test 3: Read Back
    printf("\n🧪 Test 3: Read File 'hello.txt'\n");
    printf("---------------------------------\n");
    fd = rust_fs_open("/hello.txt", O_RDONLY);
    if (fd > 0) {
        char buf[128] = {0};
        ssize_t n = rust_fs_read(fd, buf, sizeof(buf));
        if (n > 0) {
            printf("✅ Read content: \"%s\"\n", buf);
        } else {
            printf("❌ Read failed or empty\n");
        }
        rust_fs_close(fd);
    } else {
        printf("❌ Failed to re-open file\n");
    }
    
    printf("\n✅ Filesystem tests completed!\n");
}

int main() {
    test_filesystem();
    return 0;
}
