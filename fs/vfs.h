#ifndef VFS_H
#define VFS_H

/*
 * AstraOS Virtual Filesystem (VFS)
 *
 * Stub interface for the filesystem abstraction layer.
 *
 * Future implementation will provide:
 *   - VFS node (vnode) abstraction over concrete filesystems
 *   - Mount table management
 *   - File descriptor table per process
 *   - POSIX-like operations: open, read, write, close, stat
 *   - Initial drivers: initramfs (in-memory), later ext2 on disk
 */

#include <stddef.h>
#include <stdint.h>

/* Opaque file descriptor */
typedef int32_t fd_t;

/* Initialise the VFS layer (no-op in v0.1 stub) */
void vfs_init(void);

#endif /* VFS_H */
