/* Shader description goes here */
class
cSurface_test(float	Kd = 1)
{
constant float 	m_maxdepth;
varying float	m_raydepth;
varying color	m_patC = 1;
varying color	m_texC;
string			m_geoname;

//------------------------------------------------
// Only called once. Get Option values.
//------------------------------------------------
void construct() {
	// Example:
	// option("trace:maxdepth", m_maxdepth);
	}

//------------------------------------------------
// Called once per grid. Get the values of Attributes.
//------------------------------------------------
public void begin() {
	// Examples:
	// attribute("Sides", m_sides);
	// rayinfo("depth", m_raydepth);
	// attribute("identifier:name", m_geoname);
	}

//------------------------------------------------
// Implement this method if it is possible to cheaply
// tell the renderer the opacity of the micropolygon.
//------------------------------------------------
//public void opacity(output color Oi) { }

//------------------------------------------------
// Calculate pattern colors or read texture(s)
//------------------------------------------------
public void prelighting(output color Ci, Oi) { 
	// Examples:
	// m_patC = (s > 0.5) ? color(1,0,0) : color(1,1,1);
	// m_texC = texture(texname);
	}
	
//------------------------------------------------
// Calculate the illumination values and set Oi & Ci.
// Called per point. Note that lights can be queried using
// a category. For example,
// 		lights[] = getlights("category", "diffuse");
//------------------------------------------------
public void lighting(output color Ci, Oi) {
	normal 	n = normalize(N); 
	normal 	nf = faceforward(n, I);
	shader 	lights[] = getlights();
	uniform float 	i, num = arraylength(lights);
	color  	diffusecolor = 0;
	
	for (i = 0; i < num; i += 1) {
		vector L;	// from the lightsource to the surface
		color Cl;	// receives the lightsource color
		lights[i]->light(L, Cl);
		// classic Lambert
		diffusecolor += Cl * max(0, nf.normalize(-L)); 
		}
	Oi = Os;
	Ci = Oi * m_patC * diffusecolor;
	}

//------------------------------------------------
// Final opportunity to adjust Ci and Oi
//------------------------------------------------
public void postlighting(output color Ci, Oi) { }

//------------------------------------------------
// If implemented the next two will be used ONLY during raytracing
//------------------------------------------------
//public void diffuselighting(output color Ci, Oi, [irradiance ] );
//public void specularlighting(output color Ci, Oi);

}
  
