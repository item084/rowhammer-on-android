# RowHammer in Smartphones [EECS582 course project]

## Content

- `rh-kernel` is the implementation of experiments on Nexus 5 from the kernel for memory test.
- `rh-kernel-evict` is the implementation of experiments on Nexus 5 from the kernel for evict-based RowHammer.
- `rh-use` is the implementation of RowHammers from user space. It also include the row size detector and experiment results.
- `drammer-impl` is the implementation of `drammer` attack from native binary. Adapted from [drammer](https://github.com/vusec/drammer).
- `Lowmem` is the Java wrapper implementation for drammer binary to avoid Low Memory Killer.
