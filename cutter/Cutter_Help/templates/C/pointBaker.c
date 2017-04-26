/* 	Source code for an old style shadeop written in the "C'
	programming language. Also included in this file is the
	code for a shader that can be used the test the shadeop. 
	Malcolm Kesson April 11 2008.
/*
surface
pointBaker(float Kd = 1)
{
color	surfcolor = 1;
normal	n = normalize(N);
color	diffuseColor = diffuse(n) * Kd;

// Get the brightness of the illumination
color	hsvColor = ctransform("rgb", "hsv", diffuseColor);
float	brightness = hsvColor[2];

// Offset the point according to the brightness
point pp = P + n * brightness;
pp =  transform("world",pp);

// Use our custom shadeop to bake the particle
pointBaker( pp );

Oi = Os;
Ci = Oi * Cs * diffuseColor;
}
*/
//--------snip--------snip--------snip--------snip

#include <stdio.h>
#include "shadeop.h"

// Our local private variables go here. 
FILE 	*f = NULL;

SHADEOP_CLEANUP(pointBaker_cleanup) {
if(f != NULL) {
	fprintf(f, "] \"constantwidth\" [0.008]\n");
	fclose(f);
	}
f = NULL;
} 

SHADEOP_TABLE(pointBaker) =
{
	{ "void pointBaker_p (point)", "", "pointBaker_cleanup" }, 
	{""}
};

// This shadeop does not return a value, therefore, 
// argv[0] is not used.
SHADEOP (pointBaker_p)
{
float *pnt = (float*)argv[1];

// The file has not been opened
if(f == NULL) 
	{
	f = fopen("./pointbake.rib", "w");
	fprintf(f, "Points \"P\" [\n");
	}
// Write the xyz position of a point	
fprintf(f, "%f %f %f\n", pnt[0], pnt[1], pnt[2]);
return 0;	// no errors
}

// This function is called once before the shadeop is used
// during a render job. However, it is not used by this
// shadeop.
SHADEOP_INIT(pointBaker_init) {
return 0;
} 


