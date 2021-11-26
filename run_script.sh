#!/bin/bash

declare -a l2size=("1MB")
declare -a repl=("BIPRP" "DIPRP" "LIPRP" "MRURP" "LRURP")
declare -a benchmark=("mcf" "bzip2" "lbm" "namd" "gobmk" "sjeng")

for bench in "${benchmark[@]}"; do
	for policy in "${repl[@]}"; do
		for size in "${l2size[@]}"; do

            SECONDS=0
            echo " "
            printf "start timestamp: %s\n" "$(date)"
            echo "running sim for benchmark:" $bench ", size:" $size ", cache replacement policy:" $policy "..."
            echo "------------------------------------------------------------------------"

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
            --standard-switch=50000000 \
            &> a_verify_dip/$bench/$size/$pol/out.txt

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
            # --standard-switch=50000 \
            # &> a_verify_dip/$bench/$size/$pol/out.txt

            duration=$SECONDS
            echo " "
            echo "sim completed for benchmark:" $bench ", size:" $size ", cache replacement policy:" $policy
            printf "end timestamp: %s\n" "$(date)"
            echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
            echo "------------------------------------------------------------------------------------------------------------"
            echo "------------------------------------------------------------------------------------------------------------"

		done
	done
done

wait