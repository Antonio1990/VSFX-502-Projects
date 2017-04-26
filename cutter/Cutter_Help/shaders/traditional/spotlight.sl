/*
	The light should be instanced in a rib file as follows,
	TransformBegin
		Translate 0 5 0
        Rotate -90 1 0 0
        Scale 1 1 -1 # <<-- note the negative z scaling
        LightSource "spot_test" 2 "intensity" 1
    TransformEnd
   For more information refer to,
   		http://fundza.com/rman_shaders/lights/directional/index.html#maya_houdini
*/
light
spot_test(
    float	intensity = 1;
    color	lightcolor = 1;
    float	coneangle = 30;
    float	conedeltaangle = 5;
    float	decay = 2)
{
float	atten, cosangle;

// The next group of variables are uniform because 
// they do not change across the surface being illuminated
uniform vector	axis = vector "shader" (0,0,1);
uniform	float	outer = radians(coneangle),
				inner = radians(coneangle - conedeltaangle);
uniform float 	outer_cosine = cos(outer),
		  		inner_cosine = cos(inner);
uniform point	lightPosition = point "shader" (0,0,0);

illuminate(lightPosition, axis, coneangle) {

	cosangle = L.axis / length(L);
	
	// L.L (dot product) returns the distance from the 
	// surface being illuminated to the light source 
	// squared. Therefore, decay is fixed as quadratic falloff.
	atten = pow(cosangle, decay)/(L.L);
	
	// Drop the intensity in the penumbra region
	atten *= smoothstep(outer_cosine, cos(inner), cosangle);
	Cl = atten * intensity * lightcolor;
	}
}
