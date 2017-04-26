/* Shader description goes here */
surface
specular_test(float	Ka = 1,			/* ambient brightness */
					Kd = 0.5,		/* basic brightness */
					Ks = 0.7,		/* hilite brightness */
					roughness = 0.1;/* hilite spread */
			color 	hilitecolor = 1	/* hilite color */ )
{
color	ambientcolor, diffusecolor, speccolor,
		surfcolor = 1;

/* STEP 1 - make a copy of the surface normal one unit in length */
normal	n = normalize(N);
normal	nf = faceforward(n, I);

/* STEP 2 - set the apparent surface opacity */
Oi = Os;

/* STEP 3 - calculate the lighting components */
ambientcolor = Ka * ambient();
diffusecolor = Kd * diffuse(nf);

vector	i = normalize(-I);
speccolor = Ks * specular(nf, i, roughness) * hilitecolor;

/* STEP 4 - calculate the apparent surface color */
Ci = Oi * Cs * surfcolor * (ambientcolor + diffusecolor + speccolor);
}
