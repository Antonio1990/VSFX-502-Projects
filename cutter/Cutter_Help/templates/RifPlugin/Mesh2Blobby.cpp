/*
Mesh2Blobby.cpp
Refer to,
	www.fundza.com/devkit/rifplugins/mesh2blobby/index.html
for information about this rif.

Malcolm Kesson
Jan 28 2016
*/
#include <RifPlugin.h>
#include <RixInterfaces.h>
#include <RifFilter.h>
#include <ri.h>
#include <list>
#include <string>
#include <cstdlib>

class Mesh2Blobby : public RifPlugin {
	public:
	     	 Mesh2Blobby(RtFloat scale);
	virtual ~Mesh2Blobby() { }
	virtual RifFilter &GetFilter();
	
	private:
		RifFilter   	m_filters;
		RtFloat  		m_blobscale;
		
		// Utilities_____________________________________________________________
		static RtPoint	*getVerticesPtr(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]);
		static RtInt	countVertices(RtInt npolys, RtInt nvertices[], RtInt vertices[]);
		static RtVoid 	writeBlobby(RtInt numverts, RtPoint *vertices, RtFloat scale);
		
		// Callbacks_____________________________________________________________
		static RtVoid   generalpolygons(RtInt npolys, RtInt *nloops, RtInt *nverts, 
								RtInt* verts, RtInt n, RtToken nms[], RtPointer vals[]);
							
		static RtVoid   hierarchicalSubdivisionMeshV(RtToken mask, RtInt nf,
                    			RtInt nverts[], RtInt verts[], RtInt nt, RtToken tags[],
                    			RtInt nargs[], RtInt intargs[], RtFloat floatargs[],
                    			RtToken stringargs[], RtInt, RtToken[], RtPointer[]);
	};

// Entry point called by PRMan.
RifPlugin *RifPluginManufacture(int argc, char **argv) {
	RtFloat defaultscale = 0.4;
	if(argc == 1 && *argv[0] != '\0')
		defaultscale = atof(argv[0]);
    return new Mesh2Blobby(defaultscale);
	}
		
// Constructor
Mesh2Blobby::Mesh2Blobby(RtFloat scale) {
	m_blobscale = scale;
	//std::cout << "scale = " << scale << "\n";
	m_filters.PointsGeneralPolygonsV = generalpolygons;
	m_filters.HierarchicalSubdivisionMeshV = hierarchicalSubdivisionMeshV;
	m_filters.Filtering = RifFilter::k_Continue;
	}

RifFilter& Mesh2Blobby::GetFilter() { 
	return m_filters; 
	}

// Utility:
// Given the number of points in a vertex array and a scaling factor this 
// method emits a RiBlobby. For example, if "numverts" is 100 this method 
// will output a blobby consisting of 100 merged blobs.
void Mesh2Blobby::writeBlobby(RtInt numverts, RtPoint *vertices, RtFloat scale) {
    RtInt 	numBlobs = numverts;
    RtInt 	numEllipsoidCodes = numBlobs * 2;
	RtInt 	numBlendingCodes = 2 + numBlobs;
	RtInt	numTotalCodes = numEllipsoidCodes + numBlendingCodes;
    RtInt 	codes[numTotalCodes];
	int 	mat_index = 0, n = 0;
	while(n < numEllipsoidCodes) {
		codes[n] = 1001;
		codes[n+1] = mat_index;
		mat_index += 16;
		n += 2;
		}
	// Blending 
	codes[n++] = 0;			// opcode to additively blend all blobs
	codes[n++] = numBlobs;	// number of blobs to blend
	
	RtInt index = 0;
	while(n < numTotalCodes)
		codes[n++] = index++;	
    RtInt 	numFloats = 16 * numBlobs;
    RtFloat mat[numFloats]; // 16 values per matrix
	RtFloat *fPtr = (RtFloat*)vertices;
	for(int n = 0; n < numBlobs * 16; n += 16) {
		mat[n] = scale; mat[n+1] = mat[n+2] = mat[n+3] = 0;
		mat[n+4] = 0;   mat[n+5] = scale;     mat[n+6] = mat[n+7] = 0;
		mat[n+8] = mat[n+9] = 0; mat[n+10] = scale; mat[n+11] = 0;
		mat[n+12] = *fPtr++; 
		mat[n+13] = *fPtr++; 
		mat[n+14] = *fPtr++;
		mat[n+15] = 1;
		}
    RtInt nstrings = 1;
	char *str[1] = {"\0"};
    RiBlobby(numBlobs, numTotalCodes, codes, numFloats, mat, nstrings, str, RI_NULL);
	}

// Utility:
// The writeBlobby() method must know the number of number of blobs - a value we derive
// from the number of unique vertices specified by the rib statements "PointsGeneralPolygons"
// and "HierarchicalSubdivisionMesh". Many of the indices in their nvertices array 
// are duplicates but the use of std::list.unique() ensures we a correct count.
RtInt Mesh2Blobby::countVertices(RtInt npolys, RtInt* nvertices, RtInt* vertices) {
	// Get the raw indices:
	int vertCount = 0;
	for(int n = 0; n < npolys; n++)
		vertCount += nvertices[n];
	// Extract the unique indices:
	std::list<int> int_list;
	for(int n = 0; n < vertCount; n++)
		int_list.push_back(vertices[n]);
	int_list.sort();
	int_list.unique();
	return int_list.size();
	}
	
// Utility:
// Scans the paramNames (token names) until RI_P ("P") is found,
// then returns a pointer (address) to the first vertex of the points array.
RtPoint* Mesh2Blobby::getVerticesPtr(RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
	for(int n = 0; n < paramNum; n++) {
		if(paramNames[n] == RI_P || strcmp(paramNames[n], RI_P) == 0)
			return (RtPoint*)paramVals[n];
		}
	return NULL;
	}
	
// Callback:
// Handles the PointsGeneralPolygons primitive.
RtVoid Mesh2Blobby::generalpolygons(RtInt npolys, RtInt* nloops, RtInt nverts[], RtInt verts[], 
								 	RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
	Mesh2Blobby *self = static_cast<Mesh2Blobby*> (RifGetCurrentPlugin());

	int numverts = countVertices(npolys, nverts, verts);
	RtPoint *vertices = getVerticesPtr(paramNum, paramNames, paramVals);
	writeBlobby(numverts, vertices, self->m_blobscale);
	}
// Callback:
// Handles the HierarchicalSubdivisionMesh primitive.
RtVoid Mesh2Blobby::hierarchicalSubdivisionMeshV(RtToken mask, 
					RtInt nf, RtInt nverts[], RtInt verts[], 
					RtInt nt, RtToken tags[], RtInt nargs[], RtInt intargs[], RtFloat floatargs[],
                    RtToken stringargs[], 
					RtInt paramNum, RtToken paramNames[], RtPointer paramVals[]) {
	Mesh2Blobby *self = static_cast<Mesh2Blobby*> (RifGetCurrentPlugin());
	
	int numverts = countVertices(nf, nverts, verts);
	RtPoint *vertices = getVerticesPtr(paramNum, paramNames, paramVals);
	writeBlobby(numverts, vertices, self->m_blobscale);
	}
