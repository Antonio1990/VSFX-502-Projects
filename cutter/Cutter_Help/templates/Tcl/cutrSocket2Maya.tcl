# An example of using a SOCKET to send a MEL script to Maya.
# The script assumes the mel command,
#    commandPort -n ":2222"
# has been set in Maya.

#========================================================
# Opens a socket given IP address and a port number.
# For example,
#  set mysocket [openSocket 10.7.120.204 2222]
# Returns a socket identifer
proc openSocket { host port } {
set sock [socket $host $port]
#fconfigure $sock -buffering line
return $sock
}
#========================================================

#========================================================
# Sends text to the open socket
proc sendMelTo { socket_ID text } {
puts $socket_ID $text
flush $socket_ID
puts $socket_ID ""
}
#========================================================


# Step 1	Open the socket to an active Maya session
set targetIP localhost
set maya [openSocket $targetIP 2222]

# Step 2	Either make a MEL script - this script clears the scene, then creates a cylinder
set mel "select -all;\n"
append mel "delete;\n"
append mel "cylinder\n"

# Step 3	OR, read an existing script
set fileID [open "H:/mel/simple.mel"]
set text [read $fileID]

# Step 4	Send either the $mel text or the text
#			from the file. Alternatively, the MEL
#			source command could be used
set choice a
switch $choice {
	a	{ sendMelTo $maya $mel }
	b	{ sendMelTo $maya $text }
	c	{ sendMelTo $maya "source H:/mel/simple.mel" }
	}
