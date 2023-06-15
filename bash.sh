#!/bin/bash

algorithms=("lru" "2a" "fifo")
filenames=("compilador" "compressor" "matriz" "simulador")
sizes=("2" "4")
buffers=("128" "16384")

rm -rf ./gen
mkdir ./gen

for algorithm in "${algorithms[@]}"
do
  for filename in "${filenames[@]}"
  do
    for size in "${sizes[@]}"
    do
      for buffer in "${buffers[@]}"
      do
        output_file="${algorithm}_${filename}_${size}_${buffer}.txt"
        echo "Running ./main $algorithm $filename.log $size $buffer"
        ./main $algorithm $filename.log $size $buffer > ./gen/$output_file
        echo "Output saved in $output_file"
        echo ""
      done
    done
  done
done
