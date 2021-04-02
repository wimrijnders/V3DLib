# Known Issues

## Not `OpenGL` compatible

`V3DLib` can not work on a Pi4 with `OpenGL` running. You need to run it without a GUI ('headless'),
except for simple cases such as the `Hello` demo, which only outputs data.
The issue is that the VideoCore L2 cache can not be shared with other applications when `OpenGL` is hogging it.

It *is* possible to disable the L2 cache. This will affect performance badly, though.
Also, from what I understand, you will need a specially compiled linux kernel to deal with a disabled L2 cache.

For `vc4`, there is a workaround for this: use DMA exclusively. For `v3d`, this is not an option.


## 32-bit programs will not run with a 64-bit kernel

While it is certainly possible to run 32-bit programs with a 64-bit kernel, the initialization code
for buffer objects fails. The memory offset returned by the `v3d` device driver is invalid (in fact, it
is the amount of available memory).

To run with a 64-bit kernel, programs using `v3d` will need to be compiled as 64-bits also.


## Some things will not run due to kernel issues

There are still some parts which will compile perfectly but not run properly; notably the `Mandelbrot` demo
will run *sometimes* on `v3d`, and otherwise hang.

**NOTE 20210317:** `Mandelbrot` on 32-bit `v3d` is running fine now.
                   There are still issues on 64-bit, where 'Timer expired' can still occur. Once that happens,
                   the message pops up of every usage.

This is in part due to issues in the linux kernel, see the [Issues page](Doc/Issues.md).
There are also some unit tests which have the same problem, these are disabled when running on `VideoCore VI`.

I haven't been able to resolve these issues and I am waiting for a kernel update with fixes.
All code for the `VideoCore IV` compiles and runs fine.


## Kernel Warning and Hang

**NOTE:**  This appears to be a know bug in the kerner driver.

Issue: https://github.com/raspberrypi/linux/pull/3816
Fix pending: https://github.com/raspberrypi/linux/pull/3816/commits/803f25eb03d2698c79eea495be7dee47c3bb86c2

So it appears we just need to wait.

### Test Platform

