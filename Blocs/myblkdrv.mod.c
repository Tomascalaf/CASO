#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x32e21920, "module_layout" },
	{ 0x26087692, "kmalloc_caches" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x59d2a97, "param_ops_int" },
	{ 0x5b590f79, "blk_cleanup_disk" },
	{ 0x720a27a7, "__register_blkdev" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x999e8297, "vfree" },
	{ 0xa648e561, "__ubsan_handle_shift_out_of_bounds" },
	{ 0x97651e6c, "vmemmap_base" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xa7bfbf2f, "current_task" },
	{ 0x9b50f1d4, "del_gendisk" },
	{ 0x7df9c00c, "debugfs_remove" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xb4d55af4, "set_capacity" },
	{ 0x23e0dc2e, "bio_endio" },
	{ 0xb5a459dc, "unregister_blkdev" },
	{ 0x7cd8d75e, "page_offset_base" },
	{ 0x56fd44e0, "blk_queue_flag_clear" },
	{ 0xf76b3852, "blk_queue_flag_set" },
	{ 0xa916b694, "strnlen" },
	{ 0xab122c90, "blk_queue_physical_block_size" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xf35141b2, "kmem_cache_alloc_trace" },
	{ 0x37a0cba, "kfree" },
	{ 0x69acdf38, "memcpy" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x4d1448d7, "device_add_disk" },
	{ 0x4678fe15, "param_ops_ulong" },
	{ 0x5ebf7eed, "page_endio" },
	{ 0xe39f65a5, "__blk_alloc_disk" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "45AC9156CBF61A7EFCE032D");
