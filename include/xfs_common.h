#ifndef __XFS_COMMON_H__
#define __XFS_COMMON_H__

#include <stdint.h>
#include <stdio.h>

#ifndef S_IFMT
#define S_IFMT 00170000
#endif
#ifndef S_IFSOCK
#define S_IFSOCK 0140000
#endif
#ifndef S_IFLNK
#define S_IFLNK 0120000
#endif
#ifndef S_IFREG
#define S_IFREG 0100000
#endif
#ifndef S_IFBLK
#define S_IFBLK 0060000
#endif
#ifndef S_IFDIR
#define S_IFDIR 0040000
#endif
#ifndef S_IFCHR
#define S_IFCHR 0020000
#endif
#ifndef S_IFIFO
#define S_IFIFO 0010000
#endif
#ifndef S_ISUID
#define S_ISUID 0004000
#endif
#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX 0001000
#endif

#ifndef S_ISLNK
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#endif
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(m)  (((m) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)  (((m) & S_IFMT) == S_IFSOCK)
#endif

#define MY_UUID_SIZE 37
typedef char my_uuid_t[MY_UUID_SIZE];

typedef signed char __s8;
typedef unsigned char __u8;
typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint64_t __be64;

typedef uint32_t __le32;

typedef struct xfs_timestamp {
    __be32 sec;
    __be32 nsec;
}xfs_timestamp_t;

typedef enum {
    XFS_UNKNOW_ENDIAN = 0x00,
    XFS_LIT_ENDIAN = 0x01,
    XFS_BIG_ENDIAN = 0x02
}XFS_ENDIAN_ENUM;

// 0xABCD or 0xCDAB
#define xfs_getu16(endian, x) \
(uint16_t)(((endian) == XFS_LIT_ENDIAN) ? \
            (((uint8_t *)(x))[0] + (((uint8_t *)(x))[1] << 8)) : \
            (((uint8_t *)(x))[1] + (((uint8_t *)(x))[0] << 8)))

// signed
#define xfs_gets16(endian, x) \
((int16_t)xfs_getu16(endian, x))

// 0xABCDEF or 0xEFCDAB
#define xfs_getu24(endian, x) \
(uint32_t)(((endian) == XFS_LIT_ENDIAN) ? \
            (((uint8_t *)(x))[0] + (((uint8_t *)(x))[1] << 8) + (((uint8_t *)(x))[2] << 16)) : \
            (((uint8_t *)(x))[2] + (((uint8_t *)(x))[1] << 8) + (((uint8_t *)(x))[0] << 16)))

// signed
#define xfs_gets24(endian, x) \
((int32_t)xfs_getu24(endian, x))

// 0x89ABCDEF or 0xEFCDAB89
#define xfs_getu32(endian, x) \
(uint32_t)(((endian) == XFS_LIT_ENDIAN) ? \
            ((((uint8_t *)(x))[0] << 0) + \
             (((uint8_t *)(x))[1] << 8) + \
             (((uint8_t *)(x))[2] << 16) + \
             (((uint8_t *)(x))[3] << 24)) : \
            ((((uint8_t *)(x))[3] << 0) + \
             (((uint8_t *)(x))[2] << 8) + \
             (((uint8_t *)(x))[1] << 16) + \
             (((uint8_t *)(x))[0] << 24)))

#define xfs_gets32(endian, x) \
((int32_t)xfs_getu32(endian, x))

// 0x456789ABCDEF or 0xEFCDAB896745
#define xfs_getu48(endian, x)   \
(uint64_t)( ((endian) == XFS_LIT_ENDIAN)  ?	\
            ((uint64_t) \
             ((uint64_t)((uint8_t *)(x))[0] <<  0)+ \
             ((uint64_t)((uint8_t *)(x))[1] <<  8) + \
             ((uint64_t)((uint8_t *)(x))[2] << 16) + \
             ((uint64_t)((uint8_t *)(x))[3] << 24) + \
             ((uint64_t)((uint8_t *)(x))[4] << 32) + \
             ((uint64_t)((uint8_t *)(x))[5] << 40)) \
                                          : \
            ((uint64_t) \
             ((uint64_t)((uint8_t *)(x))[5] <<  0)+ \
             ((uint64_t)((uint8_t *)(x))[4] <<  8) + \
             ((uint64_t)((uint8_t *)(x))[3] << 16) + \
             ((uint64_t)((uint8_t *)(x))[2] << 24) + \
             ((uint64_t)((uint8_t *)(x))[1] << 32) + \
             ((uint64_t)((uint8_t *)(x))[0] << 40)) )

// 0x0123456789ABCDEF or 0xEFCDAB8967452301
#define xfs_getu64(endian, x)   \
(uint64_t)( ((endian) == XFS_LIT_ENDIAN)  ?	\
            ((uint64_t) \
             ((uint64_t)((uint8_t *)(x))[0] << 0)  + \
             ((uint64_t)((uint8_t *)(x))[1] << 8) + \
             ((uint64_t)((uint8_t *)(x))[2] << 16) + \
             ((uint64_t)((uint8_t *)(x))[3] << 24) + \
             ((uint64_t)((uint8_t *)(x))[4] << 32) + \
             ((uint64_t)((uint8_t *)(x))[5] << 40) + \
             ((uint64_t)((uint8_t *)(x))[6] << 48) + \
             ((uint64_t)((uint8_t *)(x))[7] << 56)) \
                                          : \
            ((uint64_t) \
             ((uint64_t)((uint8_t *)(x))[7] <<  0) + \
             ((uint64_t)((uint8_t *)(x))[6] <<  8) + \
             ((uint64_t)((uint8_t *)(x))[5] << 16) + \
             ((uint64_t)((uint8_t *)(x))[4] << 24) + \
             ((uint64_t)((uint8_t *)(x))[3] << 32) + \
             ((uint64_t)((uint8_t *)(x))[2] << 40) + \
             ((uint64_t)((uint8_t *)(x))[1] << 48) + \
             ((uint64_t)((uint8_t *)(x))[0] << 56)) )

#define xfs_gets64(endian, x) \
((int64_t)xfs_getu64(endian, x))

#define DEBUG

#ifdef DEBUG
#define Dprintf(arg...) {\
printf("[debug]:%s:%s:%d --> ", __FILE__, __FUNCTION__, __LINE__);\
printf(arg);\
fflush(stdout);\
}
#else
#define Dprintf(arg...) {}
#endif

#endif // __XFS_COMMON_H__