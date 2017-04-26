/* Shader description goes here */
surface
constant_test(float	Kfb = 1	/* fake brightness */)
{
color	surfcolor = 1;

/* STEP 1 - set the apparent surface opacity */
Oi = Os;

/* STEP 2 - calculate the apparent surface color */
Ci = Oi * Cs * surfcolor * Kfb;
}
