#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/hdreg.h>
#include <linux/uaccess.h>
#include <linux/blkdev.h>
#include <linux/bio.h>

#include "interface.h"


char * alloc_disk_memory(u64 size)
{
   u64 i;
   char * p = NULL;

   // Usar vmalloc(size) per demanar memoria de dades del disc.
   // Aquesta memoria estara en l'espai del kernel.
   // linux/vmalloc.h
   p = vmalloc(PAGE_ALIGN(size));
   // Inicialitzar memoria, patro conegut 
   char j = 0;
   for(i = 0; i < size-1; i++){
   	p[i] = j;
	if(j == 15) j = 0;
	else j++;
   }

   printk(KERN_DEBUG "alloc_disk_memory %ld address %p\n", size, p);

   // retorna el punter a la memòria del kernel per guardar les dades del disc
   return p;
}


void   free_disk_memory(char * disk_mem)
{
   // la vostra implementacio va aqui
   // Evitar memory leak.
   // Usar vfree(const void *addr)
   vfree(disk_mem);


   // fi de la vostra implementacio
   printk(KERN_DEBUG "free_disk_memory %p\n", disk_mem);
}

int xrd_getgeo(struct block_device * bdev, struct hd_geometry * g)
{
	int err = -ENODEV;
    struct xrd_device *xrd = bdev->bd_disk->private_data;
   // la vostra implementacio va aqui
	g->heads = 32;
	g->sectors = 128;
	g->cylinders = xrd->size;
	g->start = 0;
	return 0;

   // fi de la vostra implementacio

        printk (KERN_DEBUG "xrd_getgeo returns\n");
        printk (KERN_DEBUG "    heads %d sectors %d\n", g->heads, g->sectors);
        printk (KERN_DEBUG "    cylinders %d start %ld\n", g->cylinders, g->start);
        return err;
}

/*
 * Copy n bytes to dst from the xrd starting at sector. Does not sleep.
 */
void copy_from_xrd(void *dst, struct xrd_device *xrd,
                        sector_t sector, size_t n)
{
        void *src;
        unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
        size_t copy;

        copy = min_t(size_t, n, PAGE_SIZE - offset);
#ifdef DEBUG
        printk(KERN_DEBUG "copy_from_xrd sector %llu, size %ld copy %ld (%d %d)\n",
                        sector, n, copy, PAGE_SECTORS, SECTOR_SHIFT);
#endif

   // la vostra implementacio va aqui
	u64 pos = sector * SECTOR_SIZE;
	src = xrd->disk_memory + pos;
	memcpy(dst, src, n);


   // fi de la vostra implementacio
}


/*
 * Copy n bytes from src to the xrd starting at sector. Does not sleep.
 */
void copy_to_xrd(struct xrd_device *xrd, const void *src,
                        sector_t sector, size_t n)
{
        void *dst;
        unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
        size_t copy;

        copy = min_t(size_t, n, PAGE_SIZE - offset);
#ifdef DEBUG
        printk(KERN_DEBUG "copy_to_xrd sector %llu, size %ld copy %ld %d\n",
                                sector, n, copy, offset);
#endif
   // la vostra implementacio va aqui
	u64 pos = sector * SECTOR_SIZE;
	dst = xrd->disk_memory + pos;
	memcpy(dst, src, n);
	


   // fi de la vostra implementacio
}

