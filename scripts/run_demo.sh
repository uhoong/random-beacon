#!/bin/bash
start=$1
end=$2
rep=($(seq "$start" "$end"))
for i in "${rep[@]}"; do
    echo "starting replica $i"
    #valgrind --leak-check=full ./examples/hotstuff-app --conf hotstuff-sec${i}.conf > log${i} 2>&1 &
    #gdb -ex r -ex bt -ex q --args ./examples/hotstuff-app --conf hotstuff-sec${i}.conf > log${i} 2>&1 &
    ./drg --pvss-ctx ./pvssconf/pvss-sec${i}.conf --idx ${i} > ./log/log${i} 2>&1 &
done
wait
