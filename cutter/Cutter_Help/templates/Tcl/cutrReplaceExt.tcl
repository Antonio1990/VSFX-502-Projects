# A procedure to replace one extension for another, for example ".tiff" to ".tif"
#
# For each file in the directory specified by "path", this procedure identifies
# files that end with "oldExt". Such files are renamed so that they end with "newExt"

proc replaceExtension { path oldExt newExt} {
	set contents [glob -nocomplain -directory $path *]
	foreach item $contents {

		if { [file isfile $item] } {
			if { [file extension $item] == $oldExt } {
				# get the path to the file without its extension
				regsub -all $oldExt\$ $item "" temp
				
				file rename -force $item  $temp$newExt

				# report what has happened
				puts "[file tail $item] renamed to [file tail $temp$newExt]"
				}
			}
		}
	}

# An example of how to run this script
#	proc name      directory name  current extension 	new extension
#replaceExtension   H:/frames      ".tiff" 				 ".tif"
