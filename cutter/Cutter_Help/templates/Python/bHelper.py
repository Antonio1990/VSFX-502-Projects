#!/usr/bin/env python

# See Templates->Python->aHelper.py for a more efficient way
# of generating geometry for prman.
# Malcolm Kesson: March 4th 2017

import sys

args = sys.stdin.readline()

if args:
	arg = args.split()
	pixels = float(arg[0])
	rad = float(arg[1])
	print 'TransformBegin'
	print 'Sphere %s %s %s 360' % (rad, -rad, rad)
	print 'TransformEnd'

	sys.stdout.write('\377')
	sys.stdout.flush()
	
	# wait for the next set of inputs
	args = sys.stdin.readline()
