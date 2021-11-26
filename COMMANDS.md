```bash
# build the gem5
scons-3 -j 4 ./build/ARM/gem5.opt
scons-3 -j 4 ./build/X86/gem5.opt


/usr/bin/env python3 $(which scons) build/X86/gem5.opt
/usr/bin/env python3 $(which scons) -j 24 build/ARM/gem5.opt

```