#include <RifPlugin.h>
#include <RifFilter.h>
#include <ri.h>
#include <string>
#include <rx.h>
  
class Particles_Rif : public RifPlugin {
    public:
             Particles_Rif(RtFloat scale);
    virtual ~Particles_Rif() { }
    virtual RifFilter &GetFilter();
    
    private:
        RifFilter   m_filters;
        RtFloat     m_size;
  
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
    return new Particles_Rif(defaultsize);
    }
        
// Constructor
Particles_Rif::Particles_Rif(RtFloat size) {
    m_size = size;
    m_filters.PointsV = points;
    m_filters.CurvesV = curves;
    m_filters.Filtering = RifFilter::k_Continue;
    }
  
RifFilter& Particles_Rif::GetFilter() { 
    return m_filters; 
    }
    
// Callback:
// Handles the Points and .
RtVoid Particles_Rif::points(RtInt nverts, RtInt nvals, RtToken tokens[], RtPointer params[]) {
    Particles_Rif *self = static_cast<Particles_Rif*> (RifGetCurrentPlugin());
    for(int n = 0; n < nvals; n++) {
        if(strcmp(tokens[n], RI_CONSTANTWIDTH) == 0) {
            *(RtFloat*)params[n] = self->m_size; 
            }
        }
    RiPointsV(nverts, nvals, tokens, params);
    }
RtVoid Particles_Rif::curves(RtToken type, RtInt ncurves, RtInt nvert[], RtToken wrap,
                         RtInt nvals, RtToken tokens[], RtPointer params[]) {
    Particles_Rif *self = static_cast<Particles_Rif*> (RifGetCurrentPlugin());
    for(int n = 0; n < nvals; n++) {
        if(strcmp(tokens[n], RI_CONSTANTWIDTH) == 0) {
            *(RtFloat*)params[n] = self->m_size; 
            }
        }
    RiCurvesV(type, ncurves, nvert, wrap, nvals, tokens, params);
    }

