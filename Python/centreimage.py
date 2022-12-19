#! /usr/bin/env python3

"""
Take an image and return an image with only the 
largest connected component.

This is depecrated as the the new makeimage.py script includes
the post-processing.
"""

import cv2 as cv
import sys
import numpy as np
import argparse


def cleanImage(im,sensitivity=None):
   """
   Do connected components analysis and remove all but the largest component.
   """
   print( f"Image type: {im.dtype}; "
       + f"Range ({im.min()},{im.max()})" )
   im = im.astype(np.uint8)

   if sensitivity:
       im2 = im.copy()
       im2[(im2<=sensitivity)] = 0
   else:
       im2 = im

   retval, labels, stats, centroids = \
     cv.connectedComponentsWithStats( im2, connectivity=8, ltype=cv.CV_32S )


   print( "Labels Shape: ", labels.shape )
   print( "Labels Map: ", labels )
   print( "Labels Range: ", labels.min(), labels.max() )
   print( "Stats: ", stats )

   components = [ x for x in enumerate( stats[:,cv.CC_STAT_AREA] ) ]
   print( "Components: ", components )

   components.sort(reverse=True, key=lambda x : x[1] )
   print( "Components: ", components )

   label = components[1][0]
   mask = ( labels == label ).astype(np.uint8)

   return cv.bitwise_and( im, im, mask=mask )

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
          prog = 'CosmoSim centreImage',
          description = 'Centre an image on the centre of light',
          epilog = '')

    parser.add_argument('fn',nargs="*", help="Input file") 
    parser.add_argument('-o','--outfile',help="Output file") 
    parser.add_argument('-R', '--reflines',action='store_true',
            help="Add reference (axes) lines")
    parser.add_argument('-A', '--artifacts',action='store_true',
            help="Attempt to remove artifacts from the Roulettes model")
    parser.add_argument('-s', '--sensitivity',
            help="Sensitivity for the connected components (only with -A)")
    args = parser.parse_args()

    fns = args.fn
    if not fns: fns = ["image-test.png"]
    for fn in fns:
       print( "Filename: ", fn )
       raw = cv.imread(fn)
       print("Raw image Shape: ", raw.shape)

       im = cv.cvtColor(raw, cv.COLOR_BGR2GRAY)
       print("Shape converted to grey scale: ", im.shape)
       print("Image type: ", im.dtype)
       centred = centreImage(im)
       if args.artifacts:
           if args.sensitivity: sensitivity = int(args.sensitivity)
           else: sensitivity = None
           centred = cleanImage(centred,sensitivity)
       if args.reflines:
           centred = drawAxes(centred)
   
       if args.outfile == None:
           outfile = "centred-" + fn
       else:
           outfile = args.outfile 
       cv.imwrite( outfile, centred )