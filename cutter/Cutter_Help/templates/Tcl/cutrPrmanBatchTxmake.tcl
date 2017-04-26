# Generates a batch script for use with Pixar's txmake.
# M. A. Kesson (Jan 23 2007)

#____________________________________________________
# Local procedure
# Input: full path to a directory
# Returns: list of tif files
#____________________________________________________
proc getTifs { dir } {
set contents [glob -nocomplain -directory $dir *tif]
set out ""
foreach item $contents {
    lappend out $item
    }
return $out
}

#____________________________________________________
# Main procedure
#____________________________________________________
proc batch { pathToTifs } {

set batch_path $pathToTifs/batch.bat

# Create a batch file for writing
set fd [open $batch_path w]

# Get a list of the tifs
set tifs [getTifs $pathToTifs]

# Look for numbered tifs
foreach item $tifs {
	if { [regexp ".\[0-9\]*" $item] == 1} {
		set tifname [file tail $item]
		set texname [regsub ".tif" $tifname ".tex"]
    	puts $fd "txmake -mode black $tifname $texname"
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
}

if {$argc == 1} {
	set path [lindex $argv 0]
	if {[file exists $path] == 0} {
		puts "The input directory does not exist!"
		puts "Cannot make a batch file."
	} else { 
		batch $path
	}
} else {
	# Use the tifs in the present working directory
	batch [pwd]
	
	# Or provide a specific directory...
	#batch PATH_TO_THE_TIFS_DIRECTORY
	}





