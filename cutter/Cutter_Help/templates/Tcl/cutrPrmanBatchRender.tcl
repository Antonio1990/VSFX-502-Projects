# Generates a batch rendering script for use with Pixar's prman.
# This TCL script can be run in one of two ways.
#	1. 	Put the script in the same directory as the RIB files to be rendered,
#		then run the script from a shell ie.
#			%tclsh batch_render.tcl
#	OR
#
#	2.	Run the script from any directory, but specify the full path
#		to the folder containing the RIB files to be rendered ie.
#			%tclsh batch_render.tcl H:/ribs
# This version of the script tries to ensure the shadow pass ribs are
# rendered before the beauty pass ribs. However, this assumes the ribs
# have been generated using Maya and mtor so it may not work in other
# situations. 
# M. A. Kesson (April 16 2005)
# revised November 8 2006

#____________________________________________________
# Local procedure
# Input: full path to a directory
# Returns: list of rib files
#____________________________________________________
proc getRibs { dir } {
set contents [glob -nocomplain -directory $dir *rib]
set out ""
foreach item $contents {
    lappend out $item
    }
return $out
}

#____________________________________________________
# Main procedure
#____________________________________________________
proc batchRender { pathToRibs } {

set batch_name "batch.bat"
set render_command "prman"

set batch_path $pathToRibs/$batch_name
puts $batch_path

# Create a batch file for writing, hence, 'w'
set fd [open $batch_path w]

# Get a list of the ribs
set ribs [getRibs $pathToRibs]

# Process the shadow pass ribs...
foreach item $ribs {
	if { [regexp ".shd|dsh.\[0-9\]*" $item] == 1 } {
    	puts $fd "$render_command $item"
		}
	}
# Then the beauty pass ribs... 
foreach item $ribs {
	if { [regexp ".shd|dsh.\[0-9\]*" $item] == 0} {
    	puts $fd "$render_command $item"
		}
	}
	
# Close the batch file
close $fd

set os [array get tcl_platform platform]
if {$os == "unix"} {
	exec "chmod" "777" $batch_path
	}

# Echo the contents of the batch file to the terminal
puts [read [open $batch_path r]]

# Uncomment the next line to execute the batch file
#exec $batch_path
}

if {$argc == 1} {
	set path [lindex $argv 0]
	if {[file exists $path] == 0} {
		puts "The input directory does not exist!"
		puts "Cannot make a batch render file."
	} else { 
		batchRender $path
	}
} else {
	# Use the ribs in the present working directory
	batchRender [pwd]
	
	# Or provide a specific directory...
	#batchRender PATH_TO_THE_RIBS_DIRECTORY
	}





