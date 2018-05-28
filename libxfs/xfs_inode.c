#include "xfs_inode.h"

// min inode buf size
unsigned int m_inode_cluster_size = 8192;
// mask sb_inoalignmt if used
int m_inoalign_mask = 3;

int blkbb = 0;//1 << (sb->sb_blocklog - BBSHIFT);

int get_inode_offset_by_ino(const xfs_dsb_t *sb, const uint64_t ino, int64_t *a_offset)
{
    const char *func_name = "get_inode_offset_by_ino";
    if (NULL == a_offset) {
        Dprintf("%s : a_offset is null\n", func_name);
        return 1;
    }
    uint32_t cluster_agbno = 0;
    int numblks = 0;

    uint32_t agno = XFS_INO_TO_AGNO(sb, ino);
    uint32_t agino = XFS_INO_TO_AGINO(sb, ino);
    uint32_t agbno = XFS_AGINO_TO_AGBNO(sb, agino);
    int offset = XFS_AGINO_TO_OFFSET(sb, agino);
    if (agno >= sb->sb_agcount || agbno >= sb->sb_agblocks || offset >= sb->sb_inopblock || XFS_AGINO_TO_INO(sb, agno, agino) != ino) {
        Dprintf("%s : bad inode number %lld\n", func_name, ino);
        return 1;
    }
    if (m_inode_cluster_size > sb->sb_blocksize && m_inoalign_mask) {
        int blks_per_cluster = m_inode_cluster_size >> sb->sb_blocklog;
        uint32_t offset_agbno = agbno &m_inoalign_mask;
        uint32_t chunk_agbno = agbno - offset_agbno;
        cluster_agbno = chunk_agbno + ((offset_agbno / blks_per_cluster) * blks_per_cluster);
        offset += ((agbno - cluster_agbno) * sb->sb_inopblock);
        numblks = XFS_FSB_TO_BB(sb, blks_per_cluster);
    } else {
        cluster_agbno = agbno;
    }
    *a_offset = XFS_AGB_TO_DADDR(sb, agno, cluster_agbno);
    *a_offset = *a_offset << BBSHIFT;
    *a_offset += offset << sb->sb_inodelog;
    return 0;
}

int get_block_offset_by_blknum(const xfs_dsb_t *sb, const uint64_t blknum, int64_t *a_offset)
{
    const char *func_name = "get_block_offset_by_blknum";
    if (NULL == a_offset) {
        Dprintf("%s : a_offset is null\n", func_name);
        return 1;
    }
    uint32_t agno = XFS_FSB_TO_AGNO(sb, blknum);
    uint32_t agbno = XFS_FSB_TO_AGBNO(sb, blknum);
    if (agno >= sb->sb_agcount || agbno >= sb->sb_agblocks) {
        Dprintf("%s : bad fsblock %lld\n", func_name, blknum);
        return 1;
    }
    *a_offset = XFS_AGB_TO_DADDR(sb, agno, agbno);
    *a_offset = *a_offset << BBSHIFT;
    //Dprintf("blk offset:%lld\n", *a_offset);
    return 0;
}

