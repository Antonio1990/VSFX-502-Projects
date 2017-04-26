/* Example of a basic (RSL2) class based shader. Either the
   displacement() or surface() method may be omitted. As shown,
   the shader enables the surface() method to access the value
   of the member variable m_hump so that coloration can be 
   sync'd to the displacement.
*/
struct Test {
	uniform float foo = 1;
	
	}
	
class 
classStruct(float	Kd = 1,
					Km = 0.1)
{
varying float  m_hump = 0;
varying normal m_norm = 0;
Test tes;


//------------------------------------------------
// Only called once. Get Option values.
//------------------------------------------------
void construct() {
	tes = Test("foo", 2);
	}

public void displacement(output point P; output normal N) {
	m_norm = normalize(N);
	m_hump = 0; // displacement code here
	P = P - m_norm * m_hump * Km;
	N = calculatenormal(P);
	}
	  
public void surface(output color Ci, Oi) {
	

	m_norm = normalize(N); // re-normize the displaced normal
	normal 	nf = faceforward(m_norm, I);
	color   surfcolor = Cs;
	color  	diffusecolor = 0;
	shader 	lights[] = getlights();
	uniform float 	i, num = arraylength(lights);
	
	for (i = 0; i < num; i += 1) {
		vector L;	// from the light source to the surface
		color Cl;	// receives the light source color
		lights[i]->light(L, Cl); // query the light source
		// Apply the classic Lambert lighting model
		diffusecolor += Cl * max(0, nf.normalize(-L)); 
		}
	Oi = Os;
	Ci = Oi * surfcolor * diffusecolor;
    }
}
