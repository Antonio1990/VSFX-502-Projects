# Some TCL scripts for use as Slim (appearance) parameter expressions.
# 
# Use of lerp() for keyframing 
# floats.
#-----snip-----snip------snip-----snip------
lerp(0.3, 0.9, $pct)

# colors
#-----snip-----snip------snip-----snip------
lerp(rgb(1,1,0), rgb(0,0,1), $pct)
#-----snip-----snip------snip-----snip------

# This script resolves the name of the object 
# (actually the transform node of the object) to which the
# shader is attached. 
#
#-----snip-----snip------snip-----snip------
[mattr [lindex [split $OBJPATH |] end-1].MY_ATTRIBUTE $F]
#-----snip-----snip------snip-----snip------

# The following script shows the "true" value of "MY_ATTRIBUTE"
# scaled by "MY_SCALING_FACTOR". 
#
#-----snip-----snip------snip-----snip------
[expr [mattr [lindex [split $OBJPATH |] end-1].MY_ATTRIBUTE $F] * MY_SCALING_FACTOR]
#-----snip-----snip------snip-----snip------

# This script can cycle through a sequence of numbered
# files.
#
# PATH is the full path to the numbered files.
# SEQUENCE_COUNT is the number of files.
# EXT is the file extension of the files
#-----snip-----snip------snip-----snip------
PATH.[format %0.4d [expr (($F - 1) % SEQUENCE_COUNT) + 1]].EXT
#-----snip-----snip------snip-----snip------

# This script accesses the translation values of a locator.
# It can be used on a "point" parameter of a shader or node.
[
set x [mattr "locator1.tx" $f]
set y [mattr "locator1.ty" $f]
set z [mattr "locator1.tz" $f]
return "$x $y $z"
]