Firmware revision number ([Release Notes](https://downloads.raspberrypi.org/raspbian/release_notes.txt)):

```
> sudo /opt/vc/bin/vcgencmd version
Aug 19 2020 17:38:16 
Copyright (c) 2012 Broadcom
version e90cba19a98a0d1f2ef086b9cafcbca00778f094 (clean) (release) (start)
```

Pi model:
```
cat /proc/device-tree/model
Raspberry Pi 4 Model B Rev 1.1
```

Linux kernel version number:
```
> uname -or
5.4.51-v7l+ GNU/Linux
```
 
----- 

The following warning message (or similar) appears in the linux log appears every time a program is
run on `v3d`. It appears to be harmless, but it may have something to do with the actual issue.

In `/var/log/kern.log`:
```
Nov 21 05:11:31 pi4-3 kernel: [72954.507898] WARNING: CPU: 0 PID: 233 at ./include/linux/dma-fence.h:533 drm_sched_main+0x238/0x31c [gpu_sched]
Nov 21 05:11:31 pi4-3 kernel: [72954.507910] Modules linked in: cmac bnep hci_uart btbcm bluetooth ecdh_generic ecc 8021q garp stp llc vc4 cec drm_kms_helper brcmfmac v3d brcmutil gpu_sched sha256_generic libsha256 bcm2835_codec(C) bcm2835_v4l2(C) cfg80211 drm v4l2_mem2mem bcm2835_isp(C) rfkill bcm2835_mmal_vchiq(C) videobuf2_vmalloc videobuf2_dma_contig videobuf2_memops videobuf2_v4l2 drm_panel_orientation_quirks snd_soc_core videobuf2_common snd_compress raspberrypi_hwmon snd_bcm2835(C) snd_pcm_dmaengine videodev vc_sm_cma(C) snd_pcm mc snd_timer snd syscopyarea sysfillrect sysimgblt fb_sys_fops rpivid_mem uio_pdrv_genirq uio ip_tables x_tables ipv6 nf_defrag_ipv6
Nov 21 05:11:31 pi4-3 kernel: [72954.508132] CPU: 0 PID: 233 Comm: v3d_cache_clean Tainted: G        WC        5.4.51-v7l+ #1333
Nov 21 05:11:31 pi4-3 kernel: [72954.508140] Hardware name: BCM2711
Nov 21 05:11:31 pi4-3 kernel: [72954.508147] Backtrace: 
Nov 21 05:11:31 pi4-3 kernel: [72954.508171] [<c020d46c>] (dump_backtrace) from [<c020d768>] (show_stack+0x20/0x24)
Nov 21 05:11:31 pi4-3 kernel: [72954.508184]  r6:d6410000 r5:00000000 r4:c129c8f8 r3:566ec00e
Nov 21 05:11:31 pi4-3 kernel: [72954.508202] [<c020d748>] (show_stack) from [<c0a39a44>] (dump_stack+0xe0/0x124)
Nov 21 05:11:31 pi4-3 kernel: [72954.508220] [<c0a39964>] (dump_stack) from [<c0221c70>] (__warn+0xec/0x104)
Nov 21 05:11:31 pi4-3 kernel: [72954.508233]  r8:00000215 r7:00000009 r6:bf10a610 r5:00000000 r4:00000000 r3:566ec00e
Nov 21 05:11:31 pi4-3 kernel: [72954.508248] [<c0221b84>] (__warn) from [<c0221d40>] (warn_slowpath_fmt+0xb8/0xc0)
Nov 21 05:11:31 pi4-3 kernel: [72954.508260]  r9:bf10a610 r8:00000215 r7:bf1086a0 r6:00000009 r5:00000000 r4:c1204f88
Nov 21 05:11:31 pi4-3 kernel: [72954.508290] [<c0221c8c>] (warn_slowpath_fmt) from [<bf1086a0>] (drm_sched_main+0x238/0x31c [gpu_sched])
Nov 21 05:11:31 pi4-3 kernel: [72954.508302]  r9:00000000 r8:c1204f88 r7:d6cd7e00 r6:00000000 r5:d77d4a00 r4:d6a218a0
Nov 21 05:11:31 pi4-3 kernel: [72954.508330] [<bf108468>] (drm_sched_main [gpu_sched]) from [<c0244e90>] (kthread+0x13c/0x168)
Nov 21 05:11:31 pi4-3 kernel: [72954.508343]  r10:d6a96adc r9:d6e9dae4 r8:bf108468 r7:d6a218a0 r6:00000000 r5:d6911b80
Nov 21 05:11:31 pi4-3 kernel: [72954.508351]  r4:d6a96ac0
Nov 21 05:11:31 pi4-3 kernel: [72954.508365] [<c0244d54>] (kthread) from [<c02010ac>] (ret_from_fork+0x14/0x28)
Nov 21 05:11:31 pi4-3 kernel: [72954.508374] Exception stack(0xd6411fb0 to 0xd6411ff8)
Nov 21 05:11:31 pi4-3 kernel: [72954.508385] 1fa0:                                     00000000 00000000 00000000 00000000
Nov 21 05:11:31 pi4-3 kernel: [72954.508396] 1fc0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
Nov 21 05:11:31 pi4-3 kernel: [72954.508407] 1fe0: 00000000 00000000 00000000 00000000 00000013 00000000
Nov 21 05:11:31 pi4-3 kernel: [72954.508419]  r10:00000000 r9:00000000 r8:00000000 r7:00000000 r6:00000000 r5:c0244d54
Nov 21 05:11:31 pi4-3 kernel: [72954.508427]  r4:d6911b80 r3:40000700
Nov 21 05:11:31 pi4-3 kernel: [72954.508438] ---[ end trace b89a229b224eb0ea ]---
```

### GPU Hang

All example programs except `Mandelbrot` run fine, despite the previous kernel warning popping up.
With `Mandelbrot`, the following eventually pops up after any number of runs (even 1).

In `/var/log/kern.log`:
```
Nov 21 05:11:44 pi4-3 kernel: [72967.800229] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* Resetting GPU for hang.
Nov 21 05:11:44 pi4-3 kernel: [72967.800273] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* V3D_ERR_STAT: 0x00001000
Nov 21 05:11:45 pi4-3 kernel: [72968.310270] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* Resetting GPU for hang.
Nov 21 05:11:45 pi4-3 kernel: [72968.310312] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* V3D_ERR_STAT: 0x00001000
Nov 21 05:11:45 pi4-3 kernel: [72968.820281] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* Resetting GPU for hang.
Nov 21 05:11:45 pi4-3 kernel: [72968.820321] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* V3D_ERR_STAT: 0x00001000
Nov 21 05:11:46 pi4-3 kernel: [72969.330284] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* Resetting GPU for hang.
Nov 21 05:11:46 pi4-3 kernel: [72969.330324] v3d fec00000.v3d: [drm:v3d_reset [v3d]] *ERROR* V3D_ERR_STAT: 0x00001000
... (ad infinitum)
```

The Pi will still function normally after this, but the GPU is blocked.
On trying to run `Mandelbrot` again, eventually the following may be shown on standard output,
and the entire Pi hangs.

On `stdout`:
```
Message from syslogd@pi4-3 at Nov 21 05:12:55 ...    # Gets repeated before every line
 kernel:[73039.151805] Internal error: Oops: a06 [#1] SMP ARM
 kernel:[73039.152018] Process kthreadd (pid: 2, stack limit = 0x5f480f10)
 kernel:[73039.152026] Stack: (0xef8ebe18 to 0xef8ec000)
 kernel:[73039.152035] be00:                                                       c0a5ad3c c0310028
 kernel:[73039.152046] be20: 00000000 c132598c ef8ebe54 ffffffff 00000000 d6e52dc0 00000000 00800700
 kernel:[73039.152058] be40: 00000000 d6e5318c ef8ebedc ef8ebe58 c0220478 c03e358c 00000000 eff4f740
 kernel:[73039.152069] be60: c120e530 00000000 00000000 00000000 00800700 c12b5b80 00000000 00000000
 kernel:[73039.152081] be80: 00000000 efa82180 00000000 00000000 ef8ebf30 c1204f88 00000000 00000000
 kernel:[73039.152092] bea0: 00000000 00000000 ef8ec2f4 566ec00e d5692018 c1204f88 c1204f88 00800700
 kernel:[73039.152103] bec0: 00000000 ef8ea000 00000000 d57bfb00 ef8ebf2c ef8ebee0 c0220800 c021ee04
 kernel:[73039.152115] bee0: ef8ebf5c ef8ebf30 c0a54f70 c024ce24 c020cfe0 c020bbc8 c020bbec 566ec00e
 kernel:[73039.152126] bf00: 00000000 c1204f88 c120e5b0 c12b6484 00000000 ef8ea000 00000001 d57bfb00
 kernel:[73039.152138] bf20: ef8ebf74 ef8ebf30 c0220c0c c0220780 00800700 00000000 00000000 00000000
 kernel:[73039.152149] bf40: 00000000 00000011 c0244d54 d57bfb00 00000000 00000000 c0a5545c 566ec00e
 kernel:[73039.152160] bf60: d5c8a8d4 d57bfb14 ef8ebfac ef8ebf78 c0246284 c0220ba4 ef8ebf94 566ec00e
 kernel:[73039.152172] bf80: c0230370 00000000 c0246090 00000000 00000000 00000000 00000000 00000000
 kernel:[73039.152183] bfa0: 00000000 ef8ebfb0 c02010ac c024609c 00000000 00000000 00000000 00000000
 kernel:[73039.152194] bfc0: 00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000
 kernel:[73039.152205] bfe0: 00000000 00000000 00000000 00000000 00000013 00000000 00000000 00000000
 kernel:[73039.152409] Code: e1a00000 e5953014 e3540000 e3a02000 (e7842003) 
```

There is a [pending fix](https://github.com/raspberrypi/linux/pull/3816) for this.
I am anxiously waiting for its release.


## Fixed Issues

This lists things never to forget.

## Pi1 QPUs not enabled

Call to `qpu_enable()` in `Mailbox.cpp` was failing. `/boot/config.txt` was as follows:

```
...
[pi4]
# Enable DRM VC4 V3D driver on top of the dispmanx display stack
dtoverlay=vc4-fkms-v3d
max_framebuffers=2

[all]
#dtoverlay=vc4-fkms-v3d
dtoverlay=vc4-kms-v3d
gpu_mem=128
```

Turns out that `dtoverlay` shouldi *not* be specified for Pi1. Commenting out `dtoverlay` in the all-section
resolved the issue.
 
