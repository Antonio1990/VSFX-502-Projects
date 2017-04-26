/* Shader description goes here */
light
point_test(
    float intensity = 1;
    color lightcolor = 1)
{
uniform point lightPosition = point "shader" (0,0,0);

illuminate(lightPosition) {
	// L.L (dot product) returns the distance from the 
	// surface being illuminated to the light source 
	// squared. Therefore, decay is fixed as quadratic 
	// falloff.
	Cl = intensity * lightcolor / L.L;
	Cl *= sin(sqrt(L.L) * 16);
	}
}
