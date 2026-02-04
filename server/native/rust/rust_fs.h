#ifndef HARMONY_RUST_FS_H
#define HARMONY_RUST_FS_H

#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// Flags (Simulated)
#ifndef O_CREAT
#define O_CREAT 1
#endif
#ifndef O_RDONLY
#define O_RDONLY 2
#endif
#ifndef O_WRONLY
#define O_WRONLY 4
#endif

// API
int rust_fs_init(void);
int rust_fs_open(const char* path, int flags);
ssize_t rust_fs_read(int fd, void* buf, size_t len);
ssize_t rust_fs_write(int fd, const void* buf, size_t len);
int rust_fs_close(int fd);
int rust_fs_ls(const char* path);

#ifdef __cplusplus
}
#endif

#endif // HARMONY_RUST_FS_H
