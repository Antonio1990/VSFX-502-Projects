#!/usr/bin/env python

# 1  Copy this script to the images folder and then run it.
#    It will create a file called,
#		run_denoise.bat (Windows)
#    or
#		run_denoise (OSX and Linux) 
# 2  Double click the run_denoise script and it will open a
#    terminal that will show the progress of the denoising.
# Malcolm Kesson
# March 13 2017

import os
import sys
import glob
import re

command = 'denoise  --crossframe -v variance -f default.filter.json '
rootpath = os.path.dirname(__file__)
glob_pat = os.path.join(rootpath, "*.exr");
re_pat = re.compile(r'(\w+)[._]+(\d+)[._]+exr')
		
image_paths = glob.glob(glob_pat)
if len(image_paths) > 0:
	image_paths.sort()
	proj_dir_name = os.path.dirname(image_paths[0])
	image_name = os.path.basename(image_paths[0])
	
	offset = image_name.find('.')
	if offset > 0:
		numeric_extensions = ''
		image_name = image_name[0:offset]
		run_denoise_path = os.path.join(rootpath, 'run_denoise')
		if os.name == "nt":
			run_denoise_path += '.bat'
		f = open(run_denoise_path , 'w')
		f.write('%s %s.' % (command, os.path.join(proj_dir_name, image_name)))
		for path in image_paths:
			image_name = os.path.basename(path)
			if image_name.find('_filtered') == -1:
				match = re.search(re_pat, image_name)
				if match:
					num_ext = match.group(2)
					numeric_extensions += num_ext + ','

		if len(numeric_extensions) > 1:
			numeric_extensions = numeric_extensions[:-1]
			f.write('{%s}.exr ' % (numeric_extensions))
		f.close()
