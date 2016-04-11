#!/bin/bash

rm ./samples/*
rm ./haarcascade/* -r

find ./negativeImages -name '*.*' >negatives.dat
find ./positiveImages -name '*.*' >positives.dat

perl createtrainsamples.pl positives.dat negatives.dat samples 400 "opencv_createsamples  -bgcolor 2 -bgthresh 2 -maxxangle 0.2 -maxyangle 0.2 -maxzangle 0.2 -maxidev 2 -w 15 -h 15"
find ./samples -name '*.vec' > samples.dat
./mergevec samples.dat samples.vec
#find ./positiveImages/ -name '*.*' -exec identify -format '%i 1 0 0 %w %h' \{\} \; > samples.dat
#opencv_createsamples -info samples.dat -vec samples.vec -w 30 -h 30
#opencv_createsamples -vec samples.vec  -w 30 -h 30

opencv_haartraining -data haarcascade -vec samples.vec -bg negatives.dat -nstages 10 -nsplits 2 -minhitrate 0.999 -maxfalsealarm 0.3 -npos 400 -nneg 555 -w 15 -h 15 -nonsym -mem 4096 -mode ALL
./convert_cascade --size="15x15" haarcascade haarcascade_right_arrow7.xml


