/*
Partices_rif.cpp
Compiling:
	__WINDOWS__________
	Users must have Visual Studio installed on their computer. In 
	Cutter's preferences go to the 
		Languages->Cpp tab->VS Command Line Setup
	and browse for Visual Studio's vcvarsx86_amd64.bat file.
	For example,
	Microsoft Visual Studio 12.0/VC/bin/x86_amd64/vcvarsx86_amd64.bat
	
	__LINUX & OSX_____
	Users do NOT need to change any of the Cpp compiler options.
	
	To compile use the keyboard shortcut ALT + e or CONTROL + e.
Useage: 
    In Maya open the Render Settings dialog window and ensure 
		RenderMan/RIS
	is being used. Go to the Advanced tab and open the "Ri Filters"
	panel.
		- click the "New Rif" button,
		- browse for the compiled Particles.dll or Particles.so,  
        - in the "Rif Args" text field enter the thickness of the
          points or curves. For example, 0.02
    Render a frame.
Refer to,
    www.fundza.com/devkit/rifplugins/particles/index.html
for more details about using this rif.
Malcolm Kesson
Feb 6 2016
*/
#include <RifPlugin.h>
#include <RifFilter.h>
#include <ri.h>
#include <string>
#include <rx.h>

class Particles : public RifPlugin {
	public:
	     	 Particles(RtFloat scale);
	virtual ~Particles() { }
	virtual RifFilter &GetFilter();
	
	private:
		RifFilter   m_filters;
		RtFloat  	m_size;

		// Callback_____________________________________________________________
		static RtVoid   points(RtInt nverts, RtInt, RtToken[], RtPointer[]);
		static RtVoid   curves(RtToken type, RtInt ncurves, RtInt nvert[], 
                    		   RtToken wrap, RtInt, RtToken[], RtPointer[]);
	};

// Entry point called by PRMan.
RifPlugin *RifPluginManufacture(int argc, char **argv) {
	RtFloat defaultsize = 0.51;
	if(argc == 1)// && *argv[0] != '\0')
		defaultsize = atof(argv[0]);
    return new Particles(defaultsize);
	}
		
// Constructor
Particles::Particles(RtFloat size) {
	m_size = size;
	m_filters.PointsV = points;
	m_filters.CurvesV = curves;
	m_filters.Filtering = RifFilter::k_Continue;
	}

RifFilter& Particles::GetFilter() { 
	return m_filters; 
	}
	
// Callback:
// Handles the Points and .
RtVoid Particles::points(RtInt nverts, RtInt nvals, RtToken tokens[], RtPointer params[]) {
	Particles *self = static_cast<Particles*> (RifGetCurrentPlugin());
    for(int n = 0; n < nvals; n++) {
		if(strcmp(tokens[n], RI_CONSTANTWIDTH) == 0) {
			*(RtFloat*)params[n] = self->m_size; 
			}
		}
	RiPointsV(nverts, nvals, tokens, params);
	}
RtVoid Particles::curves(RtToken type, RtInt ncurves, RtInt nvert[], RtToken wrap,
						 RtInt nvals, RtToken tokens[], RtPointer params[]) {
	Particles *self = static_cast<Particles*> (RifGetCurrentPlugin());
    for(int n = 0; n < nvals; n++) {
		if(strcmp(tokens[n], RI_CONSTANTWIDTH) == 0) {
			*(RtFloat*)params[n] = self->m_size; 
			}
		}
	RiCurvesV(type, ncurves, nvert, wrap, nvals, tokens, params);
	}
