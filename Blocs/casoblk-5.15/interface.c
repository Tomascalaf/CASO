// SPDX-License-Identifier: GPL-2.0-only
/*
 * Ram backed block device driver.
 *
 * Copyright (C) 2007 Nick Piggin
 * Copyright (C) 2007 Novell Inc.
 *
 * Parts derived from drivers/block/rd.c, and drivers/block/loop.c, copyright
 * of their respective owners.
 */

/*
 * Adaptat com a pràctica de CASO, abril 2023
 */

#include <linux/init.h>
#include <linux/initrd.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/pagemap.h>
#include <linux/radix-tree.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/backing-dev.h>
#include <linux/debugfs.h>

#include <linux/uaccess.h>

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/hdreg.h>
#include "interface.h"


/**************** Interficie de les funcions que heu d'implementar **********/

// funcio per demanar la memoria que mantindra les dades del disc.
// Usa p = vmalloc(PAGE_ALIGN(size)) per demanar-la al kernel
// Inicialitza la memoria fent que cada caracter valgui i%256:
//     for (i=0; i < size; ++i) p[i] = i;
char * alloc_disk_memory(u64 size);

// funcio per alliberar la memoria que mante les dades del disc.
// Usa vfree(p) per alliberar
void   free_disk_memory(char * disk_mem);


// funcio per retornar la geometria del disc.
// Accedeix al vostre disc (bdev->bd_disk->private_data, de tipus xrd_struct)
//  omple l'estructura que rep per referencia (geo):
///    struct hd_geometry {
///          unsigned char heads;
///          unsigned char sectors;
///          unsigned short cylinders;
///          unsigned long start;
///    };
// plena amb heads= 32
//           sectors= 128
//           cylinders= <mida del disc> / geo->heads/geo->sectors/SECTOR_SIZE
//           start= 0
// podeu trobar la mida del disc a l'estructura xrd_struct, camp "size".
// no oblideu fer un return 0; per indicar que tot ha anat be.
int xrd_getgeo(struct block_device * bdev, struct hd_geometry *geo);


// funcio per llegir del ramdisk. Rep:
//  dst: adre�a en l'espai del kernel on escriure la informacio
//  xrd: punter a l'estructura del nostre disc
//  sector: sector del disc d'on s'ha de llegir
//  n: numero de bytes que volem llegir
void copy_from_xrd(void *dst, struct xrd_device *xrd,
                        sector_t sector, size_t n);

// funcio per escriure al ramdisk. Rep:
// xrd: punter a l'estructura del nostre disc
// src: adre�a en l'espai del kernel d'on llegir la informacio
// sector: sector del disc on volem escriure
// n: numero de bytes que volem escriure
void copy_to_xrd(struct xrd_device *xrd, void *src,
                        sector_t sector, size_t n);


/****************************************************************************/


/***** Funcions de la interficie d'operacions del dispositiu de bloc ********/
/*                    NO les heu de canviar                                 */
/*             Nomes son aqui per veure quan es criden                      */


/*
 * copy_to_xrd_setup must be called before copy_to_xrd. It may sleep.
 */
static int copy_to_xrd_setup(struct xrd_device *xrd, sector_t sector, size_t n)
{
#ifdef DEBUG
	printk(KERN_DEBUG "copy_to_xrd_setup sector %llu, size %ld\n", sector, n);
#endif
	return 0;
}


/*
 * Process a single bvec of a bio.
 */
static int xrd_do_bvec(struct xrd_device *xrd, struct page *page,
			unsigned int len, unsigned int off, unsigned int op,
			sector_t sector)
{
	void *mem;
	int err = 0;
#ifdef DEBUG
	printk(KERN_DEBUG "xrd_do_bvec page %p len %d off %d op %d sector %llu\n", 
			page, len, off, op, sector);
#endif

	if (op_is_write(op)) {
		err = copy_to_xrd_setup(xrd, sector, len);
		if (err)
			goto out;
	}
	mem = kmap_atomic(page);
	if (!op_is_write(op)) {
		copy_from_xrd(mem + off, xrd, sector, len);
		flush_dcache_page(page);
	} else {
		flush_dcache_page(page);
		copy_to_xrd(xrd, mem + off, sector, len);
		//err = -EIO;
	}
	kunmap_atomic(mem);

out:
	if (err) printk(KERN_DEBUG "xrd_do_bvec page %p err %d\n", page, err);
	return err;
}

