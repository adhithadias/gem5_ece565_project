#! /bin/bash

# configs/spec2k6/run.py is same as syscall emulation
# it contains spec2006 benchmarks that is added by Tim

./build/ARM/gem5.opt --debug-flags=CacheRepl -d biprp_out \
configs/spec2k6/run.py \
--benchmark=bzip2 \
--maxinsts=1000 \
--cpu-type=O3_ARM_v7a_3 \
--caches --l2cache \
--num-l2caches=1 \
--l1d_size='32kB' \
--l1i_size='32kB' \
--l2_size='1024kB' \
--l1d_assoc=2 \
--l1i_assoc=2 \
--l2_assoc=8 \
--cacheline_size=64 \
--cache_repl='BIPRP'

# TODO

# need to set L1 3 cycles hit? and 2 ports?
# need to set L2 12 cycles hit?

# Memory 300 cycles, 30 for inter-block transfer
# Bus width - 32 bytes
# TLB 256 entries fully associative miss penalty -- 300 cycle (assuming Physical page tables so TLB miss handled by hardware and goes to memory)
# Branch misprediction penalty - 15 cycles.
# CPU -- out of order issue queue 40 entries issue width - 4 Ld/st Q (aka LSQ) size 32 entries 32 entry return address stackabout 8K entries in branch predictor tables

echo "BIP RP job completed"
