#!/bin/sh

set -e

BINARY_PATH=./libparty/baseOT


for N in 128 256 1024; do
    for T in 1 2 4 8; do
        echo "[+] benchmarking N=$N, T=$T"
        # read -s
        ssh encryptosim1 /usr/bin/time -v $BINARY_PATH \
            -r 0 \
            -o out_sender.txt \
            -n $N \
            -a 0.0.0.0 \
            -t $T \
            2> "baseOTs_0_${N}_${T}.txt" &
        PID=$!
        sleep .001s
        ssh encryptosim2 /usr/bin/time -v $BINARY_PATH \
            -r 1 \
            -o out_receiver.txt \
            -a encryptosim1.ddns.net \
            -n $N \
            -t $T \
            2> "baseOTs_1_${N}_${T}.txt"
        wait $PID
        grep wall baseOTs_*_${N}_${T}.txt
        sleep 1s
    done
done
