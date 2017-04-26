// Pixar Source: 
//     http://renderman.pixar.com/resources/current/RenderMan/bkmapApi.html
// brickmaplookup.cpp
// This program demonstrates how to use the brick map API to read
// data from brick map files.
//
// Input: a brick map name, position p, and normal n. For example,
//     ./brickmaplookup YOUR_FILE.bkm 1.5 0.5 0 0 0 0
// Output: the brick map data in the voxel corresponding to p and n.
//

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "assert.h"
#include <brickmap.h>
#define N 8
#define TRUE 1
#define FALSE 0

typedef struct xyz {
    float x, y, z;
} xyz;

typedef struct bbox {
    struct xyz min, max;
} bbox;


//
// Determine which child octant p is in and and that octant's bounding box
//
static void
determineChild(struct bbox *bbox, struct xyz *p,
             int *child, struct bbox *childbbox) // results
{
    struct xyz midpoint;
    int ci, cj, ck;

    // Compute the midpoint of brick bbox
    midpoint.x = 0.5f * (bbox->max.x + bbox->min.x);
    midpoint.y = 0.5f * (bbox->max.y + bbox->min.y);
    midpoint.z = 0.5f * (bbox->max.z + bbox->min.z);

    // Compute which child octant point p is in: ci,cj,ck
    ci = (p->x >= midpoint.x) ? 1 : 0;
    cj = (p->y >= midpoint.y) ? 2 : 0;
    ck = (p->z >= midpoint.z) ? 4 : 0;
    *child = ci + cj + ck;
    assert(0 <= *child && *child <= 7);

    // Compute child bbox
    childbbox->min.x = ci ? midpoint.x : bbox->min.x;
    childbbox->min.y = cj ? midpoint.y : bbox->min.y;
    childbbox->min.z = ck ? midpoint.z : bbox->min.z;
    childbbox->max.x = ci ? bbox->max.x : midpoint.x;
    childbbox->max.y = cj ? bbox->max.y : midpoint.y;
    childbbox->max.z = ck ? bbox->max.z : midpoint.z;
}


//
// Determine which voxel (i,j,k) of bbox that point p is in
//
static void
determineVoxel(struct bbox *bbox, struct xyz *p,
             int *i, int *j, int *k) // results
{
    double fi, fj, fk; // double precision required for tiny voxels
    float edgelen;

    edgelen = bbox->max.x - bbox->min.x; // same for y and z since square bricks

    // Compute which voxel (i,j,k) the point p is in
    fi = (p->x - bbox->min.x) / edgelen * N;
    fj = (p->y - bbox->min.y) / edgelen * N;
    fk = (p->z - bbox->min.z) / edgelen * N;

    *i = (int)fi;
    *j = (int)fj;
    *k = (int)fk;
    assert(-1 <= *i && *i <= N);
    assert(-1 <= *j && *j <= N);
    assert(-1 <= *k && *k <= N);

    // Ugly detail: point p can sometimes be right on or outside the
    // bbox, so i, j, or k gets a value of -1 or N.  Particularly
    // common is that p is right on the bbox max value so one of the
    // indices gets the value N.  We pretend that the point was inside
    // by moving the offending index into the range [0,N-1].
    if (*i == -1) *i = 0;
    if (*i == N) *i = N-1;
    if (*j == -1) *j = 0;
    if (*j == N) *j = N-1;
    if (*k == -1) *k = 0;
    if (*k == N) *k = N-1;
    assert(0 <= *i && *i < N);
    assert(0 <= *j && *j < N);
    assert(0 <= *k && *k < N);
}


//
// Recursively lookup in the brick map
//
int
recLookup(struct xyz *p, struct xyz *n, BkmBrickMap brickmap,
        BkmBrickMapNode node, struct bbox *nodebbox,
        int datasize, float *data)
{
    BkmBrickMapNode children[8], child;
    struct bbox childbbox;
    float *voxeldata;
    int c; // child octant: [0..7]
    int i, j, k; // voxel containing point p
    int v, d;
    int nvoxels, voxelnumbers[512], voxelhasdata[512];
    int success;

    // Get the octree child node that contains p
    //BkmGetOctreeChildren(node, children); // for PRMan 14
    BkmGetOctreeChildrenList(brickmap, node, children); // recommended for PRMan 15
    determineChild(nodebbox, p, &c, &childbbox);
    child = children[c];

    // Call child node if present
    if (child) { // child node exists: recurse
        success = recLookup(p, n, brickmap, child, &childbbox,
                            datasize, data);
        if (success) return TRUE;
    }

    // We get to this point if the node is a leaf or if the child
    // lookup failed (no voxel data).  Lookup voxel in this brick.

    determineVoxel(nodebbox, p, &i, &j, &k);
    voxeldata = (float*) calloc(datasize * 512, sizeof(float));
    BkmGetVoxelData(brickmap, node, i, j, k, i, j, k,
                    &nvoxels, voxelnumbers, voxelhasdata, voxeldata);

    if (nvoxels == 0)
        return FALSE; // failure

    v = i + j*N + k*N*N;
    assert(voxelhasdata[v]);
    for (d = 0; d < datasize; d++)
        data[d] = voxeldata[v*datasize+d];

    return TRUE; // success
}


