/* Shader description goes here */
displacement
displace_test(float Km = 0.1 /* displacement magnitude */)
{
float	hump = 0;

/* STEP 1 - make a copy of the surface normal one unit in length */
normal n = normalize(N);

/* STEP 2 - calculate an appropriate value for the displacement */
hump = 0;

/* STEP 3 - calculate the new position of the surface point */
/*			"P" based on the value of hump */
P = P - n * hump * Km;

/* STEP 4 - calculate the new orientation of the surface normal */
N = calculatenormal(P);
}
