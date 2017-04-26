/*
A rif that accepts one input that will be applied as a scaling factor to
all shading rates except those with a value of 100 or higher. The code
provides a example of how to implement a simple Rif plugin. 
Malcolm Kesson
16 July 2015
*/

#include <stdio.h>
#include <stdlib.h>
#include <RifPlugin.h>
#include <RifFilter.h>
#include <string>
//#include <sstream>
//#include <RixInterfaces.h>
//#include <rx.h>
//#include <ri.h>

class ShadingRate : public RifPlugin {
	public:
			 ShadingRate(RtFloat scalingFactor);
	virtual ~ShadingRate() { }	
	virtual RifFilter &GetFilter() { return m_filter; }
	
	private:
		RifFilter   m_filter;
		RtInt		m_rate_scaler;
		static RtVoid	shadingrate(RtFloat rate);
	};

//-----------------------------------------------------
// RifPluginManufacture
//-----------------------------------------------------
// Entry point
RifPlugin *RifPluginManufacture(int argc, char **argv) {
	RtFloat n = 1;
	if(argc > 0)
		n = atof(argv[0]);
	n = (n < 0) ? 1 : n;
    return new ShadingRate(n);
	}
//-----------------------------------------------------
// ShadingRate
//-----------------------------------------------------
// Constructor
ShadingRate::ShadingRate(RtFloat scaleFactor) {
	m_filter.ShadingRate = shadingrate;  // install our custom callback
	m_rate_scaler = scaleFactor;
	}
//-----------------------------------------------------
// ShadingRate
//-----------------------------------------------------
// Our callback version of "RiShadingRate"
RtVoid ShadingRate::shadingrate(RtFloat rate) {
	ShadingRate *self = static_cast<ShadingRate*> (RifGetCurrentPlugin());
	RtFloat newrate = rate;
	if(rate < 100)
		newrate = self->m_rate_scaler * rate;
	RiShadingRate(newrate);
	}






















