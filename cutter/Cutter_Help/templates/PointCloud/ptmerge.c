/*
 * ptmerge.c
 * This program demonstrates how to use the pointcloud API to read
 * and write points from/to point cloud files.
 *
 * The program reads some point cloud files and writes their content
 * out as a single point cloud file.
 * Source:
 *	  http://renderman.pixar.com/resources/current/RenderMan/ptcloudApi.html
 * To use the program,
 *    ptmerge infile1.ptc infile2.ptc outfile.ptc
 * or,
 *    ./ptmerge infile1.ptc infile2.ptc outfile.ptc
 */

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "assert.h"

#include "pointcloud.h"


int
main(int argc, char *argv[]) {
    PtcPointCloud *inptcs = NULL; /* an array of PtcPointClouds (pointers) */
    PtcPointCloud outptc = NULL;
    float w2e[16], w2n[16], format[3];
    float point[3], normal[3];
    float radius, *data;
    int nInFiles = argc-2, f, v;
    int datasize, nvars, nv;
    int npoints, p;
    char **vartypes = NULL, **varnames = NULL, **vt = NULL, **vn = NULL; // arrays of strings
    char *inname, *outname;

    if (argc < 3) {
      fprintf(stderr, "ptmerge error: needs at least one input file and an output\n");
      exit(1);
    }

    inptcs = (PtcPointCloud *) malloc(nInFiles * sizeof(PtcPointCloud));

    /* Open the first input files to determine data types */
    inname = argv[1];
    inptcs[0] = PtcSafeOpenPointCloudFile(inname);
    if (!inptcs[0]) {
      fprintf(stderr, "ptmerge error: unable to open input file %s\n",
              inname);
      exit(1);
    }

    PtcGetPointCloudInfo(inptcs[0], "nvars", &nvars);
    PtcGetPointCloudInfo(inptcs[0], "vartypes", &vartypes);
    PtcGetPointCloudInfo(inptcs[0], "varnames", &varnames);

    /* Open following input files (if any) and make sure they have the
       same data types as the first */
    for (f = 1; f < nInFiles; f++) {
      inname = argv[f+1];
      inptcs[f] = PtcSafeOpenPointCloudFile(inname);
      if (!inptcs[f]) {
          fprintf(stderr, "ptmerge error: unable to open input file %s, skipping it.\n",
                  inname);
          continue;
      }

      PtcGetPointCloudInfo(inptcs[f], "nvars", &nv);
      PtcGetPointCloudInfo(inptcs[f], "vartypes", &vt);
      PtcGetPointCloudInfo(inptcs[f], "varnames", &vn);

      if (nv != nvars) {
          fprintf(stderr, "ptmerge error: input files differ number of vars\n");
          exit(1);
      }
      for (v = 0; v < nvars; v++) {
          if (strcmp(vartypes[v], vt[v])) {
              fprintf(stderr, "ptmerge error: input files differ in data types: %s vs %s\n",
                      vartypes[v], vt[v]);
              exit(1);
          }
          if (strcmp(varnames[v], vn[v])) {
              fprintf(stderr, "ptmerge error: input files differ in data names: %s vs %s\n",
                      varnames[v], vn[v]);
              exit(1);
          }
      }
    }

    /* Get transformation matrices and image format from first file */
    PtcGetPointCloudInfo(inptcs[0], "world2eye", w2e);
    PtcGetPointCloudInfo(inptcs[0], "world2ndc", w2n);
    PtcGetPointCloudInfo(inptcs[0], "format", format);

    /* Create output file with the same data types as input file(s) */
    outname = argv[argc - 1];
    outptc = PtcCreatePointCloudFile(outname, nvars, vartypes, varnames,
                                   w2e, w2n, format);
    if (!outptc) {
      fprintf(stderr, "Unable to open output file %s.\n", outname);
      exit(1);
    }

    PtcGetPointCloudInfo(inptcs[0], "datasize", &datasize);
    data = (float *) malloc(datasize * sizeof(float));

    /* Loop over the input files, reading all points and writing them to
       the output file. */
    for (f = 0; f < nInFiles; ++f) {
      PtcGetPointCloudInfo(inptcs[f], "npoints", &npoints);
      inname = argv[f+1];
      printf("input file '%s' has %i points\n", inname, npoints);
      for (p = 0; p < npoints; p++) {
          PtcReadDataPoint(inptcs[f], point, normal, &radius, data);
          PtcWriteDataPoint(outptc, point, normal, radius, data);
      }
      PtcClosePointCloudFile(inptcs[f]);
    }

    free(inptcs);
    free(data);

    /* Finish writing the point cloud data and close files */
    PtcFinishPointCloudFile(outptc);

    printf("merged file '%s' written\n", outname);

    return 0; /* success */
}
