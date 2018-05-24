#include "tsk_xfs.h"
#include "xfs_inode.h"

static uint8_t xfs_fscheck(TSK_FS_INFO *fs, FILE *hFile)
{
    if (NULL == fs) {
        return 1;
    }
    uint32_t val = 0;
    ssize_t cnt = tsk_fs_read(fs, (TSK_OFF_T)0, (char *)&val, sizeof(val));
    if (sizeof(val) != cnt) {
        Dprintf("read val size error\n");
        return 1;
    }
    TSK_ENDIAN_ENUM flag;
    if (1 == tsk_guess_end_u32(&flag, (uint8_t *)&val, XFS_SB_MAGIC)) {
        Dprintf("var %ld magic:%ld\n", val, XFS_SB_MAGIC);
        return 1;
    }
    fs->endian = flag;
    return 0;
}

// error:1    success:0
static int setSuperBlock(XFS_INFO *xfs)
{
    const char *func_name = "setSuperBlock";
    if (NULL == xfs) {
        Dprintf("%s :xfs is null\n", func_name);
        return 1;
    }
    TSK_FS_INFO *fs = &(xfs->fs_info);
    if (NULL == fs) {
        Dprintf("%s :xfs->fs_info is null\n", func_name);
        return 1;
    }
    TSK_ENDIAN_ENUM end = fs->endian;
    if (TSK_UNKNOWN_ENDIAN == end) {
        Dprintf("%s :unknown endian\n", func_name);
        return 1;
    }
    char buf[512] = {0};
    tsk_fs_read(fs, 0, buf, 512);
    xfs_dsb_t *sb = &(xfs->sb);
    int ret = parse_buff_to_superblock(sb, buf, (XFS_ENDIAN_ENUM)end);
    if (0 != ret) {
        Dprintf("%s :parse buff to superblock err\n", func_name);
        return 1;
    }

    return 0;
}


TSK_FS_INFO * xfs_open(TSK_IMG_INFO *a_img_info, TSK_OFF_T a_offset, TSK_FS_TYPE_ENUM a_ftype, uint8_t test)
{
    const char *func_name = "xfs_open";
    XFS_INFO *xfs = NULL;
    TSK_FS_INFO *fs = NULL;
    tsk_error_reset();
    if (0 == TSK_FS_TYPE_ISXFS(a_ftype)) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s: Invalid FS type", func_name);
        return NULL;
    }
    if (0 == a_img_info->sector_size) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s: sector size is 0.", func_name);
    }
    if (NULL == (xfs = (XFS_INFO *)tsk_fs_malloc(sizeof(XFS_INFO)))) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s: tsk_fs_malloc err.", func_name);
        return NULL;
    }
    fs = &(xfs->fs_info);
    fs->tag = TSK_FS_INFO_TAG;
    fs->endian = TSK_UNKNOWN_ENDIAN;
    //fs->ftype = a_ftype;
    fs->img_info = a_img_info;
    fs->offset = a_offset;
    if (1 == xfs_fscheck(fs, NULL)) {
        Dprintf("check error\n");
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s: xfs_fscheck err.", func_name);
        return NULL;
    }
    fs->dev_bsize = a_img_info->sector_size;
    if (1 == setSuperBlock(xfs)) {
        Dprintf("setsuperblock error\n");
        return NULL;
    }
    fs->block_size = xfs->sb.sb_blocksize;
    fs->block_count = xfs->sb.sb_dblocks;
    fs->ftype = TSK_FS_TYPE_XFS;
    fs->root_inum = xfs->sb.sb_rootino;
    fs->file_add_meta = xfs_file_add_meta;
    fs->dir_open_meta = xfs_dir_open_meta;
    fs->close = xfs_close;
    fs->name_cmp = xfs_name_cmp;
    fs->load_attrs = xfs_make_data_run;
    fs->get_default_attr_type = tsk_fs_unix_get_default_attr_type;

    return fs;
}

