//
// Created by caesar kekxv on 2020/9/15.
//

#include <cstdio>
#include "lfs.h"
#include "lfs_util.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// Block device specific tracing
#ifdef LFS_RAMBD_YES_TRACE
#define LFS_RAMBD_TRACE(...) LFS_TRACE(__VA_ARGS__)
#else
#define LFS_RAMBD_TRACE(...)
#endif

// rambd config (optional)
struct lfs_rambd_config {
    // 8-bit erase value to simulate erasing with. -1 indicates no erase
    // occurs, which is still a valid block device
    int32_t erase_value;

    // Optional statically allocated buffer for the block device.
    void *buffer;
};

// rambd state
typedef struct lfs_rambd {
    uint8_t *buffer;
    const struct lfs_rambd_config *cfg;
} lfs_rambd_t;

lfs_rambd_t rambd_t;

// Create a RAM block device using the geometry in lfs_config
int lfs_rambd_create(const struct lfs_config *cfg);

int lfs_rambd_createcfg(const struct lfs_config *cfg,
                        const struct lfs_rambd_config *bdcfg);

// Clean up memory associated with block device
int lfs_rambd_destroy(const struct lfs_config *cfg);

// Read a block
int lfs_rambd_read(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size);

// Program a block
//
// The block must have previously been erased.
int lfs_rambd_prog(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, const void *buffer, lfs_size_t size);

// Erase a block
//
// A block must be erased before being programmed. The
// state of an erased block is undefined.
int lfs_rambd_erase(const struct lfs_config *cfg, lfs_block_t block);

// Sync the block device
int lfs_rambd_sync(const struct lfs_config *cfg);


// configuration of the filesystem is provided by this struct
struct lfs_config cfg;

// entry point
int main(void) {
    {
        cfg.context = (void *) &rambd_t;
        // block device operations
        cfg.read = lfs_rambd_read;
        cfg.prog = lfs_rambd_prog;
        cfg.erase = lfs_rambd_erase;
        cfg.sync = lfs_rambd_sync;

        // block device configuration
        cfg.read_size = 16;
        cfg.prog_size = 16;
        cfg.block_size = 1024;
        cfg.block_count = 2;
        cfg.block_cycles = 500;
        cfg.cache_size = 16;
        cfg.lookahead_size = 16;
        cfg.read_buffer = NULL;
        cfg.prog_buffer = NULL;
        cfg.lookahead_buffer = NULL;
        cfg.name_max = 0;
        cfg.file_max = 0;
        cfg.attr_max = 0;
    };
    int err = lfs_rambd_create(&cfg);
    if (err) {
        return 1;
    }
    // mount the filesystem
    err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    // read current count
    uint32_t boot_count = 0;
    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

    // update boot count
    boot_count += 1;
    lfs_file_rewind(&lfs, &file);
    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

    // remember the storage is not updated until the file is closed successfully
    lfs_file_close(&lfs, &file);

    // release any resources we were using
    lfs_unmount(&lfs);

    // print the boot count
    printf("boot_count: %d\n", boot_count);
    lfs_rambd_destroy(&cfg);
    return 0;
}


int lfs_rambd_createcfg(const struct lfs_config *cfg,
                        const struct lfs_rambd_config *bdcfg) {
    LFS_RAMBD_TRACE("lfs_rambd_createcfg(%p {.context=%p, "
                    ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                    ".read_size=%" PRIu32 ", .prog_size=%" PRIu32 ", "
                                                                  ".block_size=%" PRIu32 ", .block_count=%" PRIu32 "}, "
                                                                                                                   "%p {.erase_value=%" PRId32 ", .buffer=%p})",
                    (void *) cfg, cfg->context,
                    (void *) (uintptr_t) cfg->read, (void *) (uintptr_t) cfg->prog,
                    (void *) (uintptr_t) cfg->erase, (void *) (uintptr_t) cfg->sync,
                    cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count,
                    (void *) bdcfg, bdcfg->erase_value, bdcfg->buffer);
    lfs_rambd_t *bd = (lfs_rambd_t *) cfg->context;
    bd->cfg = bdcfg;

    // allocate buffer?
    if (bd->cfg->buffer) {
        bd->buffer = (uint8_t *) bd->cfg->buffer;
    } else {
        bd->buffer = (uint8_t *) lfs_malloc(cfg->block_size * cfg->block_count);
        if (!bd->buffer) {
            LFS_RAMBD_TRACE("lfs_rambd_createcfg -> %d", LFS_ERR_NOMEM);
            return LFS_ERR_NOMEM;
        }
    }

    // zero for reproducability?
    if (bd->cfg->erase_value != -1) {
        memset(bd->buffer, bd->cfg->erase_value,
               cfg->block_size * cfg->block_count);
    }

    LFS_RAMBD_TRACE("lfs_rambd_createcfg -> %d", 0);
    return 0;
}

