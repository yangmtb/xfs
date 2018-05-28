#ifndef __XFS_INODE_H__
#define __XFS_INODE_H__

#include "xfs_common.h"
#include "xfs_sb.h"

#define XFS_INODE_MAGIC 0x494e // 'IN'
#define XFS_DIR3_BLOCK_MAGIC 0x58444233 // 'XDB3': single block dirs
#define XFS_DIR3_DATA_MAGIC 0x58444433 // 'XDD3': multiblock dirs
#define XFS_DIR3_FREE_MAGIC 0x58444633 // 'XDF3': free index blocks

typedef enum xfs_dinode_fmt {
    XFS_DINODE_FMT_DEV,   // xfs_dev_t
    XFS_DINODE_FMT_LOCAL, // bulk data
    XFS_DINODE_FMT_EXTENTS, // struct xfs_bmbt_rec
    XFS_DINODE_FMT_BTREE, // struct xfs_bmdr_block
    XFS_DINODE_FMT_UUID // added long ago, but never used
}xfs_dinode_fmt_t;

typedef struct xfs_dir3_blk_hdr {
    __be32 magic;
    __be32 crc;
    __be64 blkno;
    __be64 lsn;
    my_uuid_t uuid;
    __be64 owner;
}xfs_dir3_blk_hdr_t;

typedef struct xfs_dir2_data_free {
    __be16 offset;
    __be16 length;
}xfs_dir2_data_free_t;

typedef struct xfs_dir3_data_hdr {
    xfs_dir3_blk_hdr_t hdr;
    xfs_dir2_data_free_t best_free[3];
    __be32 pad;
}xfs_dir3_data_hdr_t;

typedef struct xfs_dir3_entry {
    __be64 inumber;
    __u8 namelen;
    char *name; // namelen
    //char name[256]; // namelen
    __u8 filetype;
    char *pad; // alinment 8 byte; (19+namelen)&0xFFF8 - (12+namelen)
    __be16 tag;
}xfs_dir3_entry_t;

typedef struct {
    __u8 namelen;
    __be16 offset;
    char *name; // namelen
    __u8 filetype;
    __be32 inumber;
}inline_list_t;

typedef enum dir_sub_type {
    DIR_SUB_FILE = 1,
    DIR_SUB_DIR = 2,
    DIR_SUB_LNK = 7
}dir_sub_type_t;

typedef struct {
    __u8 extentflag;  // 01 bit
    __be64 startoff;  // 54 bit
    __be64 startblock; // 52 bit
    __be32 blockcount; // 21 bit
}ext_info_t;

typedef struct xfs_inode {
    __be16 magic;
    __be16 type;   // 7 bit
    __be16 mode; // 9 bit
    __u8 version;
    __u8 format;
    __be16 onlink;
    __be32 uid;
    __be32 gid;
    __be32 nlink;
    __be16 projid_lo;
    __be16 projid_hi;
    __u8 pad[6];
    __be16 flushiter;
    xfs_timestamp_t atime;
    xfs_timestamp_t mtime;
    xfs_timestamp_t ctime;
    __be64 size;
    __be64 nblocks;
    __be32 extsize;
    __be32 nextents;
    __be16 anextents;
    __u8 forkoff;
    __s8 aformat;
    __be32 dmevmask;
    __be16 dmstate;
    __be16 flags;
    __be32 gen;

    __be32 next_unlinked;
    
    __be32 crc;
    __be64 changecount;
    __be64 lsn;
    __be64 flags2;
    __be32 cowextsize;
    __u8 pad2[12];
    xfs_timestamp_t crtime;
    __be64 ino;
    my_uuid_t uuid;

    // extent (file always)
    ext_info_t *exts;
    int extcount;
    // dir (inline)
    struct {
        __u8 count;
        __u8 i8count;
        __be32 i4;
    }u3;
    inline_list_t *list; // u3.count
    // dir extent
    xfs_dir3_data_hdr_t dirhdr;
    xfs_dir3_entry_t *entry; // util 0xffff
    // lnk file
    char *lnk_path;
}xfs_inode_t;

typedef struct xfs_dir_ext {
    xfs_dir3_data_hdr_t hdr;
    xfs_dir3_entry_t *entrys;
    int subcount;
}xfs_dir_ext_t;

extern int blkbb;

#define BBSHIFT 9

#define XFS_FSB_TO_AGNO(sb, fsbno) \
    ((uint32_t)((fsbno) >> sb->sb_agblklog))
#define XFS_FSB_TO_AGBNO(sb, fsbno) \
    ((uint32_t)((fsbno) & (((uint32_t)1 << sb->sb_agblklog) - 1)))
#define XFS_FSB_TO_BB(sb, fsbno)  ((fsbno) << (sb->sb_blocklog - BBSHIFT))

#define XFS_AGB_TO_DADDR(sb, agno, agbno) \
    ((int64_t)XFS_FSB_TO_BB(sb, (uint64_t)(agno)*(sb->sb_agblocks) + (agbno)))

#define XFS_INO_MASK(k)  (uint32_t)((1ULL << (k)) - 1)
#define XFS_INO_OFFSET_BITS(sb) (sb)->sb_inopblog

#define XFS_INO_AGINO_BITS(sb) ((sb)->sb_agblklog + (sb)->sb_inopblog)
#define XFS_INO_AGBNO_BITS(sb) (sb)->sb_agblklog

#define XFS_INO_TO_AGNO(sb, i) \
    ((uint32_t)((i) >> XFS_INO_AGINO_BITS(sb)))
#define XFS_INO_TO_AGINO(sb, i) \
    ((uint32_t)(i) & XFS_INO_MASK(XFS_INO_AGINO_BITS(sb)))
#define XFS_INO_TO_AGBNO(sb, i) \
    (((uint32_t)(i) >> XFS_INO_OFFSET_BITS(sb)) & \
    XFS_INO_MASK(XFS_INO_AGBNO_BITS(sb)))
    
#define XFS_AGINO_TO_INO(sb, a, i) \
    (((uint64_t)(a) << XFS_INO_AGINO_BITS(sb)) | (i))
#define XFS_AGINO_TO_AGBNO(sb, i) ((i) >> XFS_INO_OFFSET_BITS(sb))
#define XFS_AGINO_TO_OFFSET(sb, i) \
    ((i) & XFS_INO_MASK(XFS_INO_OFFSET_BITS(sb)))

// error:1 success:0 return value in offset
extern int get_inode_offset_by_ino(const xfs_dsb_t *sb, const uint64_t ino, int64_t *a_offset);
extern int get_block_offset_by_blknum(const xfs_dsb_t *sb, const uint64_t blknum, int64_t *a_offset);
extern int parse_buff_to_inode(xfs_inode_t *inode, const char *buf, const XFS_ENDIAN_ENUM end);
extern int parse_buff_to_dir_ext(xfs_dir_ext_t *dir_ext, const char *buf, const uint64_t buf_size, const XFS_ENDIAN_ENUM end);
extern void free_inode(xfs_inode_t *inode);
extern void free_dir_ext(xfs_dir_ext_t *dir_ext);

#endif // __XFS_INODE_H__