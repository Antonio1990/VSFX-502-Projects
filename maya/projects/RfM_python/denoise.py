#!/usr/bin/env python

# For Use with Linux ONLY
# 1  Copy this script to the images folder and then run it.
#    It will create a file called "denoise_all" (Linux) or
#    "denoise_all.bat" (Windows).
# 2  Execute the "denoise_all" script and it will open a
#    terminal that will show the progress of the denoising.
# Malcolm Kesson
# Nov 20 2016

import os, glob, sys

rootpath = os.path.dirname(__file__)

exr_pattern = os.path.join(rootpath, "*.exr");
exr_files = glob.glob(exr_pattern)
exr_files.sort()

# Don't assume all the exr files have the same filename
namesDict = {}
hasImageNamed_variance = False
nameOfVarianeImage = ''
for exr in exr_files:
	full_name =  os.path.basename(exr)
	parts = full_name.split('.')
	image_name = ''
	for n in range( len(parts) - 2):
		image_name += parts[n] + '.'
	image_name = image_name.rstrip('.')
	# exclude any previously filtered images
	if image_name.endswith('_filtered') == False:
		namesDict[image_name] = [] # will contain a list of numeric extensions
	if image_name.endswith('_variance'):
		hasImageNamed_variance = True
		nameOfVarianeImage = image_name
		
# The dictionary of exr names should have at least one entry
# Get a list of the numeric extensions for each exr file. We assume the numeric
# extension follows this form "name.0001.exr".
for key in namesDict.keys():
	exr_pattern = os.path.join(rootpath, key + '.*.exr');
	exr_files = glob.glob(exr_pattern)
	for exr in exr_files:
		parts = exr.split('.')
		if len(parts) > 2:
			num_ext = parts[len(parts) - 2]
			namesDict[key].append(num_ext)

binpath = os.path.join(os.getenv('RMANTREE'), 'bin', 'denoise')
command = '"%s" --crossframe -v variance -f default.filter.json ' % binpath

# Create an output script that will perform the denoising
if os.name == "nt":
	path = os.path.join(rootpath, 'denoise_all.bat')
else:
	path = os.path.join(rootpath, 'denoise_all')
f = open(path , 'w')

# The easiest situations to handle are cases 1 and 2 shown below. If there are no image files
# with "_variance" we assume the ones with the shortest names are the main passes. 
main_pass = ''
if hasImageNamed_variance == False:
	main_pass = min(namesDict.keys(), key=len)
else:
	main_pass = nameOfVarianeImage
	
numStr = ",".join(namesDict[main_pass])
f.write('%s %s.{%s}.exr\n' % (command,main_pass,numStr))
namesDict.pop(main_pass)

if hasImageNamed_variance:
	# Just a repetition of the first command
	f.write('%s %s.{%s}.exr' % (command,main_pass,numStr.rstrip(',')))
	for key in namesDict.keys():
		numStr = ",".join(namesDict[key])
		f.write(' %s.{%s}.exr' % (key,numStr))
f.close()

# Case 1:
# If no AOVs have been assigned the images directory will only contain a single set of files,
#		denoise.005.0001.exr
#		denoise.005.0002.exr
#		etc...
# Consequently, filtering is done as follows,
#     denoise  --crossframe -v variance -f default.filter.json denoise.005.{0001,0002,0003,0004,0005}.exr"
# ------------------------------------------------------------------------------------------
# Case 2:
# If one or more AOVs have been assigned but they belong to single .exr files, for example,
#		denoise.004.0001.exr
#		denoise.004.0002.exr
#		denoise.004_lpe_diffuse,lpe_specular.0001.exr
#		denoise.004_lpe_diffuse,lpe_specular.0002.exr
#		etc..
# Consequently, filtering is done as follows,
# 	denoise  --crossframe -v variance -f default.filter.json denoise.004.{0001,0002,0003,0004,0005}.exr"
# ------------------------------------------------------------------------------------------
# Case 3:
# If one or more AOVs have been assigned but they belong to multiple .exr files, for example,
#		denoise.003_variance.0001.exr
#		denoise.003_variance.0002.exr
#		denoise.003_indirectdiffuse0.0001.exr
#		denoise.003_indirectdiffuse0.0002.exr
#		denoise.003_lpe_diffuse,lpe_specular,curvature.0001.exr
#		denoise.003_lpe_diffuse,lpe_specular,curvature.0002.exr
# Consequently, filtering is done as follows,
# 	System "denoise  --crossframe -v variance -f default.filter.json   denoise.003_variance.{0001,0002,0003,0004,0005}.exr"
#   System "denoise  --crossframe -v variance -f default.filter.json   denoise.003_variance.{0001,0002,0003,0004,0005}.exr denoise.003_indirectdiffuse0.{0001,0002,0003,0004,0005}.exr"
# ------------------------------------------------------------------------------------------




