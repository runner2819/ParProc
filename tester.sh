#!/bin/bash

#filename="test_qoi2png"
#./cmake-build-release/qoi $filename"_1.qoi" $filename"_1.png"
#./cmake-build-release/qoi $filename"_1.png" $filename"_2.qoi"
#./cmake-build-release/qoi $filename"_2.qoi" $filename"_2.png"

filename=$2
$1 $filename".png" $filename"_1.qoi" $3
$1 $filename"_1.qoi" $filename"_1.png" $3
$1 $filename"_1.png" $filename"_2.qoi" $3
$1 $filename"_2.qoi" $filename"_2.png" $3

diff $filename"_1 copy.png" $filename"_1.png" -s
diff $filename"_1.png" $filename"_2.png" -s
diff $filename"_1.qoi" $filename"_2.qoi" -s
#rm -rf *_1.qoi *_1.png *_2.qoi *_2.png
