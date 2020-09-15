//
// Created by caesar kekxv on 2020/9/15.
//

#include <cstdio>
#include "lfs.h"
#include "lfs_util.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

uint8_t _file_Buff[2][1024];

int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block,
                                    lfs_off_t off, void *buffer, lfs_size_t size) {
    memcpy(buffer, &_file_Buff[block][off], size);
    return 0;
}

int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block,
                                    lfs_off_t off, const void *buffer, lfs_size_t size) {
    memcpy(&_file_Buff[block][off], buffer, size);
    return 0;
}

int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block) {
    memset(_file_Buff[block], 0x00, sizeof(_file_Buff[block]));
    return 0;
}

int user_provided_block_device_sync(const struct lfs_config *c) {
    return 0;
}

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
        // block device operations
        .read  = user_provided_block_device_read,
        .prog  = user_provided_block_device_prog,
        .erase = user_provided_block_device_erase,
        .sync  = user_provided_block_device_sync,

        // block device configuration
        .read_size = 16,
        .prog_size = 16,
        .block_size = 1024,
        .block_count = 2,
        .cache_size = 16,
        .lookahead_size = 16,
        .block_cycles = 500
};

void test_mkdir() {
    lfs_mkdir(&lfs, "/");
    lfs_mkdir(&lfs, "/root");
    lfs_mkdir(&lfs, "/data");
}

void test_write_boot_count() {
    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    printf("lfs_file_size: %d\n", lfs_file_size(&lfs, &file));
    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);


    // print the boot count
    printf("boot_count: %d\n", boot_count);
}

// entry point
int main(void) {
    // mount the filesystem
    int err = lfs_mount(&lfs, &cfg);
    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        err = lfs_mount(&lfs, &cfg);
    }
    test_mkdir();
    test_write_boot_count();
    test_write_boot_count();
    // release any resources we were using
    lfs_unmount(&lfs);
    return 0;
}
