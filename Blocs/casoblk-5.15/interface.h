
/* Definitions for the implementation functions in XRD */


#define XRD_MAJOR (240)

#define XRD_MAX_PARTS (4)

#define XRD_MAX_DISKS (2)

#define XRD_SIZE (16384)

/*
 * Each block ramdisk device has a memory_area that stores
 * the pages containing the block device's contents.
 */
struct xrd_device {
        int                     xrd_number;
        struct gendisk          *xrd_disk;
        struct list_head        xrd_list;

        /*
         * Backing store of pages and size. This is the contents
         * of the block device.
         */
        char                    *disk_memory;
        u64                     size;
};


