# A procedure to add an extension, for example "tif", to a number of files that do not have an extension.
#
# For each file in the directory specified by "path", this procedure identifies
# files that do not have an extension. Such files are renamed with the specified
# extension.

proc addExtension { path ext } {
	set contents [glob -nocomplain -directory $path *]
	foreach item $contents {
		# un-comment the next three lines to make this procedure recursive		
		#if { [file isdirectory $item] } {
		#	addExtension $item $ext
		#	} 
		
		if { [file isfile $item] } {
			# NO EXTENSION...
			if { [file extension $item] != $ext } {
				file rename -force $item  $item$ext
				
				# report what has happened
				puts "[file tail $item] renamed to [file tail $item$ext]"
				}
			}
		}
	}
# An example of how to run this script
#	proc name      directory name  extension name
#	addExtension   H:/frames    	".tif"
