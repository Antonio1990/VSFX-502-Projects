// Copied from RPS/lib/rsl/shaders/helloAS.sl
// Renamed ppAshikhminShirley for use with Cutters beauty rib templates.
// A barebones template of a physically plausible shiny material. It 
// assumes the material is opaque. Based on,  
//		RPS_18/lib/rsl/shaders/helloAS.sl
// M.A.Kesson
// Aug 12 2013

#include <stdrsl/RadianceSample.h>
#include <stdrsl/ShadingContext.h>
#include <stdrsl/SpecularAS.h>
#include <stdrsl/Fresnel.h>
#include <stdrsl/Lambert.h>
#include <stdrsl/Colors.h>

class ppShinyTemplate(
	float  diffuseGain = .75; 
	color  diffuseColor = color(.18);
	string diffuseMap = "";
	string roughnessMap = "";
	float  Km = 0;
	string displacementMap = "";
	string lightCategory = "";
	
	float  specularGain = 0.75; 
	float  anisotropy = 0;
	color  specularColor = color(1);
	float  specularRoughness = 0.02;
	float  indirectDiffuseSamples = 64;
	float  specularSamples = 16;
	float  applySRGB = 1;
	float  photonEstimator = 100;
	string causticMap = "";
	float __computesOpacity = 0;
	)
{
stdrsl_ShadingContext 	m_shadingCtx;
stdrsl_SpecularAS 		m_specular;
stdrsl_Fresnel 	m_fresnel;	// handles barebones reflection/refraction 
stdrsl_Lambert 	m_diffuse;	// handles lambert diffuse calculations
uniform float	m_oneByPI = 1/PI;
shader	m_lights[];

//_____________________________________________________________
// The value(s) of Option(s) would be evaluated here.
public void construct() {
	m_shadingCtx->construct();
	}
//_____________________________________________________________
// The value(s) of Attribute(s) would be evaluated here.
public void begin() {
	m_shadingCtx->init();
	}

//_____________________________________________________________
// the material does not implement a fast opacity lookup.
// public void opacity(output color Oi) { }

//_____________________________________________________________
public void displacement(output point P;output normal N) {
	// Handle surface texture (displacement) mapping.
	if(displacementMap != "" && Km != 0) {
		filterregion filt;
		filt->calculate2d(s, t);
		float disp =  Km * texture(displacementMap[0], filt);
		// Assigns the result of the displacement to P and N.
		m_shadingCtx->displace(m_shadingCtx->m_OrigNn, disp, "displace");
        }
	// Initialize the shading context, using 's' as the input for
	// computing the tangents needed for anisotropic specular.
	m_shadingCtx->initWithTangentField(s);
	}
	
//_____________________________________________________________
// Called for both Hider "hidden" and Hider "raytrace"
public void prelighting(output color Ci, Oi) {
	m_lights = getlights("category", lightCategory);
	}
	
//_____________________________________________________________
public void initSpec() {
	m_fresnel->init(m_shadingCtx, 1/*mediaior*/, 1.33/*ior*/);

	float roughscale = 1;
	if(roughnessMap!= "")
		roughscale = texture(roughnessMap[0]);
	roughscale *= 1 - m_fresnel->m_Kr; // reduce roughness at grazing angle
	
	m_specular->init(m_shadingCtx, 
	     		specularGain * specularColor * m_fresnel->m_Kr, 
				specularRoughness * roughscale,
				anisotropy/*anisoRatio*/, 
				1/*roughscale*/, 
				1/*minsamps*/, 
				specularSamples);
	// What about spec map????
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
	m_diffuse->init(diffColor);
	}

//_____________________________________________________________
// In lighting we deliver full solution, obtain the diffuse and specular results and apply 
// fresnel to diffuse term.
//
// This is not called IF the RIB specifies Hider "raytrace" AND the pipeline methods
// diffuselighting() and specularlighting() have not beem implemented by the shader.
// However, if the rib specifies Hider "hidden" then lighting() will be used even if the
// shader DOES implement diffuselighting() and specularlighting().
public void lighting(output color Ci, Oi) {
	color  diff = 0, spec, sss = 1;
	
	initDiff();
	initSpec();
	
	__radiancesample samps[];
	directlighting(this, m_lights, "specularresult", spec, "diffuseresult", diff,
	                       			"materialsamples", samps); 

	color indspec = indirectspecular(this, "materialsamples", samps);

	if(indirectDiffuseSamples > 0)
		diff += m_diffuse->m_diffColor * indirectdiffuse(P,m_shadingCtx->m_Ns, 
					indirectDiffuseSamples);

	if(causticMap != "")
		diff += m_diffuse->m_diffColor * 
					photonmap(causticMap, P, m_shadingCtx->m_Ns, "estimator", photonEstimator);
					
	Ci += diff * m_fresnel->m_Kt + spec + indspec;
	}


//_____________________________________________________________
// In diffuselighting, we produce view-independent lighting (no fresnel).
//
// Only called when the RIB specifies Hider "raytrace" or if the
// lighting() method has not been implemented by a shader.
public void diffuselighting(output color Ci, Oi, Ii) {
	initDiff();

	Ci = directlighting(this, m_lights);
	if(indirectDiffuseSamples > 0)
		Ci += m_diffuse->m_diffColor * 
			indirectdiffuse(P, m_shadingCtx->m_Nn, indirectDiffuseSamples);
	
	}
	
//_____________________________________________________________
// In specularlighting we produce final radiance assuming 
// diffuse result is present in Ci. We share samples generated 
// by indirectspecular with directlighting.
// Only called when the RIB specifies Hider "raytrace" or if the
// lighting() method has not been implemented by a shader.
public void specularlighting(output color Ci, Oi) {
	color diff = Ci;
	__radiancesample samps[];
	color spec = directlighting(this, m_lights, 
	                                    "materialsamples", samps);
	color indspec = indirectspecular(this, "materialsamples", samps);
	Ci = diff * m_fresnel->m_Kt + spec + indspec;
	}

//_____________________________________________________________
public void evaluateSamples(string distribution;
                            output __radiancesample samples[]) {
	if(distribution == "diffuse") {
		m_diffuse->evalDiffuseSamps(m_shadingCtx, m_fresnel, samples);
		}
	else
		if(distribution == "specular") {
			m_specular->evalSpecularSamps(m_shadingCtx, m_fresnel, samples);
			}
	}
//_____________________________________________________________
public void generateSamples(string type; output __radiancesample samples[]) {
	if(type == "specular") {
		m_specular->genSpecularSamps(m_shadingCtx, m_fresnel, type, samples);
		}
	}
}



