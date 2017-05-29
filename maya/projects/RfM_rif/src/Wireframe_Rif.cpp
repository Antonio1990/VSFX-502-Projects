/*
Wireframe_Rif.cpp
Refer to,
    www.fundza.com/devkit/rifplugins/Wireframe/index.html
for information about this rif.
  
Malcolm Kesson
Sept 8 2016
  
Sept 16 2016: Corrected a memory copy error
*/
#include <RifPlugin.h>
#include <RixInterfaces.h>
#include <RifFilter.h>
#include <ri.h>
#include <rx.h>
#include <string>
#include <sstream>
#include <stdlib.h> 
  
#define MESH 0
#define WIRE 1
#define WIRE_AND_MESH 2
#define DEFAULT_MODE WIRE_AND_MESH
  
class Wireframe_Rif : public RifPlugin {
    public:
              Wireframe_Rif(RtFloat width);
    virtual ~Wireframe_Rif() { }
    virtual RifFilter &GetFilter();
    
    private:
        RixMessages     *m_msg;
        RixRenderState  *m_rstate;
        RifFilter       m_filters;
        RtFloat         m_width;
        
        // Utilities_____________________________________________________________
        virtual RtPoint *getPolyPoints(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]);
        virtual int     getRenderMode(std::string name);
        virtual RtVoid  convertToCurves(RtInt npolys, RtInt nloops[], RtInt vertsPerLoop[], RtInt vertsIndices[],
                                            RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]);
        // Callbacks_____________________________________________________________
        static RtVoid   polygonV(RtInt npolys, RtInt *nloops, RtInt *nverts, 
                                RtInt* verts, RtInt n, RtToken nms[], RtPointer vals[]);
    };
  
//-------------------------------------------------------------
// Entry point called by PRMan.
RifPlugin *RifPluginManufacture(int argc, char **argv) {
    RtFloat defaultwidth = 0.025;
    if(argc == 1 && *argv[0] != '\0')
        defaultwidth = atof(argv[0]);
    return new Wireframe_Rif(defaultwidth);
    }
//-------------------------------------------------------------
// Constructor
Wireframe_Rif::Wireframe_Rif(RtFloat scale) {
    m_width = scale;
    m_filters.PointsGeneralPolygonsV = polygonV;
    m_filters.Filtering = RifFilter::k_Continue;
    
    RixContext  *rixContext = RxGetRixContext();
    m_msg = (RixMessages*) rixContext->GetRixInterface(k_RixMessages);
    m_rstate = (RixRenderState*) rixContext->GetRixInterface(k_RixRenderState);
    }
//-------------------------------------------------------------
RifFilter& Wireframe_Rif::GetFilter() { 
    return m_filters; 
    }
//-------------------------------------------------------------
// Callback:
RtVoid Wireframe_Rif::polygonV(RtInt npolys, RtInt nloops[], RtInt vertsPerLoop[], RtInt vertsIndices[], 
                                  RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    Wireframe_Rif *self = static_cast<Wireframe_Rif*> (RifGetCurrentPlugin());
  
    // If the atttibute is absent getRenderMode() will return DEFAULT_MODE,
    // either MESH, WIRE or WIRE_AND_MESH
    int mode = self->getRenderMode("user:wireframe");
  
    if(mode == 0)
        RiPointsGeneralPolygonsV(npolys, nloops, vertsPerLoop, vertsIndices, 
                                 paramNum, paramNames, paramVals);
    else if(mode == 1)
        self->convertToCurves(npolys, nloops, vertsPerLoop, vertsIndices, paramNum, paramNames, paramVals);
    else
        {
        RiPointsGeneralPolygonsV(npolys, nloops, vertsPerLoop, vertsIndices, 
                                 paramNum, paramNames, paramVals);
        self->convertToCurves(npolys, nloops, vertsPerLoop, vertsIndices, paramNum, paramNames, paramVals);
        }
    }
//_______________________________________________________________________
// Utility:
// Returns a pointer to the first point of the "P" array.
RtPoint* Wireframe_Rif::getPolyPoints(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    for(int n = 0; n < paramNum; n++) {
        if(paramNames[n] == RI_P || strcmp(paramNames[n], RI_P) == 0)
            return (RtPoint*)paramVals[n];
        }
    return NULL;
    }