static blk_qc_t xrd_submit_bio(struct bio *bio)
{
	struct xrd_device *xrd = bio->bi_bdev->bd_disk->private_data;
	sector_t sector = bio->bi_iter.bi_sector;
	struct bio_vec bvec;
	struct bvec_iter iter;

#ifdef DEBUG
	printk(KERN_DEBUG "xrd_submit_bio... sector %llu\n", sector);
#endif

	bio_for_each_segment(bvec, bio, iter) {
		unsigned int len = bvec.bv_len;
		int err;

		/* Don't support un-aligned buffer */
		WARN_ON_ONCE((bvec.bv_offset & (SECTOR_SIZE - 1)) ||
				(len & (SECTOR_SIZE - 1)));

		err = xrd_do_bvec(xrd, bvec.bv_page, len, bvec.bv_offset,
				  bio_op(bio), sector);
		if (err)
			goto io_error;
		sector += len >> SECTOR_SHIFT;
	}

	bio_endio(bio);
	return BLK_QC_T_NONE;
io_error:
	printk(KERN_DEBUG "xrd_submit_bio: io_error\n");
	bio_io_error(bio);
	return BLK_QC_T_NONE;
}

static int xrd_rw_page(struct block_device *bdev, sector_t sector,
		       struct page *page, unsigned int op)
{
	struct xrd_device *xrd = bdev->bd_disk->private_data;
	int err;
#ifdef DEBUG
	printk (KERN_DEBUG "xrd_rw_page sector %llu, page %p, op %d\n", 
			sector, page, op);
#endif

	if (PageTransHuge(page))
		return -ENOTSUPP;
	err = xrd_do_bvec(xrd, page, PAGE_SIZE, 0, op, sector);
	page_endio(page, op_is_write(op), err);
	return err;
}

static const struct block_device_operations xrd_fops = {
	.owner =		THIS_MODULE,
	.submit_bio =		xrd_submit_bio,
	.rw_page =		xrd_rw_page,
	.getgeo =               xrd_getgeo,
};

/*
 * And now the modules code and kernel interface.
 */
static int rd_nr = XRD_MAX_DISKS;
module_param(rd_nr, int, 0444);
MODULE_PARM_DESC(rd_nr, "Maximum number of xrd devices");

unsigned long rd_size = XRD_SIZE;
module_param(rd_size, ulong, 0444);
MODULE_PARM_DESC(rd_size, "Size of each RAM disk in kbytes.");

static int max_part = 4;
module_param(max_part, int, 0444);
MODULE_PARM_DESC(max_part, "Num Minors to reserve between devices");

MODULE_LICENSE("GPL");
MODULE_ALIAS_BLOCKDEV_MAJOR(XRD_MAJOR);
MODULE_ALIAS("xrd");

#ifndef MODULE
/* Legacy boot options - nonmodular */
static int __init ramdisk_size(char *str)
{
	rd_size = simple_strtol(str, NULL, 0);
	return 1;
}
__setup("ramdisk_size=", ramdisk_size);
#endif

/*
 * The device scheme is derived from loop.c. Keep them in synch where possible
 * (should share code eventually).
 */
static LIST_HEAD(xrd_devices);
static DEFINE_MUTEX(xrd_devices_mutex);
static struct dentry *xrd_debugfs_dir;

static int xrd_alloc(int i)
{
	struct xrd_device *xrd;
	struct gendisk *disk;
	char buf[DISK_NAME_LEN];
	int err = -ENOMEM;

	mutex_lock(&xrd_devices_mutex);
	list_for_each_entry(xrd, &xrd_devices, xrd_list) {
		if (xrd->xrd_number == i) {
			mutex_unlock(&xrd_devices_mutex);
			return -EEXIST;
		}
	}
	xrd = kzalloc(sizeof(*xrd), GFP_KERNEL);
	if (!xrd) {
		mutex_unlock(&xrd_devices_mutex);
		return -ENOMEM;
	}
	xrd->xrd_number		= i;
	list_add_tail(&xrd->xrd_list, &xrd_devices);
	mutex_unlock(&xrd_devices_mutex);
#if 0
	spin_lock_init(&xrd->xrd_lock);
	INIT_RADIX_TREE(&xrd->xrd_pages, GFP_ATOMIC);
#endif

	snprintf(buf, DISK_NAME_LEN, "xrd%d", i);
	if (!IS_ERR_OR_NULL(xrd_debugfs_dir))
		debugfs_create_u64(buf, 0444, xrd_debugfs_dir,
				&xrd->size);

	disk = xrd->xrd_disk = blk_alloc_disk(NUMA_NO_NODE);
	if (!disk)
		goto out_free_dev;

	disk->major		= XRD_MAJOR;
	disk->first_minor	= i * max_part;
	disk->minors		= max_part;
	disk->fops		= &xrd_fops;
	disk->private_data	= xrd;
	disk->flags		= GENHD_FL_EXT_DEVT;
	strlcpy(disk->disk_name, buf, DISK_NAME_LEN);
				    // set_capacity takes sectors (512 bytes)
	set_capacity(disk, rd_size * 2);

	xrd->size = rd_size*1024;  // rd_size is in KBytes
	xrd->disk_memory = alloc_disk_memory(xrd->size);

	if (xrd->disk_memory==NULL)
		goto out_free_disk;
	
	/*
	 * This is so fdisk will align partitions on 4k, because of
	 * direct_access API needing 4k alignment, returning a PFN
	 * (This is only a problem on very small devices <= 4M,
	 *  otherwise fdisk will align on 1M. Regardless this call
	 *  is harmless)
	 */
	blk_queue_physical_block_size(disk->queue, PAGE_SIZE);

	/* Tell the block layer that this is not a rotational device */
	blk_queue_flag_set(QUEUE_FLAG_NONROT, disk->queue);
	blk_queue_flag_clear(QUEUE_FLAG_ADD_RANDOM, disk->queue);
	blk_queue_flag_set(QUEUE_FLAG_NOWAIT, disk->queue);
	err = add_disk(disk);
	if (err)
		goto out_cleanup_disk;

	return 0;

out_cleanup_disk:
	blk_cleanup_disk(disk);
out_free_disk:
out_free_dev:
	mutex_lock(&xrd_devices_mutex);
	list_del(&xrd->xrd_list);
	mutex_unlock(&xrd_devices_mutex);
	kfree(xrd);
	return err;
}