uint8_t xfs_file_add_meta(TSK_FS_INFO *a_fs, TSK_FS_FILE *a_file, TSK_INUM_T a_inum)
{
    const char *func_name = "xfs_file_add_meta";
    //Dprintf("%s comming\n", func_name);
    XFS_INFO *xfs = (XFS_INFO *)a_fs;
    uint8_t allocedMeta = 0;
    tsk_error_reset();
    if (NULL == a_file) {
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s: fs_file is NULL", func_name);
        return 1;
    }
    if (NULL == a_file->meta) {
        a_file->meta = tsk_fs_meta_alloc(0);
        if (NULL == a_file->meta) {
            return 1;
        }
        allocedMeta = 1;
    } else {
        tsk_fs_meta_reset(a_file->meta);
    }

    int64_t off = 0;
    int ret = get_inode_offset_by_ino(&(xfs->sb), a_inum, &off);
    if (0 != ret) {
        Dprintf("%s : get inode(%d) offset err \n", func_name, a_inum);
        return TSK_ERR;
    }
    char buf[512] = {0};
    ssize_t cnt = tsk_fs_read(a_fs, off, buf, 512);
    if (512 != cnt) {
        Dprintf("%s : fs read err\n", func_name);
        return TSK_ERR;
    }
    xfs_inode_t *xfs_inode = calloc(1, sizeof(xfs_inode_t));
    ret = parse_buff_to_inode(xfs_inode, buf, xfs->fs_info.endian);
    if (0 == ret) {
        if (a_inum != xfs_inode->ino) {
            Dprintf("%s : a_inum(%lld) != ino(%lld)\n", func_name, a_inum, xfs_inode->ino);
        }
        a_file->meta->addr = a_inum;
        a_file->meta->atime = xfs_inode->atime.sec;
        a_file->meta->atime_nano = xfs_inode->atime.nsec;
        a_file->meta->mtime = xfs_inode->mtime.sec;
        a_file->meta->mtime_nano = xfs_inode->mtime.nsec;
        a_file->meta->ctime = xfs_inode->ctime.sec;
        a_file->meta->ctime_nano = xfs_inode->ctime.nsec;
        a_file->meta->attr_state = TSK_FS_META_ATTR_EMPTY;
        if (a_file->meta->attr) {
            tsk_fs_attrlist_markunused(a_file->meta->attr);
        }
        a_file->meta->attr = NULL;
        //*(uint64_t *)(a_file->meta->content_ptr) = (xfs_inode->startblock);
        if (NULL != xfs_inode->exts) {
            a_file->meta->content_len = 1*sizeof(ext_info_t);
            a_file->meta->content_ptr = calloc(1, sizeof(ext_info_t));
            memcpy(a_file->meta->content_ptr, (xfs_inode->exts), 1*sizeof(ext_info_t));
        } else {
            a_file->meta->content_len = 0;
            a_file->meta->content_ptr = NULL;
        }
        a_file->meta->content_type = TSK_FS_META_CONTENT_TYPE_DEFAULT;
        a_file->meta->uid = xfs_inode->uid;
        a_file->meta->gid = xfs_inode->gid;
        a_file->meta->size = xfs_inode->size;
        a_file->meta->nlink = xfs_inode->nlink;
        a_file->meta->flags = TSK_FS_META_FLAG_USED | TSK_FS_META_FLAG_ALLOC;
        if (S_ISDIR(xfs_inode->type)) {
            a_file->meta->type = TSK_FS_META_TYPE_DIR;
        } else if (S_ISREG(xfs_inode->type)) {
            a_file->meta->type = TSK_FS_META_TYPE_REG;
        } else {
            Dprintf("%s : type others\n", func_name);
        }
        //Dprintf("%s %d: file ok\n", func_name, a_file->name);
    }
    free_inode(xfs_inode);
    return 0;
}

