#pragma once
#include <afxwin.h>
#include <assert.h>
#include <iritprsr.h>
#include "Vector.h"
#include "Matrix.h"

#define NUM_OF_AXES 3

#define BG_DEFAULT_COLOR	RGB(0, 0, 0)
#define WIRE_DEFAULT_COLOR	RGB(255, 255, 255)
#define FRAME_DEFULAT_COLOR	RGB(255, 0, 0,)

#define FRAME_WIDTH 2

// TODO: combine all CCGview and IRitworld shared parameters
// such as this one and m_nAxis
enum Axis {
	X_AXIS,
	Y_AXIS,
	Z_AXIS
};

struct IritPoint {
	Vector vertex;
	Vector normal;

	bool has_normal;

	struct IritPoint *next_point;
};

struct State {
	bool vertex_normals;
	bool polygon_normals;
	bool object_frame;
	bool perspective;
	bool object_transform;

	Matrix ratio_mat;
	Matrix coord_mat;
	Matrix center_mat;
	Matrix world_mat;
	Matrix object_mat;
};

class IritPolygon {
	int m_point_nr;
	struct IritPoint *m_points;

	IritPolygon *m_next_polygon;

public:
	Vector normal;
	bool has_normal;

	IritPolygon();

	~IritPolygon();
	
	bool addPoint(struct IritPoint &point);

	/* Creates an homogenious with a normal */
	bool addPoint(double &x, double &y, double &z, double &normal_x, double &normal_y,
		double &normal_z);

	bool addPoint(IPVertexStruct *vertex, bool has_normal);

	IritPolygon *getNextPolygon();

	void setNextPolygon(IritPolygon *polygon);

	/* Draws an polygon (draw lines between each of its points).
	 * Each of the points is multiplied by a transformation matrix.
	 * @pDCToUse - a pointer to the the DC with which the
	 *				polygon is drawn
	 * @transformation - the matrix which multiplies each point
	*/
	// TODO: The coordinates aren't adjusted to screen view and therefore
	//			cannot be drawn properly on screen. The drawing function needs
	//			therefore be adjusted.
	void draw(CDC *pDCToUse, struct State state);

	// Operators overriding
	IritPolygon &operator++();
};

/* This class represents an object in the IRIT world. An object is formed from
 * a list of polygons (each represented by its vertices)
 */
class IritObject {
	int m_polygons_nr;
	IritPolygon *m_polygons;
	IritPolygon *m_iterator;

public:
	IritObject();
	
	~IritObject();

	/* Received a pointer to a new polygon, and adds this polygon to the list
	 * of polygons in the object. The polygon is always added as the last
	 * polygon.
	 * @polygon - a pointer to the polygon to add
	 */
	void addPolygonP(IritPolygon *polygon);

	/* Creates an empty polygon and returns a pointer to it.
	 * the polygon is added to the list of polygons of the object
	 * as the last polygon.
	 * returns null pointer on memory failure
	 */
	IritPolygon *createPolygon();

	/* Draws an object (each of its polygons at a time). Each
	 * of the points of the object are multiplied by a transformation
	 * matrix.
	 * @pDCToUse - a pointer to the the DC with which the
	 *				object is drawn
	 * @transformation - the transformation matrix, by default the identity
	*/
	void draw(CDC *pDCToUse, struct State state);
};

class IritWorld {
	// TODO: rearrange public and private params.
	int m_objects_nr;
	IritObject **m_objects_arr;

	void drawFrame(CDC *pDCToUse);

public:

	// World state
	struct State state;

	// Bounding frame params
	Vector max_bound_coord,
		   min_bound_coord;

	COLORREF bg_color,
			 wire_color,
			 frame_color;

	IritWorld();

	IritWorld(Vector axes[NUM_OF_AXES], Vector &axes_origin);

	~IritWorld();

	/* Sets a coordinate system (with axes and origin point)
	 * @axes[3] - three orthonormal vectors in world space
	 * @axes_origin - origin point in world space
	*/
	void setScreenMat(Vector axes[NUM_OF_AXES], Vector &axes_origin, int screen_widht, int screen_height);

	/* Creates an empty object and returns a pointer to it.
	 * the object is added to the list of objects in the IritWorld.
	 * It is added as the last object
	 * return NULL on memory allocation error
	*/
	IritObject *createObject();

	/* Receieves a pointer to an object and adds the object
	 * the objects' list.
	 * @p_object - a pointer to the object that needs to be added
	 * returns false on allocation error and true otherwise
	*/
	bool addObjectP(IritObject *p_object);

	bool isEmpty();

	void draw(CDC *pDCToUse);
};