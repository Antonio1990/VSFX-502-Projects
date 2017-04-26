# Template TCL code for a RenderMan Helper application.
# For more details see 
#    http://www.fundza.com/rman_helper/tcl/index.html
# See the template RIB file that can use this helper script. It is in the
# RIB templates menu - "cutrTclHelperTester.rib"
# Malcolm Kesson Oct 2006

fconfigure stdout -translation binary
  
while { [gets stdin buffer] != -1 } {
	set pixels [lindex $buffer 0]
	set rad [lindex $buffer 1]
	
	# uncomment to show the number of pixels
	# puts stderr "pixel coverage is $pixels"
  
	puts "TransformBegin"
		puts "Sphere $rad -$rad $rad 360"
	puts "TransformEnd"
	
	puts "\377" 
	flush stdout
	}