TSK_RETVAL_ENUM xfs_dir_open_meta(TSK_FS_INFO *a_fs, TSK_FS_DIR **a_dir, TSK_INUM_T a_inum)
{
    const char *func_name = "xfs_dir_open_meta";
    //Dprintf("%s comming\n", func_name);
    XFS_INFO *xfs = (XFS_INFO *)a_fs;
    if (NULL == a_dir) {
        tsk_error_reset();
        tsk_error_set_errno(TSK_ERR_FS_ARG);
        tsk_error_set_errstr("%s : a_dir is null", func_name);
        return TSK_ERR;
    }
    TSK_FS_DIR *dir = *a_dir;
    if (dir) {
        tsk_fs_dir_reset(dir);
        dir->addr = a_inum;
    } else {
        if (NULL == (*a_dir = dir = tsk_fs_dir_alloc(a_fs, a_inum, 128))) {
            tsk_error_reset();
            tsk_error_set_errno(TSK_ERR_FS_ARG);
            tsk_error_set_errstr("%s : dir alloc err", func_name);
            return TSK_ERR;
        }
    }

    // handle the orphan directory if it't contents were requested
    if (a_inum == TSK_FS_ORPHANDIR_INUM(a_fs)) {
        return tsk_fs_dir_find_orphans(a_fs, dir);
    }

    // get the inode and verify it has attributes
    if (NULL == (dir->fs_file = tsk_fs_file_open_meta(a_fs, NULL, a_inum))) {
        tsk_error_errstr2_concat(" - xfs_dir_open_meta");
        return TSK_COR;
    }

    int64_t off = 0;
    int ret = get_inode_offset_by_ino(&(xfs->sb), a_inum, &off);
    if (0 != ret) {
        Dprintf("%s : get inode(%d) offset err \n", func_name, a_inum);
        return TSK_ERR;
    }
    char buf[512] = {0};
    ssize_t cnt = tsk_fs_read(a_fs, off, buf, 512);
    if (512 != cnt) {
        Dprintf("%s : fs read err\n", func_name);
        return TSK_ERR;
    }
    xfs_inode_t *xfs_dir = calloc(1, sizeof(xfs_inode_t));
    if (NULL == xfs_dir) {
        Dprintf("%s : calloc xfs_dir null\n", func_name);
        return TSK_ERR;
    }
    ret = parse_buff_to_inode(xfs_dir, buf, xfs->fs_info.endian);
    if (0 == ret) {
        if (XFS_DINODE_FMT_EXTENTS & xfs_dir->format && xfs_dir->nblocks > 0) {
            ret = get_block_offset_by_blknum(&(xfs->sb), xfs_dir->exts->startblock, &off);
            if (0 != ret) {
                //Dprintf("%s : get block(%lld) offset err \n", func_name, xfs_dir->startblock);
                free_inode(xfs_dir);
                return TSK_ERR;
            }
            //Dprintf("%lld block off : %lld, size:%d\n", xfs_dir->startblock, off, xfs_dir->size);
            char *dir_buf = calloc(1, xfs_dir->size);
            if (NULL == dir_buf) {
                Dprintf("%s : calloc dir_buf err\n", func_name);
                free_inode(xfs_dir);
                return TSK_ERR;
            }
            cnt = tsk_fs_read(a_fs, off, dir_buf, xfs_dir->size);
            if (cnt != xfs_dir->size) {
                Dprintf("%s : fs read dir_buf err\n", func_name);
                free(dir_buf);
                free_inode(xfs_dir);
                return TSK_ERR;
            }
            xfs_dir_ext_t *xfs_dir_ext = calloc(1, sizeof(xfs_dir_ext_t));
            ret = parse_buff_to_dir_ext(xfs_dir_ext, dir_buf, xfs->fs_info.endian);
            TSK_FS_NAME *name = tsk_fs_name_alloc(256, 0);
            if (NULL != name) {
                for (int i = 0; i < xfs_dir_ext->subcount; ++i) {
                    xfs_dir3_entry_t *entry = &(xfs_dir_ext->entrys[i]);
                    name->meta_addr = entry->inumber;
                    name->flags = TSK_FS_NAME_FLAG_ALLOC;
                    memset(name->name, 0, name->name_size);
                    strncpy(name->name, entry->name, name->name_size-1);
                    switch (entry->filetype) {
                    case DIR_SUB_FILE:
                        name->type = (TSK_FS_NAME_TYPE_REG);
                        break;
                    case DIR_SUB_DIR:
                        name->type = (TSK_FS_NAME_TYPE_DIR);
                        break;
                    default:
                        Dprintf("%s : switch others%d\n", func_name, entry->filetype);
                    }
                    tsk_fs_dir_add(dir, name);
                }
                tsk_fs_name_free(name);
            }
            //for (int j = 0; j < xfs_dir_ext->subcount; ++j) {
                //xfs_dir3_entry_t *entry = &(xfs_dir_ext->entrys[j]);
                //Dprintf("%d %d %d: tag:%x name:%s\n", entry, j, entry->inumber, entry->tag, entry->name);
                //Dprintf("%d i:%2d inumber:%7lld tag:%4x len:%2d pad:%d name:%s\n", entry, j, entry->inumber, entry->tag, entry->namelen, ((19+entry->namelen)&0xFFF8) - (12+entry->namelen), entry->name);
            //}
            free(dir_buf);
            free_dir_ext(xfs_dir_ext);
        } else if (XFS_DINODE_FMT_LOCAL & xfs_dir->format) {
            TSK_FS_NAME *name = tsk_fs_name_alloc(256, 0);
            if (NULL != name) {
                for (int i = 0; i < xfs_dir->u3.count; ++i) {
                    name->meta_addr = xfs_dir->list[i].inumber;
                    name->flags = TSK_FS_NAME_FLAG_ALLOC;
                    memset(name->name, 0, name->name_size);
                    strncpy(name->name, xfs_dir->list[i].name, name->name_size-1);
                    switch (xfs_dir->list[i].filetype) {
                    case DIR_SUB_FILE:
                        name->type = (TSK_FS_NAME_TYPE_REG);
                        break;
                    case DIR_SUB_DIR:
                        name->type = (TSK_FS_NAME_TYPE_DIR);
                        break;
                    default:
                        Dprintf("%s : switch others%d\n", func_name, xfs_dir->list[i].filetype);
                    }
                    tsk_fs_dir_add(dir, name);
                }
                tsk_fs_name_free(name);
            }
        } else {
            Dprintf("no local no extents:%d %d\n", xfs_dir->format, xfs_dir->type);
        }
    }
    free_inode(xfs_dir);

    return TSK_OK;
}

