# Auto loads slim files from users "H:/slim" directory
# Insert into line 48 of the "slim.ini" file.
# The ini file can be found at
# C:/Program Files/Pixar/rat-6.0.1/etc/slim.ini
#
set userdir M:/slim
if { [file exists $userdir] } {
	set slimfiles [glob -nocomplain -directory $userdir *slim]
	foreach item $slimfiles {
		set filename [file tail $item]
		LoadExtension slim [file join $userdir $filename] template
		}
	}
