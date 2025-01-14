#! /bin/sh

pdir=../../Python
dir1=SphereModel
dir2=RouletteSIS
diffdir=diff

mkdir -p $dir1 $dir2 $diffdir

fn=../spheres.csv

python3 $pdir/datagen.py -L s --directory="$dir1" --csvfile $fn 
python3 $pdir/datagen.py -L sr --directory="$dir2" --csvfile $fn 
python3 $pdir/compare.py --diff $diffdir $dir1 $dir2