//
// Input: a brick map name, position p, and normal n.
// Output: the brick map data in the voxel corresponding to p and n.
//
int
main(int argc, char *argv[]) {
    BkmBrickMap brickmap = NULL; // (a void pointer)
    BkmBrickMapNode roots[7], root; // (void pointers)
    struct bbox bbox;
    struct xyz point, normal;
    float absnx, absny, absnz;
    float *data;
    int nvars, datasize;
    int octree, v, d;
    int success;
    char **vartypes = NULL, **varnames = NULL; // arrays of strings
    char *filename;

    if (argc != 8) {
        fprintf(stderr, "brickmaplookup: needs 7 input: filename px py pz nx ny nz\n");
        exit(1);
    }

    // Parse input
    filename = argv[1];
    point.x = atof(argv[2]);
    point.y = atof(argv[3]);
    point.z = atof(argv[4]);
    normal.x = atof(argv[5]);
    normal.y = atof(argv[6]);
    normal.z = atof(argv[7]);
    printf("filename = '%s'; p = (%f %f %f); n = (%f %f %f)\n",
           filename, point.x, point.y, point.z, normal.x, normal.y, normal.z);

    // Open the brick map file
    brickmap = BkmOpenBrickMapFile(filename);
    if (!brickmap) {
        fprintf(stderr, "brickmaplookup error: unable to open file '%s'\n",
                filename);
        exit(1);
    }

    // Get various info about the brick map contents
	char bboxStr[] = "bbox";
	char nvarsStr[] = "nvars";
	char vartypesStr[] = "vartypes";
	char varnamesStr[] = "varnames";
	char datasizeStr[] = "datasize";
	BkmGetBrickMapInfo(brickmap, bboxStr, &bbox);
    //BkmGetBrickMapInfo(brickmap, "bbox", &bbox);
    BkmGetBrickMapInfo(brickmap, nvarsStr, &nvars);
    BkmGetBrickMapInfo(brickmap, vartypesStr, &vartypes);
    BkmGetBrickMapInfo(brickmap, varnamesStr, &varnames);
    BkmGetBrickMapInfo(brickmap, datasizeStr, &datasize);

    // Print variable types and names
    printf("Variables in brick map '%s':\n", filename);
    for (v = 0; v < nvars; v++)
        printf("  var %i: %s %s\n", v, vartypes[v], varnames[v]);
    printf("  (datasize = %i)\n", datasize);

    // Get the 7 brick map octree roots and pick the one appropriate for
    // the normal
    BkmGetOctreeRoots(brickmap, roots);
    absnx = fabsf(normal.x);
    absny = fabsf(normal.y);
    absnz = fabsf(normal.z);
    if (absnx*absnx + absny*absny + absnz*absnz < 1e-6f)
        octree = 0;
    else if (absnx >= absny && absnx >= absnz) // x is dominant
        octree = (normal.x > 0.0f) ? 1 : 2;
    else if (absny >= absnx && absny >= absnz) // y is dominant
        octree = (normal.y > 0.0f) ? 3 : 4;
    else // z is dominant
        octree = (normal.z > 0.0f) ? 5 : 6;

    root = roots[octree];
    if (root == NULL) {
        printf("No data for normal (%f %f %f) ~ octree %i\n",
                normal.x, normal.y, normal.z, octree);
        exit(1);
    }

    // Do recursive lookup in brick map octree
    data = (float*) calloc(datasize, sizeof(float));
    success = recLookup(&point, &normal, brickmap, root, &bbox, datasize, data);

    if (success) {
        // Print lookup result
        printf("Data for point (%f %f %f) and normal (%f %f %f):\n",
               point.x, point.y, point.z, normal.x, normal.y, normal.z);
        for (d = 0; d < datasize; d++)
            printf("  data %i: %f\n", d, data[d]);
    } else { // failure
        printf("No data for point (%f %f %f) and normal (%f %f %f)\n",
               point.x, point.y, point.z, normal.x, normal.y, normal.z);
    }
return 0;
}