int xfs_name_cmp(TSK_FS_INFO *a_fs, const char *s1, const char *s2)
{
    const char *func_name = "xfs_name_cmp";
    //Dprintf("%s comming\n", func_name);
    return strcmp(s1, s2);
}

uint8_t xfs_make_data_run(TSK_FS_FILE *a_file)
{
    const char *func_name = "xfs_make_data_run";
    //Dprintf("%s comming\n", func_name);
    TSK_FS_ATTR *attr;
    TSK_FS_META *meta = a_file->meta;
    XFS_INFO *xfs = (XFS_INFO *)a_file->fs_info;
    tsk_error_reset();
    if ((NULL != meta->attr) && (TSK_FS_META_ATTR_STUDIED == meta->attr_state)) {
        return 0;
    } else if (TSK_FS_META_ATTR_ERROR == meta->attr_state) {
        return 1;
    } else if (NULL != meta->attr) {
        tsk_fs_attrlist_markunused(meta->attr);
    } else if (NULL == meta->attr) {
        meta->attr = tsk_fs_attrlist_alloc();
    } else {
        Dprintf("%s : others\n", func_name);
    }
    if (0 == TSK_FS_TYPE_ISXFS(xfs->fs_info.ftype)) {
        tsk_error_set_errno(TSK_ERR_FS_INODE_COR);
        tsk_error_set_errstr("%s : not a XFS file system. %x", xfs->fs_info.ftype);
        return 1;
    }
    if (NULL == (attr = tsk_fs_attrlist_getnew(meta->attr, TSK_FS_ATTR_NONRES))) {
        return 1;
    }
    TSK_OFF_T length = roundup(meta->size, xfs->fs_info.block_size);
	TSK_FS_ATTR_FLAG_ENUM flag = TSK_FS_ATTR_FLAG_NONE;
    if (tsk_fs_attr_set_run(a_file, attr, NULL, NULL,
                                TSK_FS_ATTR_TYPE_DEFAULT, TSK_FS_ATTR_ID_DEFAULT,
                                meta->size, meta->size, length, flag, 0))
    {
            return 1;
    }
    ext_info_t *exts = meta->content_ptr;
    //uint64_t startblock = *((uint64_t *)meta->content_ptr);
    /*int64_t off = 0;
    int ret = get_block_offset_by_blknum(&(xfs->sb), startblock, &off);
    if (0 != ret) {
        Dprintf("%s : get block(%lld) offset err \n", func_name, startblock);
        return 1;
    }
    */
    //Dprintf("startblock:%d, len:%lld\n", exts->startblock, exts->blockcount);
    TSK_FS_ATTR_RUN *data_run = tsk_fs_attr_run_alloc();
    if (data_run == NULL) { return 1; }

    data_run->offset = exts->startoff;
    data_run->addr = exts->startblock;
    data_run->len = exts->blockcount;
    // save the run
    if (tsk_fs_attr_add_run(a_file->fs_info, attr, data_run)) {
        return 1;
    }
    meta->attr_state = TSK_FS_META_ATTR_STUDIED;

    return 0;
}

void xfs_close(TSK_FS_INFO *a_fs)
{
    const char *func_name = "xfs_close";
    Dprintf("%s comming\n", func_name);
    XFS_INFO *xfs = (XFS_INFO *)a_fs;
    tsk_release_lock(&xfs->cache_lock);
    tsk_deinit_lock(&xfs->cache_lock);
    free(xfs);
}