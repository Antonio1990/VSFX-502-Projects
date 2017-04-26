// A barebones template of a physically plausible material that 
// ignores specularity. It assumes the material is opaque. Based
// on,  
//		RPS_18/lib/rsl/shaders/plausibleMatte.sl
// M.A.Kesson
// Aug 12 2013

#include <stdrsl/ShadingContext.h>
#include <stdrsl/RadianceSample.h>
#include <stdrsl/Lambert.h>
#include <stdrsl/Colors.h>
//#include <stdrsl/SampleMgr.h>

class ppMatteTemplate (
    float  diffuseGain = 0.75;
	color  diffuseColor = color(0.18);
	string diffuseMap = "";
	float  samples = 32;	
	float  Km = 0;
	string displacementMap = "";
	string lightCategory = "";
	float  applySRGB = 1;
	string causticMap = "";
	float  photonEstimator = 200;
	float  __computesOpacity = 0;  /* does not implement opacity() */
    )
{
// Member variables
stdrsl_ShadingContext 	m_shadingCtx; 	// handles geo data such as shading normal
stdrsl_Lambert 	m_diffuse;	// handles lambert diffuse calculations
stdrsl_Fresnel 	m_fresnel;	// handles barebones reflection/refraction 
uniform shader	m_lights[];	// list of the active lights

//_____________________________________________________________
// The value(s) of Option(s) would be evaluated here.
public void construct() {
	m_shadingCtx->construct();
	}

//_____________________________________________________________
// The value(s) of Attribute(s) would be evaluated here.
public void begin() { }

//_____________________________________________________________
// the material does not implement a fast opacity lookup.
// public void opacity(output color Oi) { }

//_____________________________________________________________
// Called for both Hider "hidden" and Hider "raytrace".
public void prelighting(output color Ci, Oi) {
	m_lights = getlights("category", lightCategory);
	}
	
//_____________________________________________________________
public void displacement(output point P; output normal N) {
	// Handle surface texture (displacement) mapping.
	if(displacementMap != "" && Km != 0) {
		filterregion filt;
		filt->calculate2d(s, t);
		float disp =  Km * texture(displacementMap[0], filt);
		// Assigns the result of the displacement to P and N.
		m_shadingCtx->displace(m_shadingCtx->m_OrigNn, disp, "displace");
        }
	// Handles the update of the shading normal Ns.
	m_shadingCtx->init();
	}

//_____________________________________________________________
public void initDiff() {
	// Handle surface texture (color) mapping.
	color diffColor = diffuseColor * diffuseGain * Cs;
	if(diffuseMap != "") {
		filterregion filt;
		filt->calculate2d(s, t);
		color c = color texture(diffuseMap, filt);
		if(applySRGB != 0) 
			c = srgbToLinear(c);
		diffColor *= c;
		}
	// Handles the number of samples that will be used by generateSamples().
	m_diffuse->init(m_shadingCtx, diffColor,0,0);
	}
//_____________________________________________________________
public void diffuselighting(output color Ci, Oi) {
	color direct = 0, indirect = 0, caustics = 0;

	initDiff();

	// The directlighting() shadeop accumulates __radiancesample data,
	// 	     this::generateSamples()  triggers  light::generateSamples()
	// 	    light::generateSamples()  triggers   this::evaluateSamples()
	//       this::evaluateSamples()  triggers  light::evaluateSamples()
	// at the conclusion of which the shadeop returns a final color that
	// ignores specularity becuse it is being called from diffuselight().
	direct = directlighting(this, m_lights);
	
	// Handle indirect illumination ie. color bleeding.
	if(samples > 0)
		indirect = indirectdiffuse(P, m_shadingCtx->m_Ns, samples, "maxdist", 1e10);
						
	// Handle any illumination contributed by caustics
	if(causticMap != "")
		caustics = photonmap(causticMap, P, m_shadingCtx->m_Ns,
                                        "estimator", photonEstimator);
	Ci = Ci + direct + indirect + caustics;
	}

//_____________________________________________________________
// Triggered by the call to the directlighting() shadeop.
public void generateSamples(string type;
							output __radiancesample samples[]) {
	if (type == "diffuse")// || type == "diffusespecular")
		m_diffuse->genDiffuseSamps(m_shadingCtx, m_fresnel, samples);
	}
	
//_____________________________________________________________
// Called by each light::generateSamples() method.
public void evaluateSamples(string distribution;
							output __radiancesample samples[]) {
	if (distribution == "diffuse")
		m_diffuse->evalDiffuseSamps(m_shadingCtx, m_fresnel, samples);    
	}
}
