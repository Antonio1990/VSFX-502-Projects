# An example of an "it" (Image Tool) script written in the TCL 
# (Tool Command Language). The script must be executed from the 
# "it" console. To run the script.
# 1. Make sure that "it" has been launched.
# 2. Right mouse click in either the "catalog" or "it" window.
# 3. Choose the Windows->Console... menu item.
# 4. In the console enter the name of this proc
#        % anaglyph_plain
#
# IMPORTANT: An environment variable called "RMS_SCRIPT_PATHS" 
# must point to the DIRECTORY in which this script is saved.
# To set the variable add this line of text to your Maya.env
# file,
#		RMS_SCRIPT_PATHS=PATH/TO/DIRECTORY
#
# Many thanks to Ian Hsieh and James Burgess of Pixar for HELP
# in preparing this script.
#
# The matrices are from Peter Wimmer's great web page,
#	http://3dtv.at/Knowhow/AnaglyphComparison_en.aspx
# 
# M.Kesson Dec 2009
#
proc anaglyph_plain {} {
[ice::load camera_left.tif  -h left_raw]
[ice::load camera_right.tif -h right_raw]

it IceExpr "left_rgb := left_raw Shuffle(list(0,1,2))"
it IceExpr "right_rgb := right_raw Shuffle(list(0,1,2))"

# Create two cards
it IceExpr "leftcard  := IceImage Card(IceComponentType Float, 0.2, .7, 0.7)"
it IceExpr "rightcard := IceImage Card(IceComponentType Float, 1.0, 0.1, 0.1)"

# Multiply images with the cards
it IceExpr "left_tinted  := left_raw Multiply(leftcard)"
it IceExpr "right_tinted := right_raw Multiply(rightcard)"

# Add images together
it IceExpr "stereo := left_tinted Add(right_tinted)"
}
