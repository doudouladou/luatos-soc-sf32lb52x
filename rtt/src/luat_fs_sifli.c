#include "rtthread.h"
#include "drv_flash.h"
#include "ptab.h"
#include "luat_base.h"
#include "luat_fs.h"
#include "lfs.h"
#include "luat_timer.h"
static int fs_inited = 0;
extern const struct luat_vfs_filesystem vfs_fs_lfs2;
extern const struct luat_vfs_filesystem vfs_fs_luadb;
extern const struct luat_vfs_filesystem vfs_fs_ram;
extern size_t luat_luadb_act_size;

static lfs_t lfs;
int luat_lfs_init(size_t lfs_size);
void luat_luadb_read_bytes(char* dst, const char* ptr, size_t len) {
    size_t offset = (size_t)ptr;
    // LLOGD("dst %p offset %08X len %08X", dst, offset, len);
    rt_flash_read(LUAT_SCRIPT_START_ADDR + offset, (uint8_t *)dst, len);
}
int luat_fs_init(void)
{
    if (fs_inited)
        return 0;
    fs_inited = 1;
    luat_vfs_init(NULL);
    luat_vfs_reg(&vfs_fs_lfs2);
#ifdef __LUATOS__
	luat_vfs_reg(&vfs_fs_luadb);
    luat_vfs_reg(&vfs_fs_ram);
#endif

    luat_lfs_init(LUAT_FS_SIZE / 4096);
	luat_fs_conf_t conf = {
		.busname = (char*)&lfs,
		.type = "lfs2",
		.filesystem = "lfs2",
		.mount_point = "/"
	};
	luat_fs_mount(&conf);

    luat_luadb_act_size = LUAT_SCRIPT_SIZE;
    // LLOGD("luadb tmp addr %08X %08X", luadb_addr, luat_luadb_act_size);
	luat_fs_conf_t conf2 = {
		.busname = (char*)(0),
		// .busname = buff,
		.type = "luadb",
		.filesystem = "luadb",
		.mount_point = "/luadb/",
	};
	luat_fs_mount(&conf2);

#ifdef __LUATOS__
    luat_fs_conf_t conf3 = {
		.busname = NULL,
		.type = "ram",
		.filesystem = "ram",
		.mount_point = "/ram/"
	};
	luat_fs_mount(&conf3);
#endif
    return -1;
};
// lfs适配

/***************************************************
 ***************       MACRO      ******************
 ***************************************************/
#define LFS_BLOCK_DEVICE_READ_SIZE (256)
#define LFS_BLOCK_DEVICE_PROG_SIZE (256)
#define LFS_BLOCK_DEVICE_CACHE_SIZE (256)
#define LFS_BLOCK_DEVICE_ERASE_SIZE (4096) // one sector 4KB
// #define LFS_BLOCK_DEVICE_TOTOAL_SIZE (FLASH_FS_REGION_SIZE * 1024)
#define LFS_BLOCK_DEVICE_LOOK_AHEAD (16)
/***************************************************
 *******    FUNCTION FORWARD DECLARTION     ********
 ***************************************************/


static int block_device_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);

static int block_device_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);

static int block_device_erase(const struct lfs_config *cfg, lfs_block_t block);

static int block_device_sync(const struct lfs_config *cfg);

static int lfs_statfs_count(void *p, lfs_block_t b);

static char lfs_read_buf[256];
static char lfs_prog_buf[256];
static char  __attribute__((aligned(4))) lfs_lookahead_buf[LFS_BLOCK_DEVICE_LOOK_AHEAD];

// configuration of the filesystem is provided by this struct
struct lfs_config lfs_cfg =
{
    .context = NULL,
    // block device operations
    .read = block_device_read,
    .prog = block_device_prog,
    .erase = block_device_erase,
    .sync = block_device_sync,

    // block device configuration
    .read_size = LFS_BLOCK_DEVICE_READ_SIZE,
    .prog_size = LFS_BLOCK_DEVICE_PROG_SIZE,
    .block_size = LFS_BLOCK_DEVICE_ERASE_SIZE,
    //.block_count = LFS_BLOCK_DEVICE_TOTOAL_SIZE / LFS_BLOCK_DEVICE_ERASE_SIZE,
    .block_cycles = 200,
    .cache_size = LFS_BLOCK_DEVICE_CACHE_SIZE,
    .lookahead_size = LFS_BLOCK_DEVICE_LOOK_AHEAD,

    .read_buffer = lfs_read_buf,
    .prog_buffer = lfs_prog_buf,
    .lookahead_buffer = lfs_lookahead_buf,
    .name_max = 63,
    .file_max = 0,
    .attr_max = 0
};

static int block_device_read(const struct lfs_config *cfg, lfs_block_t block,lfs_off_t off, void *buffer, lfs_size_t size)
{
    int ret = -1;
    // LLOGD("block_device_read ,block = %d, off = %d,  size = %d", block, off, size);
    ret = rt_flash_read(LUAT_FS_START_ADDR + (block * LFS_BLOCK_DEVICE_ERASE_SIZE) + off, buffer, size);
    // LLOGD("block_device_read return val : %d",ret);
    if (ret <= 0)
    {
        return -1;
    }
    return 0;
}

static int block_device_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int ret = -1;
    // LLOGD("block_device_prog ,block = %d, off = %d,  size = %d",block, off, size);
    ret = rt_flash_write(LUAT_FS_START_ADDR + (block * LFS_BLOCK_DEVICE_ERASE_SIZE) + off, buffer, size);
    // LLOGD("block_device_prog return val : %d",ret);
    if (ret <= 0)
    {
        return -1;
    }

    return 0;
}

static int block_device_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    int ret = -1;
    ret = rt_flash_erase(LUAT_FS_START_ADDR + (block * LFS_BLOCK_DEVICE_ERASE_SIZE), LFS_BLOCK_DEVICE_ERASE_SIZE);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

static int block_device_sync(const struct lfs_config *cfg)
{
    return 0;
}

// // Initialize
int luat_lfs_init(size_t lfs_size)
{
    lfs_cfg.block_count = lfs_size;
    int err = lfs_mount(&lfs, &lfs_cfg);
    if (err)
    {
        luat_timer_mdelay(100);
        rt_kprintf("lfs mount fail ret=%d , exec format\r\n", err);
        err = lfs_format(&lfs, &lfs_cfg);
        //printf("lfs_format %d\r\n",err);
        if(err) {
            rt_kprintf("lfs format fail ret=%d\r\n", err);
            return err;
        }

        err = lfs_mount(&lfs, &lfs_cfg);
        if(err)
            return err;
    }
    return 0;
}
