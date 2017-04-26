#!/usr/bin/env python

# Many thanks to Julian Fong,
#    http://graphics.pixar.com/library/indexAuthorFong.html 
# for supplying the code for this example helper app. Information about "PRMan
# for Python" can be found here,
#    https://renderman.pixar.com/resources/RenderMan_20/prmanForPython.html
# Specific information about the names of constants can be found here,
#    PATH/TO/RenderManProServer-21.X/include/ri.h
#
# Malcolm Kesson: March 4th 2017

import prman
import sys
import string
ri = prman.Ri()

while True:
	try:
		line = raw_input().strip()
	except EOFError:
		break
	else:
		detail, data = line.split(' ', 1);
		inputs = data.split()
		rad = float(inputs[0])
		#sys.stderr.write('num inputs = %d' % len(data.split() ) )
		ri.Begin("-")
		# Generate some geometry...
		ri.Sphere(rad, -rad, rad, 360)
		
		# The "/377" escape sequence tells prman we have finished.
		ri.ArchiveRecord(ri.COMMENT, "\n\377")
		sys.stdout.flush()
		ri.End()