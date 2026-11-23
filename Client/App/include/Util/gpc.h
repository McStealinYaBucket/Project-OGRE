/* Util/gpc.h -- October 2013 reconstruction.

   Interface of the Generic Polygon Clipper, version 2.32 (Alan Murta,
   Advanced Interfaces Group, University of Manchester; free for
   non-commercial use, see the GPC distribution for its terms). App uses it
   from util/Math.cpp; the library itself is App/util/gpc.c.

   2013 evidence: iOS 10-29 has gpc_polygon_clip and gpc_free_polygon, so
   the library was linked; the types and prototypes are GPC's own and must
   stay as they are for gpc.c and its callers. No extern "C": both sides are
   compiled with the same linkage in this tree, as in the prior one. */
#ifndef RBX_UTIL_GPC_H
#define RBX_UTIL_GPC_H

#define GPC_VERSION "2.32"

// Tolerance for merging nearly coincident edges (needs <float.h> at use).
#define GPC_EPSILON (DBL_EPSILON)

// Boolean operation applied by the clip calls.
typedef enum
{
	GPC_DIFF,  // subject minus clip
	GPC_INT,   // intersection
	GPC_XOR,   // exclusive or
	GPC_UNION  // union
} gpc_op;

typedef struct
{
	double x;
	double y;
} gpc_vertex;

// A contour or strip: num_vertices points in vertex[].
typedef struct
{
	int num_vertices;
	gpc_vertex* vertex;
} gpc_vertex_list;

// A polygon: num_contours contours, hole[i] nonzero for a hole.
typedef struct
{
	int num_contours;
	int* hole;
	gpc_vertex_list* contour;
} gpc_polygon;

// A set of triangle strips.
typedef struct
{
	int num_strips;
	gpc_vertex_list* strip;
} gpc_tristrip;

void gpc_add_contour(gpc_polygon* polygon, gpc_vertex_list* contour, int hole);

void gpc_polygon_clip(gpc_op set_operation, gpc_polygon* subject_polygon,
                      gpc_polygon* clip_polygon, gpc_polygon* result_polygon);

void gpc_tristrip_clip(gpc_op set_operation, gpc_polygon* subject_polygon,
                       gpc_polygon* clip_polygon, gpc_tristrip* result_tristrip);

void gpc_polygon_to_tristrip(gpc_polygon* polygon, gpc_tristrip* tristrip);

void gpc_free_polygon(gpc_polygon* polygon);
void gpc_free_tristrip(gpc_tristrip* tristrip);

#endif
