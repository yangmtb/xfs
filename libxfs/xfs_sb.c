#include "xfs_sb.h"

int parse_buff_to_superblock(xfs_dsb_t *sb, const char buf[512], const XFS_ENDIAN_ENUM end)
{
    const char *func_name = "parse_buff_to_superblock";
    if (NULL == sb) {
        Dprintf("%s : sb is NULL\n", func_name);
        return 1;
    }
    if (NULL == buf) {
        Dprintf("%s : buf is NULL\n", func_name);
        return 1;
    }
    if (XFS_UNKNOW_ENDIAN == end) {
        Dprintf("%s : unknow endian\n", func_name);
        return 1;
    }
    uint64_t off = 0;
    // magicnum
    sb->sb_magicnum = xfs_getu32(end, (__be32 *)&(buf[off]));
    off += sizeof(sb->sb_magicnum);
    //Dprintf("magic:%ld\n", (sb->sb_magicnum));
    if (XFS_SB_MAGIC != sb->sb_magicnum) {
        Dprintf("%s :magic(%x) not match\n", func_name, sb->sb_magicnum);
        return 1;
    }
    // block size
    sb->sb_blocksize = xfs_getu32(end, (__be32 *)&(buf[off]));
    off += sizeof(sb->sb_blocksize);
    // data blocks
    sb->sb_dblocks = xfs_getu64(end, &buf[off]);
    off += sizeof(sb->sb_dblocks);
    // realtime blocks
    sb->sb_rblocks = xfs_getu64(end, &buf[off]);
    off += sizeof(sb->sb_rblocks);
    // realtime extents
    sb->sb_rextents = xfs_getu64(end, &buf[off]);
    off += sizeof(sb->sb_rextents);
    // file system uuid
    uint32_t data1 = xfs_getu32(end, (uint32_t *)&buf[off]);
    off += sizeof(data1);
    uint16_t data2 = xfs_getu16(end, (uint16_t *)&buf[off]);
    off += sizeof(data2);
    uint16_t data3 = xfs_getu16(end, (uint16_t *)&buf[off]);
    off += sizeof(data3);
    uint8_t data4[8] = {0};
    data4[0] = (uint8_t)buf[off++];
    data4[1] = (uint8_t)buf[off++];
    data4[2] = (uint8_t)buf[off++];
    data4[3] = (uint8_t)buf[off++];
    data4[4] = (uint8_t)buf[off++];
    data4[5] = (uint8_t)buf[off++];
    data4[6] = (uint8_t)buf[off++];
    data4[7] = (uint8_t)buf[off++];
    sprintf(sb->sb_uuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", data1, data2, data3, data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
    //Dprintf("uuid:%s\n", sb->sb_uuid);
    // log start block
    sb->sb_logstart = xfs_getu64(end, &buf[off]);
    off += 8;
    // root inode number
    sb->sb_rootino = xfs_getu64(end, &buf[off]);
    off += 8;
    // bitmap inode for realtime extents
    sb->sb_rbmino = xfs_getu64(end, &buf[off]);
    off += 8;
    // summary inode for rt bitmap
    sb->sb_rsumino = xfs_getu64(end, &buf[off]);
    off += 8;
    // realtime extent size, blocks
    sb->sb_rextsize = xfs_getu32(end, &buf[off]);
    off += 4;
    // size of an allocation group
    sb->sb_agblocks = xfs_getu32(end, &buf[off]);
    off += 4;
    // number of allocation groups
    sb->sb_agcount = xfs_getu32(end, &buf[off]);
    off += 4;
    // number of rt bitmap blocks
    sb->sb_rbmblocks = xfs_getu32(end, &buf[off]);
    off += 4;
    // number of log blocks
    sb->sb_logblocks = xfs_getu32(end, &buf[off]);
    off += 4;
    // header version
    sb->sb_versionnum = xfs_getu16(end, &buf[off]);
    off += 2;
    // volume sector size
    sb->sb_sectsize = xfs_getu16(end, &buf[off]);
    off += 2;
    // inode size
    sb->sb_inodesize = xfs_getu16(end, &buf[off]);
    off += 2;
    // inodes per block
    sb->sb_inopblock = xfs_getu16(end, &buf[off]);
    off += 2;
    // file system name
    //sb->sb_fname = buf[off];
    //memcpy(sb->sb_fname, buf[off], 12);
    strncpy(sb->sb_fname, buf+off, 12);
    off += 12;
    // log2 of blocksize
    sb->sb_blocklog = buf[off++];
    // log2 of sectsize
    sb->sb_sectlog = buf[off++];
    // log2 of indoesize
    sb->sb_inodelog = buf[off++];
    // log2 of inopblock
    sb->sb_inopblog = buf[off++];
    // log2 of agblocks
    sb->sb_agblklog = buf[off++];
    // log2 of rextents
    sb->sb_rextslog = buf[off++];
    // progress
    sb->sb_inprogress = buf[off++];
    // max %
    sb->sb_imax_pct = buf[off++];
    // allocated inode
    sb->sb_icount = xfs_getu64(end, &buf[off]);
    off += 8;
    // free inodes
    sb->sb_ifree = xfs_getu64(end, &buf[off]);
    off += 8;
    // free data block
    sb->sb_fdblocks = xfs_getu64(end, &buf[off]);
    off += 8;
    // free realtime extents
    sb->sb_frextents = xfs_getu64(end, &buf[off]);
    off += 8;
    // user quota inode
    sb->sb_uquotino = xfs_getu64(end, &buf[off]);
    off += 8;
    // group quota inode
    sb->sb_gquotino = xfs_getu64(end, &buf[off]);
    off += 8;
    // quota flags
    sb->sb_qflags = xfs_getu16(end, &buf[off]);
    off += 2;
    // flags
    sb->sb_flags = buf[off++];
    // shared version number
    sb->sb_shared_vn = buf[off++];
    // inode chunk alignment
    sb->sb_inoalignmt = xfs_getu32(end, &buf[off]);
    off += 4;
    // stripe or raid unit
    sb->sb_unit = xfs_getu32(end, &buf[off]);
    off += 4;
    // stripe or raid width
    sb->sb_width = xfs_getu32(end, &buf[off]);
    off += 4;
    // log2 of dir block size
    sb->sb_dirblklog = buf[off++];
    // log2 of the log sector size
    sb->sb_logsectlog = buf[off++];
    // sector size for the log
    sb->sb_logsectsize = xfs_getu16(end, &buf[off]);
    off += 2;
    // stripe unit size for the log
    sb->sb_logsunit = xfs_getu32(end, &buf[off]);
    off += 4;
    // additional feature bits
    sb->sb_features2 = xfs_getu32(end, &buf[off]);
    off += 4;
    // bad features2
    off += 4;
    // feature masks
    sb->sb_features_compat = xfs_getu32(end, &buf[off]);
    off += 4;
    sb->sb_features_ro_compat = xfs_getu32(end, &buf[off]);
    off += 4;
    sb->sb_features_incompat = xfs_getu32(end, &buf[off]);
    off += 4;
    sb->sb_features_log_incompat = xfs_getu32(end, &buf[off]);
    off += 4;
     // superblock crc
    sb->sb_crc = xfs_getu32(end, &buf[off]);
    off += 4;
    // sparse inode chunk alignment
    sb->sb_spino_align = xfs_getu32(end, &buf[off]);
    off += 4;
    // project quota inode
    sb->sb_pquotino = xfs_getu64(end, &buf[off]);
    off += 8;
    // last write sequence
    sb->sb_lsn = xfs_getu64(end, &buf[off]);
    off += 8;
    // meta uuid
    data1 = xfs_getu32(end, (uint32_t *)&buf[off]);
    off += sizeof(data1);
    data2 = xfs_getu16(end, (uint16_t *)&buf[off]);
    off += sizeof(data2);
    data3 = xfs_getu16(end, (uint16_t *)&buf[off]);
    off += sizeof(data3);
    data4[0] = (uint8_t)buf[off++];
    data4[1] = (uint8_t)buf[off++];
    data4[2] = (uint8_t)buf[off++];
    data4[3] = (uint8_t)buf[off++];
    data4[4] = (uint8_t)buf[off++];
    data4[5] = (uint8_t)buf[off++];
    data4[6] = (uint8_t)buf[off++];
    data4[7] = (uint8_t)buf[off++];
    sprintf(sb->sb_meta_uuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", data1, data2, data3, data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
    //Dprintf("off : %d \n", off);
 
    return 0;
}