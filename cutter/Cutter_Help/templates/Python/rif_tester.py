# handy script for testing prman rif filters
import prman, rif_examples
  
ribin = 'PATH_TO_INPUT_RIB_FILE'
ribout = 'PATH_TO_OUTPUT_RIB_FILE'


ri = prman.Ri() # Access prman's RiXXX procs and definitions
ri.Option("rib", {"string asciistyle": "indented"})  # Format the output for easier reading
 
rif = rif_examples.Rif(ri)    # Get an instance of our Rif
prman.RifInit([rif])          # Tell prman about our Rif
  
ri.Begin('-')                 # Echo the rib stream as it is rendered
#ri.Begin(ribout)             # Divert the rib stream to a file
prman.ParseFile(ribin)        # Process the input rib
ri.End()                      # Tell prman we're done!
