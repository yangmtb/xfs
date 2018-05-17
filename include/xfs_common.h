#ifndef __XFS_COMMON_H__
#define __XFS_COMMON_H__

#include <stdint.h>

#define MY_UUID_SIZE 37
typedef char my_uuid_t[MY_UUID_SIZE];

typedef unsigned char __u8;
typedef uint16_t __be16;
typedef uint32_t __be32;
typedef uint64_t __be64;

typedef uint32_t __le32;

#endif // __XFS_COMMON_H__