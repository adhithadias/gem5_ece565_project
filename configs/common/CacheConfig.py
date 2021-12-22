# Copyright (c) 2012-2013, 2015-2016 ARM Limited
# Copyright (c) 2020 Barkhausen Institut
# All rights reserved
#
# The license below extends only to copyright in the software and shall
# not be construed as granting a license to any other intellectual
# property including but not limited to intellectual property relating
# to a hardware implementation of the functionality of the software
# licensed hereunder.  You may use the software subject to the license
# terms below provided that you ensure that this notice is replicated
# unmodified and in its entirety in all distributions of the software,
# modified or unmodified, in source code or in binary form.
#
# Copyright (c) 2010 Advanced Micro Devices, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Configure the M5 cache hierarchy config in one place
#

from __future__ import print_function
from __future__ import absolute_import

import math
import m5
from m5.objects import *
from common.Caches import *
from common import ObjectList

def Log2(x):
    return (math.log10(x) / math.log10(2))

def isPowerOfTwo(n):
    return (math.ceil(Log2(n)) == math.floor(Log2(n)))

def set_cache_repl_policy(options, cache_repl_policy, cache):
    if (cache_repl_policy == "LRURP"):
        cache.replacement_policy = LRURP()
    elif (cache_repl_policy == "LIPRP"):
        cache.replacement_policy = LIPRP()
    elif (cache_repl_policy == "BIPRP"):
        cache.replacement_policy = BIPRP(
            btp=options.cache_btp
        )
    elif (cache_repl_policy == "SRRIPRP"):
        cache.replacement_policy = RRIPRP(
            num_bits=options.cache_num_bits
        )
    elif (cache_repl_policy == "BRRIPRP"):
        cache.replacement_policy = BRRIPRP(
            btp=options.cache_btp,
            num_bits=options.cache_num_bits
        )
    elif (cache_repl_policy == "DRRIPRP"):
        print("initializing DRRIP cache with settings \n constituency:",
              options.cache_constituency_size, ", team_size: ",
              options.cache_team_size)

        if(not(isPowerOfTwo(options.cacheline_size))):
            print("Cache line size should be a power of 2\n")
            sys.exit(1)
        block_offset = int(Log2(options.cacheline_size))
        print("block_offset: ", block_offset)

        l2_cache_size = int(options.l2_size[0:-2])
        print("l2 size: ", l2_cache_size)

        if (options.l2_size[-2:] == "kB"):
            l2_cache_size = l2_cache_size * 1024
        elif (options.l2_size[-2:] == "MB"):
            l2_cache_size = l2_cache_size * 1024 * 1024
        else:
            print("L2 cache size should be either in kB or MB\n")
            sys.exit(1)

        num_cachelines = l2_cache_size / options.cacheline_size
        print("num_cachelines: ", num_cachelines)
        num_sets = num_cachelines / options.l2_assoc
        print("num_cache_sets: ", num_sets)
        set_offset = int(Log2(num_sets))
        print("set_offset: ", set_offset)

        cache.replacement_policy = DIPRP(
            replacement_policy_1=RRIPRP(
                num_bits=options.cache_num_bits
            ),
            replacement_policy_2=BRRIPRP(
                btp=options.cache_btp,
                num_bits=options.cache_num_bits
            ),
            constituency_size=options.cache_constituency_size,
            team_size=options.cache_team_size, # num sets in a constituency
            block_offset=block_offset, # int 6 bits
            set_offset=set_offset, # set offset 10 bits
            assoc=options.l3_assoc, # int 4 bits
            num_sets=num_sets # number of cache sets 1024
        )
        # cache.replacement_policy = DIPRP()
    elif (cache_repl_policy == "DIPRP"):
        print("initializing DIPRP cache with settings \n constituency:",
              options.cache_constituency_size, ", team_size: ",
              options.cache_team_size)

        if(not(isPowerOfTwo(options.cacheline_size))):
            print("Cache line size should be a power of 2\n")
            sys.exit(1)
        block_offset = int(Log2(options.cacheline_size))
        print("block_offset: ", block_offset)

        l2_cache_size = int(options.l2_size[0:-2])
        print("l2 size: ", l2_cache_size)

        if (options.l2_size[-2:] == "kB"):
            l2_cache_size = l2_cache_size * 1024
        elif (options.l2_size[-2:] == "MB"):
            l2_cache_size = l2_cache_size * 1024 * 1024
        else:
            print("L2 cache size should be either in kB or MB\n")
            sys.exit(1)

        num_cachelines = l2_cache_size / options.cacheline_size
        print("num_cachelines: ", num_cachelines)
        num_sets = num_cachelines / options.l2_assoc
        print("num_cache_sets: ", num_sets)
        set_offset = int(Log2(num_sets))
        print("set_offset: ", set_offset)

        cache.replacement_policy = DIPRP(
            replacement_policy_1=LRURP(),
            replacement_policy_2=BIPRP(
                btp=options.cache_btp
            ),
            constituency_size=options.cache_constituency_size,
            team_size=options.cache_team_size, # num sets in a constituency
            block_offset=block_offset, # int 6 bits
            set_offset=set_offset, # set offset 10 bits
            assoc=options.l2_assoc, # int 4 bits
            num_sets=num_sets # number of cache sets 1024
        )
        # cache.replacement_policy = DIPRP()
    elif (cache_repl_policy == "LFURP"):
        cache.replacement_policy = LRURP()
    elif (cache_repl_policy == "FIFORP"):
        cache.replacement_policy = FIFORP()
    elif (cache_repl_policy == "MRURP"):
        cache.replacement_policy = MRURP()
    elif (cache_repl_policy == "RandomRP"):
        cache.replacement_policy = RandomRP()
    else:
        print("Other cache replacement policies "
            "are not supported for execution "
            "input one of LRURP, LIPRP, BIPRP, DIPRP, "
            "LFURP, FIFORP, MRURP, or RandomRP")
        sys.exit(1)