int parse_buff_to_inode(xfs_inode_t *inode, const char *buf, const XFS_ENDIAN_ENUM end)
{
    const char *func_name = "parse_buff_to_inode";
    if (NULL == buf) {
        Dprintf("%s : buf is null\n", func_name);
        return 1;
    }
    if (XFS_UNKNOW_ENDIAN == end) {
        Dprintf("%s : unknow endian\n", func_name);
        return 1;
    }
    if (NULL == inode) {
        inode = calloc(1, sizeof(xfs_inode_t));
        //Dprintf("calloc inode:%d\n", inode);
    }
    uint64_t off = 0;
    inode->magic = xfs_getu16(end, &buf[off]);
    off += 2;
    if (XFS_INODE_MAGIC != inode->magic) {
        Dprintf("%s : magic(%x) not match\n", func_name, inode->magic);
        return 1;
    }
    inode->type = (xfs_getu16(end, &buf[off]) & 0xFE00);
    inode->mode = (xfs_getu16(end, &buf[off]) & 0x1FF);
    off += 2;
    inode->version = buf[off++];
    inode->format = buf[off++];
    inode->onlink = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->uid = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->gid = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->nlink = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->projid_lo = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->projid_hi = xfs_getu16(end, &buf[off]);
    off += 2;
    off += 6;
    inode->flushiter = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->atime.sec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->atime.nsec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->mtime.sec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->mtime.nsec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->ctime.sec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->ctime.nsec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->size = xfs_getu64(end, &buf[off]);
    off += 8;
    inode->nblocks = xfs_getu64(end, &buf[off]);
    off += 8;
    inode->extsize = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->nextents = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->anextents = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->forkoff = buf[off++];
    inode->aformat = buf[off++];
    inode->dmevmask = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->dmstate = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->flags = xfs_getu16(end, &buf[off]);
    off += 2;
    inode->gen = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->next_unlinked = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->crc = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->changecount = xfs_getu64(end, &buf[off]);
    off += 8;
    inode->lsn = xfs_getu64(end, &buf[off]);
    off += 8;
    inode->flags2 = xfs_getu64(end, &buf[off]);
    off += 8;
    inode->cowextsize = xfs_getu32(end, &buf[off]);
    off += 4;
    off += 12;
    inode->crtime.sec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->crtime.nsec = xfs_getu32(end, &buf[off]);
    off += 4;
    inode->ino = xfs_getu64(end, &buf[off]);
    off += 8;
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
    sprintf(inode->uuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", data1, data2, data3, data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
    //Dprintf("uuid:%s\n", inode->uuid);

    // local
    if (inode->format & XFS_DINODE_FMT_LOCAL && S_ISDIR(inode->type)) {
        inode->u3.count = buf[off++];
        inode->u3.i8count = buf[off++];
        inode->u3.i4 = xfs_getu32(end, &buf[off]);
        off += 4;
        if (inode->u3.i4 != inode->ino) {
            //Dprintf("i4(%ld) ino(%lld) %s some err\n", inode->u3.i4, inode->ino, func_name);
        }
        // for list space
        if (NULL == inode->list) {
            inode->list = calloc(inode->u3.count, sizeof(inline_list_t));
            //Dprintf("calloc %d inode->list:%d\n", inode->u3.count, inode->list);
        } else {
            Dprintf("%s list not null\n", func_name);
        }
        int i = 0;
        for (i = 0; i < inode->u3.count; ++i) {
            inode->list[i].namelen = buf[off++];
            inode->list[i].offset = xfs_getu16(end, &buf[off]);
            off += 2;
            inode->list[i].name = calloc((inode->list[i].namelen)+1, sizeof(char));
            if (NULL == inode->list[i].name) {
                Dprintf("%s name calloc is null\n", func_name);
                return 1;
            }
            strncpy(inode->list[i].name, buf+off, inode->list[i].namelen);
            off += inode->list[i].namelen;
            inode->list[i].filetype = buf[off++];
            inode->list[i].inumber = xfs_getu32(end, &buf[off]);
            off += 4;
        }
    } else if ((inode->format & XFS_DINODE_FMT_EXTENTS) && (inode->nblocks > 0)) { // extents
        // for find how many
        uint64_t tmpoff = off;
        uint64_t check = *(uint64_t *)(buf+tmpoff+8);
        uint32_t blockcount = 0;
        int i = 0, tmpi = 0;
        while (0 != check && blockcount < inode->nblocks) {
            tmpoff += 13;
            blockcount += (xfs_getu24(end, &buf[tmpoff]) & 0x1FFFFF);
            tmpoff += 3;
            i++;
            check = *(uint64_t *)(buf+tmpoff+8);
        }
        //Dprintf("i:%d blockcount:%d\n", i, blockcount);

        if (i > 0) {
            inode->exts = calloc(i, sizeof(ext_info_t));
            if (NULL == inode->exts) {
                Dprintf(" %d exts is null\n", i);
                return 1;
            }
            //check = *(uint64_t *)(buf+off+8);
            while (0 != i--) {
                ext_info_t *ext = &(inode->exts[tmpi]);
                if (NULL == ext) {
                    Dprintf("%d ext is null.\n", tmpi);
                    break;
                }
                ext->extentflag = ((uint8_t)buf[off] & 0x80) >> 7;
                ext->startoff = (xfs_getu64(end, &buf[off]) & 0x7FFFFFFFFFFFFE00) >> 9;
                off += 6;
                ext->startblock = (xfs_getu64(end, &buf[off]) & 0x7FFFFFFFFFFFFE0) >> 5;
                off += 7;
                ext->blockcount = (xfs_getu24(end, &buf[off]) & 0x1FFFFF);
                off += 3;
                //Dprintf("type:%d, flag:%d, startoff:%lld, startblock:%lld, blockcount:%ld\n", inode->type, ext->extentflag, ext->startoff, ext->startblock, ext->blockcount);
                if (S_ISDIR(inode->type)) {
                    //Dprintf("extent dir.\n");
                } else if (S_ISREG(inode->type)) {
                    //Dprintf("extent reg file.\n");
                } else if (S_ISLNK(inode->type)) {
                    Dprintf("extent lnk file.\n");
                } else {
                    Dprintf("extent others%d.\n", inode->type);
                }
                //check = *(uint64_t *)(buf+off+8);
                tmpi++;
            }
            inode->extcount = tmpi;
            //Dprintf("extcount:%d\n", inode->extcount);
        } else {
            Dprintf("i(%d) <= 0\n", i);
        }
    } else if (S_ISLNK(inode->type) && inode->size > 0) {
        inode->lnk_path = calloc(1, inode->size+1);
        if (NULL == inode->lnk_path) {
            Dprintf("calloc lnk path err\n");
            return 1;
        }
        strncpy(inode->lnk_path, buf+off, inode->size);
    } else if (0 == inode->size) {
        ;// size == 0
    } else {
        Dprintf("store others %lld format:%d, type:%d size:%d\n", inode->ino, inode->format, inode->type, inode->size);
    }
 
    return 0;
}

int parse_buff_to_dir_ext(xfs_dir_ext_t *dir_ext, const char *buf, const uint64_t buf_size, const XFS_ENDIAN_ENUM end)
{
    const char *func_name = "parse_buff_to_dir_ext";
    if (NULL == buf) {
        Dprintf("%s : buf is null\n", func_name);
        return 1;
    }
    if (XFS_UNKNOW_ENDIAN == end) {
        Dprintf("%s : unknow endian\n", func_name);
        return 1;
    }
    if (NULL == dir_ext) {
        //dir_ext = calloc(1, sizeof(xfs_dir_ext_t));
        Dprintf("calloc dir_ext:%d\n", dir_ext);
    }
    uint64_t off = 0;
    //dir_ext = buf;
    dir_ext->hdr.hdr.magic = xfs_getu32(end, &buf[off]);
    off += 4;
    if ((XFS_DIR3_BLOCK_MAGIC != dir_ext->hdr.hdr.magic) && (XFS_DIR3_DATA_MAGIC != dir_ext->hdr.hdr.magic)) {
        //Dprintf("magic(%ld) is err, shuld be %ld %ld\n", dir_ext->hdr.hdr.magic, XFS_DIR3_BLOCK_MAGIC, XFS_DIR3_DATA_MAGIC);
        return 1;
    }
    dir_ext->hdr.hdr.crc = xfs_getu32(end, &buf[off]);
    off += 4;
    dir_ext->hdr.hdr.blkno = xfs_getu64(end, &buf[off]);
    off += 8;
    dir_ext->hdr.hdr.lsn = xfs_getu64(end, &buf[off]);
    off += 8;
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
    sprintf(dir_ext->hdr.hdr.uuid, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", data1, data2, data3, data4[0], data4[1], data4[2], data4[3], data4[4], data4[5], data4[6], data4[7]);
    //Dprintf("uuid:%s\n", dir_ext->hdr.hdr.uuid);
    dir_ext->hdr.hdr.owner = xfs_getu64(end, &buf[off]);
    off += 8;
    dir_ext->hdr.best_free[0].offset = xfs_getu16(end, &buf[off]);
    off += 2;
    dir_ext->hdr.best_free[0].length = xfs_getu16(end, &buf[off]);
    off += 2;
    dir_ext->hdr.best_free[1].offset = xfs_getu16(end, &buf[off]);
    off += 2;
    dir_ext->hdr.best_free[1].length = xfs_getu16(end, &buf[off]);
    off += 2;
    dir_ext->hdr.best_free[2].offset = xfs_getu16(end, &buf[off]);
    off += 2;
    dir_ext->hdr.best_free[2].length = xfs_getu16(end, &buf[off]);
    off += 2;
    off += 4;
    uint64_t tmpoff = off;
    uint16_t check0 = buf[off];
    uint64_t check1 = xfs_getu64(end, &buf[off]);
    int i = 0, tmpi = 0;
    while (0xFFFF != check0 && 0 != check1 && (tmpoff < buf_size)) {
        tmpoff += 8;
        unsigned char len = buf[tmpoff++];
        tmpoff += len;
        tmpoff++;
        tmpoff += ((19+len)&0xFFF8) - (12+len);
        tmpoff += 2;
        i++;
        check0 = buf[tmpoff];
        check1 = xfs_getu64(end, &buf[tmpoff]);
    }
    if (i > 0) {
        dir_ext->entrys = calloc(i, sizeof(xfs_dir3_entry_t));
        while (i != tmpi) {
            xfs_dir3_entry_t *entry = &(dir_ext->entrys[tmpi]);
            if (NULL == entry) {
                Dprintf("%s : calloc entry err\n", func_name);
                break;
            }
            entry->inumber = xfs_getu64(end, &buf[off]);
            off += 8;
            entry->namelen = buf[off++];
            entry->name = calloc(1, entry->namelen+1);
            if (NULL == entry->name) {
                Dprintf("%s : calloc entry name err\n", func_name);
                return 1;
            }
            strncpy(entry->name, buf+off, entry->namelen);
            off += entry->namelen;
            entry->filetype = buf[off++];
            off += ((19+entry->namelen)&0xFFF8) - (12+entry->namelen);
            entry->tag = xfs_getu16(end, &buf[off]);
            off += 2;
            tmpi++;
        }
    }
    if (i != tmpi) {
        Dprintf("i(%d) not tmpi(%d)\n", i, tmpi);
        //return 1;
    }
    dir_ext->subcount = tmpi;

    return 0;
}

void free_inode(xfs_inode_t *inode)
{
    // free space
    if (NULL != inode) {
        if (NULL != (inode->exts)) {
            free(inode->exts);
        }
        if (NULL != (inode->lnk_path)) {
            free(inode->lnk_path);
        }
        if (NULL != inode->list) {
            int i = 0;
            while (i != inode->u3.count) {
                if (NULL != inode->list[i].name) {
                    free(inode->list[i].name);
                }
                ++i;
            }
            free(inode->list);
        }
        free(inode);
        inode = NULL;
    }
}

void free_dir_ext(xfs_dir_ext_t *dir_ext)
{
    if (NULL != dir_ext) {
        if (NULL != dir_ext->entrys) {
            for (int i = 0; i < dir_ext->subcount; ++i) {
                xfs_dir3_entry_t *entry = &(dir_ext->entrys[i]);
                if (NULL != entry->name) {
                    //Dprintf("%d free name:%d\n", i, entry->name);
                    free(entry->name);
                }
            }
            //Dprintf("%d free entrys:%d\n", dir_ext->entrys);
            free(dir_ext->entrys);
        }
        //Dprintf("free dir_ext:%d\n", dir_ext);
        free(dir_ext);
        dir_ext = NULL;
    }
}