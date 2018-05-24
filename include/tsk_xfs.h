#ifndef __TSK_XFS_H__
#define __TSK_XFS_H__

#include <tsk/libtsk.h>
#include <tsk/fs/tsk_fs_i.h>
#include "xfs_sb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XFS_INFO {
    TSK_FS_INFO fs_info;
    tsk_lock_t cache_lock;
    xfs_dsb_t sb;
}XFS_INFO;

extern TSK_FS_INFO * xfs_open(TSK_IMG_INFO *a_img_info, TSK_OFF_T a_offset, TSK_FS_TYPE_ENUM a_ftype, uint8_t test);
extern uint8_t xfs_file_add_meta(TSK_FS_INFO *a_fs, TSK_FS_FILE *a_file, TSK_INUM_T a_inum);
extern TSK_RETVAL_ENUM xfs_dir_open_meta(TSK_FS_INFO *a_fs, TSK_FS_DIR **a_dir, TSK_INUM_T a_inum);
extern int xfs_name_cmp(TSK_FS_INFO * a_fs, const char *s1, const char *s2);
extern uint8_t xfs_make_data_run(TSK_FS_FILE *a_file);
extern void xfs_close(TSK_FS_INFO *a_fs);

#ifdef __cplusplus
}
#endif

#endif // __TSK_XFS_H__