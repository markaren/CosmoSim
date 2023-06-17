#!/bin/sh

pyd=../../Python

python3 $pyd/datagen.py --csvfile debug.csv --outfile roulette.csv --centred -D Original &&\
python3 $pyd/roulettegen.py --csvfile roulette.csv --model Roulette --centred -D Roulette --lens Roulette
