#!/bin/bash

declare -a l2size=("1MB")
declare -a repl=("BIPRP" "DIPRP" "LIPRP" "MRURP" "LRURP")
declare -a benchmark=("mcf" "bzip2" "lbm" "namd" "gobmk" "sjeng" "namd")

for bench in "${benchmark[@]}"; do
	for policy in "${repl[@]}"; do
		for size in "${l2size[@]}"; do
			# ./build/X86/gem5.opt -d verifySHIP/cacheSensitivity/$bench/$pol/$size \
            # configs/spec2k6/run.py -b $bench --maxinsts=250000000 --cpu-type=DerivO3CPU \
            # --caches --l2cache --l3cache \
            # --fast-forward=1000000000 --warmup-insts=50000000 --standard-switch=50000000 \
            # --caches --l2cache --l3cache --l3_repl=$pol --l3_size=$size &

            echo " "
            printf "start timestamp: %s\n" "$(date)"
            echo "running sim for benchmark: " $bench " , size: " $size ", cache replacement policy: " $policy " ..."
            echo " "

            # real simulation
            
            ./build/ARM/gem5.opt -d a_verify_dip/$bench/$size/$pol \
            configs/spec2k6/run.py \
            --benchmark=$bench \
            --cpu-type=O3_ARM_v7a_3 \
            --caches --l2cache \
            --num-l2caches=1 \
            --l1d_size='32kB' \
            --l1i_size='32kB' \
            --l2_size=$size \
            --l1d_assoc=2 \
            --l1i_assoc=2 \
            --l2_assoc=16 \
            --cacheline_size=64 \
            --cache_repl=$policy \
            --cache_team_size=16 \
            --cache_constituency_size=32 \
            --maxinsts=250000000 \
            --fast-forward=1000000000 --warmup-insts=50000000 \
            --standard-switch=50000000 > a_verify_dip/$bench/$size/$pol/out.txt

            # small number of instructions to check 

            # ./build/ARM/gem5.opt -d a_verify_dip/$bench/$size/$pol \
            # configs/spec2k6/run.py \
            # --benchmark=$bench \
            # --cpu-type=O3_ARM_v7a_3 \
            # --caches --l2cache \
            # --num-l2caches=1 \
            # --l1d_size='32kB' \
            # --l1i_size='32kB' \
            # --l2_size=$size \
            # --l1d_assoc=2 \
            # --l1i_assoc=2 \
            # --l2_assoc=16 \
            # --cacheline_size=64 \
            # --cache_repl=$policy \
            # --cache_team_size=16 \
            # --cache_constituency_size=32 \
            # --maxinsts=25000 \
            # --fast-forward=10000 --warmup-insts=50000 \
            # --standard-switch=50000 > a_verify_dip/$bench/$size/$pol/out.txt

            echo " "
            echo "sim completed for benchmark: " $bench " , size: " $size ", cache replacement policy: " $policy
            printf "end timestamp: %s\n" "$(date)"
            echo " "

		done
	done
done

wait