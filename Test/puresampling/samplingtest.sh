#! /bin/sh

( cd ../.. && cmake --build build ) || exit 1

opt=$*

pdir=../../Python
dir1=PureSampledSIS
dir2=RouletteSIS
diffdir=diff

mkdir -p $dir1 $dir2 $diffdir

fn=spheres.csv

python3 $pdir/datagen.py $opt -Z 800 -L pss --directory="$dir1" --csvfile $fn  || exit 2
python3 $pdir/datagen.py $opt -Z 800 -L sr --directory="$dir2" --csvfile $fn  || exit 3
python3 $pdir/compare.py --diff $diffdir $dir1 $dir2

