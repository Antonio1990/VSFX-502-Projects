/* Shader description goes here */
surface
diffuse_test(float	Kd = 1	/* basic brightness */)
{
color	surfcolor = 1;

/* STEP 1 - make a copy of the surface normal one unit in length */
normal	n = normalize(N);
normal	nf = faceforward(n, I);

/* STEP 2 - set the apparent surface opacity */
Oi = Os;

/* STEP 3 - calculate the diffuse lighting component */
color	diffusecolor = Kd * diffuse(nf);

/* STEP 4 - calculate the apparent surface color */
Ci = Oi * Cs * surfcolor * diffusecolor;
}
