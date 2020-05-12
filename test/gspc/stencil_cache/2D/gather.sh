#!/bin/bash

set -euo pipefail

for ((R=2;R<=3;R+=1))
do
    for ((X=50;X<=200;X+=50))
    do
        for ((Y=50;Y<=200;Y+=50))
        do
            H=$((2*(R-1)+1))
            S=$((4*R*(R-1)+1))

            ( for ((M=S;M<=$((Y*H+S));M+=S))
              do
                  ${1:?"GATHER_BIN"} -- --X $X --Y $Y --R $R --M $M --C 5 --L 5
              done
            ) | grep number | tee gather.$R.$X.$Y.out
          done
    done
done