static void xrd_probe(dev_t dev)
{
	xrd_alloc(MINOR(dev) / max_part);
}

static void xrd_del_one(struct xrd_device *xrd)
{
	del_gendisk(xrd->xrd_disk);
	blk_cleanup_disk(xrd->xrd_disk);
	free_disk_memory(xrd->disk_memory);
	printk(KERN_DEBUG "Disk memory deallocated\n");
	mutex_lock(&xrd_devices_mutex);
	list_del(&xrd->xrd_list);
	mutex_unlock(&xrd_devices_mutex);
	kfree(xrd);
}

static inline void xrd_check_and_reset_par(void)
{
	if (unlikely(!max_part))
		max_part = 1;

	/*
	 * make sure 'max_part' can be divided exactly by (1U << MINORBITS),
	 * otherwise, it is possiable to get same dev_t when adding partitions.
	 */
	if ((1U << MINORBITS) % max_part != 0)
		max_part = 1UL << fls(max_part);

	if (max_part > XRD_MAX_PARTS) {
		pr_info("xrd: max_part can't be larger than %d, reset max_part = %d.\n",
			XRD_MAX_PARTS, XRD_MAX_PARTS);
		max_part = XRD_MAX_PARTS;
	}
}

static int __init xrd_init(void)
{
	struct xrd_device *xrd, *next;
	int err, i;

	printk(KERN_DEBUG "xrd_init: Hello All World\n");

	/*
	 * xrd module now has a feature to instantiate underlying device
	 * structure on-demand, provided that there is an access dev node.
	 *
	 * (1) if rd_nr is specified, create that many upfront. else
	 *     it defaults to XRD_MAX_DISKS
	 * (2) User can further extend xrd devices by create dev node themselves
	 *     and have kernel automatically instantiate actual device
	 *     on-demand. Example:
	 *		mknod /path/devnod_name b 1 X	# 1 is the rd major
	 *		fdisk -l /path/devnod_name
	 *	If (X / max_part) was not already created it will be created
	 *	dynamically.
	 */

	if (__register_blkdev(XRD_MAJOR, "xrd", xrd_probe))
		return -EIO;

	xrd_check_and_reset_par();

#if 0

	xrd_debugfs_dir = debugfs_create_dir("xrd_pages", NULL);
#endif

	for (i = 0; i < rd_nr; i++) {
		err = xrd_alloc(i);
		if (err)
			goto out_free;
	}

	pr_info("xrd: module loaded\n");
	return 0;

out_free:
	unregister_blkdev(XRD_MAJOR, "xrd");
	debugfs_remove_recursive(xrd_debugfs_dir);

	list_for_each_entry_safe(xrd, next, &xrd_devices, xrd_list)
		xrd_del_one(xrd);

	pr_info("xrd: module NOT loaded !!!\n");
	return err;
}

static void __exit xrd_exit(void)
{
	struct xrd_device *xrd, *next;

	printk(KERN_DEBUG "xrd_exit: Bye Cruel World\n");

	unregister_blkdev(XRD_MAJOR, "xrd");
	debugfs_remove_recursive(xrd_debugfs_dir);

	list_for_each_entry_safe(xrd, next, &xrd_devices, xrd_list)
		xrd_del_one(xrd);

	pr_info("xrd: module unloaded\n");
}

module_init(xrd_init);
module_exit(xrd_exit);

