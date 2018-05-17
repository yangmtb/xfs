#ifndef __SB_H__
#define __SB_H__

#include "xfs_common.h"

#define XFS_SB_MAGIC 0x58465342 // 'XFSB'

// superblock - on disk version. must match the in core version above.
// must be padded to 64 bit alignment
typedef struct xfs_dsb {
    __be32 sb_magicnum; // macgic number == XFS_SB_MAGIC
    __be32 sb_blocksize; // logical block size bytes
    __be64 sb_dblocks; // number of data blocks
    __be64 sb_rblocks; // number of realtime blocks
    __be64 sb_rextents; // number of realtime extents
    my_uuid_t sb_uuid; // user-visible file system unique id
    __be64 sb_logstart; // starting block of log if internal
    __be64 sb_rootino; // root inode number
    __be64 sb_rbmino; // bitmap inode for realtime extents
    __be64 sb_rsumino; // summary inode for rt bitmap
    __be32 sb_rextsize; // realtime extent size, blocks
    __be32 sb_agblocks; // size of an allocation group
    __be32 sb_agcount; // number of allocation groups
    __be32 sb_rbmblocks; // number of rt bitmap blocks
    __be32 sb_logblocks; // number of log blocks
    __be16 sb_versionnum; // header version == XFS_SB_VERSION
    __be16 sb_sectsize; // volume sector size, bytes
    __be16 sb_inodesize; // inode size, bytes
    __be16 sb_inopblock; // inodes per block
    char sb_fname[12]; // file system name
    __u8 sb_blocklog; // log2 of sb_blocksize
    __u8 sb_sectlog; // log2 of sb_sectsize
    __u8 sb_inodelog; // log2 of sb_inodesize
    __u8 sb_inopblog; // log2 of sb_inopblock
    __u8 sb_agblklog; // log2 of sb_agblocks (rounded up)
    __u8 sb_rextslog; // log2 of sb_rextents
    __u8 sb_inprogress; // mkfs is in progress, don't mount
    __u8 sb_imax_pct; // max % of fs for inode space
                    // statistics

    // these fields must remain coutiguous. if you really want to change their layout, make sure you fix
    // the code in xfs_trans_apply_sb_deltas().
    __be64 sb_icount; // allocated inodes
    __be64 sb_ifree; // free inodes
    __be64 sb_fdblocks; // free data blocks
    __be64 sb_frextents; // free realtime extents
    // end contiguous fields
    __be64 sb_uquotino; // user quota inode
    __be64 sb_gquotino; // group quota inode
    __be16 sb_qflags; // quota flags
    __u8 sb_flags; // misc. flags
    __u8 sb_shared_vn; // shared version number
    __be32 sb_inoalignmt; // inode chunk alignment, fsblocks
    __be32 sb_unit; // stripe or raid unit
    __be32 sb_width; // stripe or raid width
    __u8 sb_dirblklog; // log2 of dir block size (fsbs)
    __u8 sb_logsectlog; // log2 of the log sector size
    __be16 sb_logsectsize; // sector size for the log, bytes
    __be32 sb_logsunit; // stripe unit size for the log
    __be32 sb_features2; // additional feature bits

    // bad features2 field as a result of failing to pad the sb
    // structure to 64 bits. some machines will be using this field
    // for features2 bits. easiest just to mark it bad and not use 
    // it for anything else.
    __be32 sb_bad_features2;

    // version 5 superblock fields start here
    // feature masks
    __be32 sb_features_compat;
    __be32 sb_features_ro_compat;
    __be32 sb_features_incompat;
    __be32 sb_features_log_incompat;

    __le32 sb_crc; // superblock crc
    __be32 sb_spino_align; // sparse inode chunk alignment
    
    __be64 sb_pquotino; // project quota inode
    __be64 sb_lsn; // last write sequence
    my_uuid_t sb_meta_uuid; // metadata file system unique id

    // must be padded to 64 bit alignment
}xfs_dsb_t;

#endif //__SB_H__