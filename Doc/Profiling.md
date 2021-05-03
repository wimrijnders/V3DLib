# Profiling Matrix multiplication


| ![vc4 full multiplication](./images/vc4_full_mult_qpus.png) | ![vc4 block multiplication](./images/vc4_block_mult_qpus.png) |
|:---:|:---:|
| `vc4` full multiplication | `v4c` block multiplication |

What you see here, is that the performance scales nicely with the number of used QPUs.
This is true for the full matrix calculation as well as the block calculation.

The calculation also scales for `v3d`:
![v3d multiplication](./images/v3d_mult_qpus.png)

From here on, the profiling times for the maximum number of QPUs will be compared for `v3d` and `vc4`.

Comparing full and block multiplication for `vc4`:

![vc4 comparing full and block multiplication](./images/vc4_full_block_mult.png)

Comparing full and block multiplication for `v3d`:

![v3d comparing full and block multiplication](./images/v3d_full_block_mult.png)