int lfs_rambd_create(const struct lfs_config *cfg) {
    LFS_RAMBD_TRACE("lfs_rambd_create(%p {.context=%p, "
                    ".read=%p, .prog=%p, .erase=%p, .sync=%p, "
                    ".read_size=%" PRIu32 ", .prog_size=%" PRIu32 ", "
                                                                  ".block_size=%" PRIu32 ", .block_count=%" PRIu32 "})",
                    (void *) cfg, cfg->context,
                    (void *) (uintptr_t) cfg->read, (void *) (uintptr_t) cfg->prog,
                    (void *) (uintptr_t) cfg->erase, (void *) (uintptr_t) cfg->sync,
                    cfg->read_size, cfg->prog_size, cfg->block_size, cfg->block_count);
    static const struct lfs_rambd_config defaults = {.erase_value=-1};
    int err = lfs_rambd_createcfg(cfg, &defaults);
    LFS_RAMBD_TRACE("lfs_rambd_create -> %d", err);
    return err;
}

int lfs_rambd_destroy(const struct lfs_config *cfg) {
    LFS_RAMBD_TRACE("lfs_rambd_destroy(%p)", (void *) cfg);
    // clean up memory
    lfs_rambd_t *bd = (lfs_rambd_t *) cfg->context;
    if (!bd->cfg->buffer) {
        lfs_free(bd->buffer);
    }
    LFS_RAMBD_TRACE("lfs_rambd_destroy -> %d", 0);
    return 0;
}

int lfs_rambd_read(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, void *buffer, lfs_size_t size) {
    LFS_RAMBD_TRACE("lfs_rambd_read(%p, "
                    "0x%" PRIx32 ", %" PRIu32", %p, %" PRIu32 ")",
                    (void *) cfg, block, off, buffer, size);
    lfs_rambd_t *bd = (lfs_rambd_t *) cfg->context;

    // check if read is valid
    LFS_ASSERT(off % cfg->read_size == 0);
    LFS_ASSERT(size % cfg->read_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // read data
    memcpy(buffer, &bd->buffer[block * cfg->block_size + off], size);

    LFS_RAMBD_TRACE("lfs_rambd_read -> %d", 0);
    return 0;
}

int lfs_rambd_prog(const struct lfs_config *cfg, lfs_block_t block,
                   lfs_off_t off, const void *buffer, lfs_size_t size) {
    LFS_RAMBD_TRACE("lfs_rambd_prog(%p, "
                    "0x%" PRIx32 ", %" PRIu32 ", %p, %" PRIu32 ")",
                    (void *) cfg, block, off, buffer, size);
    lfs_rambd_t *bd = (lfs_rambd_t *) cfg->context;

    // check if write is valid
    LFS_ASSERT(off % cfg->prog_size == 0);
    LFS_ASSERT(size % cfg->prog_size == 0);
    LFS_ASSERT(block < cfg->block_count);

    // check that data was erased? only needed for testing
    if (bd->cfg->erase_value != -1) {
        for (lfs_off_t i = 0; i < size; i++) {
            LFS_ASSERT(bd->buffer[block * cfg->block_size + off + i] ==
                       bd->cfg->erase_value);
        }
    }

    // program data
    memcpy(&bd->buffer[block * cfg->block_size + off], buffer, size);

    LFS_RAMBD_TRACE("lfs_rambd_prog -> %d", 0);
    return 0;
}

int lfs_rambd_erase(const struct lfs_config *cfg, lfs_block_t block) {
    LFS_RAMBD_TRACE("lfs_rambd_erase(%p, 0x%" PRIx32 ")", (void *) cfg, block);
    lfs_rambd_t *bd = (lfs_rambd_t *) cfg->context;

    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);

    // erase, only needed for testing
    if (bd->cfg->erase_value != -1) {
        memset(&bd->buffer[block * cfg->block_size],
               bd->cfg->erase_value, cfg->block_size);
    }

    LFS_RAMBD_TRACE("lfs_rambd_erase -> %d", 0);
    return 0;
}

int lfs_rambd_sync(const struct lfs_config *cfg) {
    LFS_RAMBD_TRACE("lfs_rambd_sync(%p)", (void *) cfg);
    // sync does nothing because we aren't backed by anything real
    (void) cfg;
    LFS_RAMBD_TRACE("lfs_rambd_sync -> %d", 0);
    return 0;
}