def config_cache(options, system):
    if options.external_memory_system and (options.caches or options.l2cache):
        print("External caches and internal caches are exclusive options.\n")
        sys.exit(1)

    if options.external_memory_system:
        ExternalCache = ExternalCacheFactory(options.external_memory_system)

    if options.cpu_type == "O3_ARM_v7a_3":
        try:
            import cores.arm.O3_ARM_v7a as core
        except:
            print("O3_ARM_v7a_3 is unavailable. Did you compile the O3 model?")
            sys.exit(1)

        dcache_class, icache_class, l2_cache_class, \
            l3_cache_class, walk_cache_class = \
            core.O3_ARM_v7a_DCache, core.O3_ARM_v7a_ICache, \
            core.O3_ARM_v7aL2, \
            L3Cache, \
            core.O3_ARM_v7aWalkCache
    elif options.cpu_type == "HPI":
        try:
            import cores.arm.HPI as core
        except:
            print("HPI is unavailable.")
            sys.exit(1)

        dcache_class, icache_class, l2_cache_class, walk_cache_class = \
            core.HPI_DCache, core.HPI_ICache, core.HPI_L2, core.HPI_WalkCache
    else:
        dcache_class, icache_class, l2_cache_class, \
        l3_cache_class, walk_cache_class = \
            L1_DCache, L1_ICache, L2Cache, L3Cache, None

        if buildEnv['TARGET_ISA'] in ['x86', 'riscv']:
            walk_cache_class = PageTableWalkerCache

    # Set the cache line size of the system
    system.cache_line_size = options.cacheline_size

    # If elastic trace generation is enabled, make sure the memory system is
    # minimal so that compute delays do not include memory access latencies.
    # Configure the compulsory L1 caches for the O3CPU, do not configure
    # any more caches.
    if options.l2cache and options.elastic_trace_en:
        fatal("When elastic trace is enabled, do not configure L2 caches.")

    if options.l2cache and options.l3cache:
        system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)
        set_cache_repl_policy(options, "LRURP", system.l2)

        system.l3 = l3_cache_class(clk_domain=system.cpu_clk_domain,
                                    size=options.l3_size,
                                    assoc=options.l3_assoc)
        set_cache_repl_policy(options, options.cache_repl, system.l3)

        system.tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.tol3bus = L3XBar(clk_domain = system.cpu_clk_domain)
        system.l2.cpu_side = system.tol2bus.master
        system.l2.mem_side = system.tol3bus.slave
        system.l3.cpu_side = system.tol3bus.master
        system.l3.mem_side = system.membus.slave

        if options.l2_hwp_type:
            hwpClass = ObjectList.hwp_list.get(options.l2_hwp_type)
            if system.l2.prefetcher != "Null":
                print("Warning: l2-hwp-type is set (", hwpClass, "), but",
                      "the current l2 has a default Hardware Prefetcher",
                      "of type", type(system.l2.prefetcher), ", using the",
                      "specified by the flag option.")
            system.l2.prefetcher = hwpClass()


    elif options.l2cache:
        # Provide a clock for the L2 and the L1-to-L2 bus here as they
        # are not connected using addTwoLevelCacheHierarchy. Use the
        # same clock as the CPUs.
        system.l2 = l2_cache_class(clk_domain=system.cpu_clk_domain,
                                   size=options.l2_size,
                                   assoc=options.l2_assoc)
        set_cache_repl_policy(options, options.cache_repl, system.l2)

        system.tol2bus = L2XBar(clk_domain = system.cpu_clk_domain)
        system.l2.cpu_side = system.tol2bus.master
        system.l2.mem_side = system.membus.slave
        if options.l2_hwp_type:
            hwpClass = ObjectList.hwp_list.get(options.l2_hwp_type)
            if system.l2.prefetcher != "Null":
                print("Warning: l2-hwp-type is set (", hwpClass, "), but",
                      "the current l2 has a default Hardware Prefetcher",
                      "of type", type(system.l2.prefetcher), ", using the",
                      "specified by the flag option.")
            system.l2.prefetcher = hwpClass()

    if options.memchecker:
        system.memchecker = MemChecker()

    for i in range(options.num_cpus):
        if options.caches:
            icache = icache_class(size=options.l1i_size,
                                  assoc=options.l1i_assoc)
            set_cache_repl_policy(options, "LRURP", icache)
            dcache = dcache_class(size=options.l1d_size,
                                  assoc=options.l1d_assoc)
            set_cache_repl_policy(options, "LRURP",dcache)

            # If we have a walker cache specified, instantiate two
            # instances here
            if walk_cache_class:
                iwalkcache = walk_cache_class()
                dwalkcache = walk_cache_class()
            else:
                iwalkcache = None
                dwalkcache = None

            if options.memchecker:
                dcache_mon = MemCheckerMonitor(warn_only=True)
                dcache_real = dcache

                # Do not pass the memchecker into the constructor of
                # MemCheckerMonitor, as it would create a copy; we require
                # exactly one MemChecker instance.
                dcache_mon.memchecker = system.memchecker

                # Connect monitor
                dcache_mon.mem_side = dcache.cpu_side

                # Let CPU connect to monitors
                dcache = dcache_mon

            if options.l1d_hwp_type:
                hwpClass = ObjectList.hwp_list.get(options.l1d_hwp_type)
                if dcache.prefetcher != m5.params.NULL:
                    print("Warning: l1d-hwp-type is set (", hwpClass, "), but",
                          "the current l1d has a default Hardware Prefetcher",
                          "of type", type(dcache.prefetcher), ", using the",
                          "specified by the flag option.")
                dcache.prefetcher = hwpClass()

            if options.l1i_hwp_type:
                hwpClass = ObjectList.hwp_list.get(options.l1i_hwp_type)
                if icache.prefetcher != m5.params.NULL:
                    print("Warning: l1i-hwp-type is set (", hwpClass, "), but",
                          "the current l1i has a default Hardware Prefetcher",
                          "of type", type(icache.prefetcher), ", using the",
                          "specified by the flag option.")
                icache.prefetcher = hwpClass()

            # When connecting the caches, the clock is also inherited
            # from the CPU in question
            system.cpu[i].addPrivateSplitL1Caches(icache, dcache,
                                                  iwalkcache, dwalkcache)

            if options.memchecker:
                # The mem_side ports of the caches haven't been connected yet.
                # Make sure connectAllPorts connects the right objects.
                system.cpu[i].dcache = dcache_real
                system.cpu[i].dcache_mon = dcache_mon

        elif options.external_memory_system:
            # These port names are presented to whatever 'external' system
            # gem5 is connecting to.  Its configuration will likely depend
            # on these names.  For simplicity, we would advise configuring
            # it to use this naming scheme; if this isn't possible, change
            # the names below.
            if buildEnv['TARGET_ISA'] in ['x86', 'arm', 'riscv']:
                system.cpu[i].addPrivateSplitL1Caches(
                        ExternalCache("cpu%d.icache" % i),
                        ExternalCache("cpu%d.dcache" % i),
                        ExternalCache("cpu%d.itb_walker_cache" % i),
                        ExternalCache("cpu%d.dtb_walker_cache" % i))
            else:
                system.cpu[i].addPrivateSplitL1Caches(
                        ExternalCache("cpu%d.icache" % i),
                        ExternalCache("cpu%d.dcache" % i))

        system.cpu[i].createInterruptController()
        if options.l2cache:
            system.cpu[i].connectAllPorts(system.tol2bus, system.membus)
        elif options.external_memory_system:
            system.cpu[i].connectUncachedPorts(system.membus)
        else:
            system.cpu[i].connectAllPorts(system.membus)

    return system

# ExternalSlave provides a "port", but when that port connects to a cache,
# the connecting CPU SimObject wants to refer to its "cpu_side".
# The 'ExternalCache' class provides this adaptation by rewriting the name,
# eliminating distracting changes elsewhere in the config code.
class ExternalCache(ExternalSlave):
    def __getattr__(cls, attr):
        if (attr == "cpu_side"):
            attr = "port"
        return super(ExternalSlave, cls).__getattr__(attr)

    def __setattr__(cls, attr, value):
        if (attr == "cpu_side"):
            attr = "port"
        return super(ExternalSlave, cls).__setattr__(attr, value)

def ExternalCacheFactory(port_type):
    def make(name):
        return ExternalCache(port_data=name, port_type=port_type,
                             addr_ranges=[AllMemory])
    return make
