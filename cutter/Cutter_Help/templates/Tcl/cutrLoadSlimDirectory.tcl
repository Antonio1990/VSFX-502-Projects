# Directs SLIM to read all the template files from a directory
# Author: Ryan Heniser 
# Email: heniser@yahoo.com
# Description: Loads slim palettes in MTOR from
#     H:/slim
# If you keep your SLIM template files in another directory be
# sure to edit the first line of this script.
#
# To use this script:
#	1. Ensure the MTOR plugin has been loaded into Maya
#	2. Open the SLIM console
#	3. Use the File->Run Script... pull down menu

set d M://slim//
set filelist [glob -nocomplain -types f -directory $d *.slim]
set num_elements [llength $filelist]
::RAT::LogMsg INFO "Loading slim templates in $d directory..."
while {$num_elements > 0} {
	::RAT::LogMsg INFO [lindex $filelist [expr $num_elements-1]]
	slim ReadSlimFile [lindex $filelist [expr $num_elements-1]]
        set num_elements [expr $num_elements-1]
}