//_______________________________________________________________________
// Utility:
// It is assumed the data of the "P" array of the source PointsGeneralPolygonsV geometry will
// match the data of the "P" array of the destination RiCurves. A copy of the original date is
// assigned a re-ordered list of points - then the memcopy in step 1 and the 
RtVoid Wireframe_Rif::convertToCurves(RtInt npolys, RtInt nloops[], RtInt vertsPerLoop[], RtInt vertsIndices[], 
                                  RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
    Wireframe_Rif *self = static_cast<Wireframe_Rif*> (RifGetCurrentPlugin());
    
    // Count the number of source polygon "P" vertices.
    int numVerts;
    int numLoops;
    int vertCount = 0;
    int loopCounter = 0;
    for(int n = 0; n < npolys; n++) {
        numLoops = nloops[n];
        for(int j = 0; j < numLoops; j++) {
            numVerts = vertsPerLoop[loopCounter++];
            vertCount += numVerts;
            }
        }
    // Step 1: Get a pointer to the source "P" data and copy it to a temp block.
    RtPoint *polyPointsPtr = getPolyPoints(paramNum, paramNames, paramVals);
    RtPoint *polyPointsCopy = (RtPoint*) calloc(1, sizeof(RtPoint) * vertCount); // temporary copy
    memcpy(polyPointsCopy, polyPointsPtr, sizeof(RtPoint) * vertCount);
    
    // Step 2: Re-order the "P" data so that each curve will have the correct
    //           sequence of points.
    int     vertIndex;
    int     vertCounter = 0;
    RtFloat *fPtr;
    loopCounter = 0;
    for(int n = 0; n < npolys; n++) {
        for(int j = 0; j < nloops[n]; j++) {
            numVerts = vertsPerLoop[loopCounter++]; 
            for(int k = 0; k < numVerts; k++) {
                vertIndex = vertsIndices[vertCounter];
                fPtr = (RtFloat*) &polyPointsCopy[vertIndex];
                
                polyPointsPtr[vertCounter][0] = *fPtr++;
                polyPointsPtr[vertCounter][1] = *fPtr++;
                polyPointsPtr[vertCounter][2] = *fPtr++;
                vertCounter++;
                }
            }
        }
    // Comment this block of code if you do not wish to always have the curves rendered
    // as cylinders. Adding the "roundcurve" attribute to the shape node of any meshes
    // will achieve the same result.
    //-----------------------------------------------------
    RixContext      *rix = RxGetRixContext();
    RixTokenStorage *tok_store = static_cast<RixTokenStorage*>(rix->GetRixInterface(k_RixGlobalTokenData));
    RtToken   tok = (char*)tok_store->GetToken("int roundcurve");
    RtToken   tokens[] = { tok };
    int       value = 1;
    RtPointer values[] = { &value };
    RiAttributeV(RI_DICE, 1, tokens, values);
    //-----------------------------------------------------
    
    // Step 3: Output the RiCurves and free the temporaty copy of the original "P" data.
    int         numCurves = loopCounter;
    RtToken     pNames[2] = { RI_P, RI_CONSTANTWIDTH };
    RtFloat     width = self->m_width;
    RtPointer     outVals[2] = { polyPointsPtr, &width };
    RiCurvesV(RI_LINEAR, numCurves, vertsPerLoop, RI_PERIODIC, 2, pNames, outVals);
    free(polyPointsCopy);
    }
//_______________________________________________________________________
// Utility:
// Returns the int value of the "user:wireframe" attribute.
// 0 means use original polymesh
// 1 menas use curves for wireframe
// 2 means use both polymesh and curves.
int Wireframe_Rif::getRenderMode(std::string name) {
    int    result = -1;
    RtInt     resultCount, resultError, resultLen = sizeof(int*);
    RixRenderState::Type resultType;
    resultError = m_rstate->GetAttribute(name.c_str(), &result, resultLen,
                        &resultType, &resultCount);
    if(resultError == 0)
        return result;
    return DEFAULT_MODE;
    }

