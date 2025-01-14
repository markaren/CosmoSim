#! /bin/sh

( cd ../.. && cmake --build build ) || exit 1

opt=$*

pdir=../../Python
dir1=SampledSIS
dir2=RouletteSIS
diffdir=diff

mkdir -p $dir1 $dir2 $diffdir

fn=../spheres.csv

python3 $pdir/datagen.py $opt -L ss --directory="$dir1" --csvfile $fn 
python3 $pdir/datagen.py $opt -L sr --directory="$dir2" --csvfile $fn 
python3 $pdir/compare.py --diff $diffdir $dir1 $dir2

