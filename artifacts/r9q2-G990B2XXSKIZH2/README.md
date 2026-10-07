# Galaxy S21 FE SM-G990B2 (G990B2XXSKIZH2) payload

Exact firmware profile for the Samsung Galaxy S21 FE 5G on firmware
`G990B2XXSKIZH2` (`samsung/r9q2xeea/r9q:16/BP2A.250605.031.A3/G990B2XXSKIZH2:user/release-keys`),
kernel `5.4.289-qgki-32192773-abG990B2XXSKIZH2` (Qualcomm SM8350 / QGKI), Android 16.

## Device profile

| Field | Value |
| --- | --- |
| Model | SM-G990B2 |
| Chipset | Qualcomm SM8350 (Snapdragon 888) |
| Android | 16 |
| Build ID | BP2A.250605.031.A3 |
| Firmware | G990B2XXSKIZH2 |
| Kernel | 5.4.289-qgki-32192773-abG990B2XXSKIZH2 |
| rt_mutex_waiter | Legacy (5.4): two rb_node, no ww_ctx |
| KMI | android13-5.4 QGKI |
| Slide oracle | configfs-only (CONFIG_DEBUG_FS not set — no tracefs) |
| KASLR slots | 32 × 64 KB |

## Hardware evidence

> **Pending device test.** Profile has been compiled and statically verified.
> Fingerprint confirmed against live device via `adb shell getprop ro.build.fingerprint`.

## Files

| File | SHA-256 |
| --- | --- |
| `cve-2026-43499-app.so` | `93d7f43ef96d52757fccf713de92879131ea61d04d8489b22c0abc2bb440b4cc` |
| `cve-2026-43499-root` | `93d7f43ef96d52757fccf713de92879131ea61d04d8489b22c0abc2bb440b4cc` |

`cve-2026-43499-app.so` is built from this tree
(`make TARGET=r9q2-G990B2XXSKIZH2 API=35`, Android NDK r27c).

**KernelSU module** (`android13-5.4_kernelsu-G990B2XXSKIZH2-kdp.ko`) is pending —
requires Samsung opensource tree for SM-G990B / r9q, kernel 5.4.289 QGKI.

## Build

```sh
make TARGET=r9q2-G990B2XXSKIZH2 ANDROID_NDK_HOME=/path/to/android-ndk-r27c
```

## Notes

- No tracefs on this device (`CONFIG_DEBUG_FS not set`). KASLR slide oracle
  uses the configfs-only route (`P0_ORACLE_GATE_SLOT` / `P0_ORACLE_PROBE_SLOT`).
- Legacy `rt_mutex_waiter` layout (5.4 QGKI): two separate `rb_node` fields,
  no `ww_ctx`, no `savestate`.
- `KMALLOC_CGROUP_TYPE=1` (CONFIG_MEMCG_KMEM=y → 3 cache types).
- `APP_FOPS_ROUTE_FINE_DELAY_TICKS 0ULL` (single timing value, no tracefs sweep needed).
