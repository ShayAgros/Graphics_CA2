#pragma once
#include <afxwin.h>
#include <assert.h>
#include <iritprsr.h>
#include "Vector.h"
#include "Matrix.h"

// The color scheme here is    <B G R *reserved*>
#define BG_DEFAULT_COLOR		{0, 0, 0, 0}       // Black
#define WIRE_DEFAULT_COLOR		{128, 128, 128, 0} // Grey
#define FRAME_DEFAULT_COLOR		{0, 0, 255, 0}     // Red
#define IRIT_NORMAL_COLOR		{0, 255, 255, 0}   // Yellow
#define CALC_NORMAL_COLOR		{255, 0, 0, 0}     // Blue
#define NORMAL_DEFAULT_COLOR	{255, 255, 255, 0} // White

#define DEFAULT_PROJECTION_PLANE_DISTANCE 20
#define DEAULT_VIEW_PARAMETERS 0, 0, -20
#define DEFAULT_FINENESS 20.0

#define RGB_TO_RGBQUAD(x) {(BYTE)((x & 0xff0000) >> 16), (BYTE)((x & 0xff00) >> 8), (BYTE)(x & 0xff), 0}

// Declerations
/* Creates a view matrix which simulates changing the position of the camera
 * @x,y,z - camera coordinates in *world* view system
 * returns a matrix which maps every object to view space
*/
Matrix createViewMatrix(double x, double y, double z);

// this enum prob isnt needed, beacuse of built in axis info - m_nAxis
enum Axis {
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	NUM_OF_AXES
};

class IritPolygon;

struct IritPoint {
	Vector vertex;
	Vector normal;

	bool is_irit_normal;

	struct IritPoint *next_point;
};

struct State {
	bool show_vertex_normal;
	bool show_polygon_normal;
	bool object_frame;
	bool is_perspective_view;
	bool object_transform;
	bool is_default_color;
	bool tell_normals_apart;

	double projection_plane_distance;
	double sensitivity;
	double fineness;

	bool is_axis_active[3];

	Matrix ratio_mat;
	Matrix coord_mat;
	Matrix center_mat;
	Matrix world_mat;
	Matrix object_mat;
	Matrix ortho_mat;
	Matrix view_mat;

	Matrix perspective_mat;
	Matrix screen_mat;

	RGBQUAD wire_color;
	RGBQUAD frame_color;
	RGBQUAD bg_color;
	RGBQUAD normal_color;
};

struct PolygonList {
	IPPolygonStruct *skel_polygon;
	IritPolygon *polygon;
	PolygonList *next;
};

struct VertexList {
	IPVertexStruct *vertex;
	PolygonList *polygon_list;
	VertexList *next;
};

class IritPolygon {
	int m_point_nr;
	struct IritPoint *m_points;

	IritPolygon *m_next_polygon;

public:
	Vector normal_start;
	Vector normal_end;
	bool is_irit_normal;

	IritPolygon();

	~IritPolygon();
	
	bool addPoint(struct IritPoint &point);

	/* Creates an homogenious with a normal */
	bool addPoint(double &x, double &y, double &z, double &normal_x, double &normal_y,
		double &normal_z);

	bool addPoint(IPVertexStruct *vertex, bool is_irit_normal, Vector normal);

	IritPolygon *getNextPolygon();

	void setNextPolygon(IritPolygon *polygon);

	/* Draws an polygon (draw lines between each of its points).
	 * Each of the points is multiplied by a transformation matrix.
	 * @pDCToUse - a pointer to the the DC with which the
	 *				polygon is drawn
	 * @state - world state (current coordinate system, scaling function
	 *					 etc.)
	 * @normal_transform - a transformation matrix for the normal vectors
	 * @vertex_transform - a transformation matrix for the the vertices (each
	 *						vertex is multiplied by this matrix before being drawn
	*/
	void draw(int *bitmap, int width, int height, RGBQUAD color, struct State state,
			  Matrix &vertex_transform);

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
	RGBQUAD object_color;

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
	 * @state - world state (current coordinate system, scaling function
	 *					 etc.)
	 * @normal_transform - a transformation matrix for the normal vectors
	 * @vertex_transform - a transformation matrix for the the vertices (each
	 *						vertex is multiplied by this matrix before being drawn
	*/
	void draw(int *bitmap, int width, int height, struct State state,
			  Matrix &vertex_transform);
};

/* This class represents an Irit Figure which is build from many objects, which have many
 * polygons, which have many dots. It can be described like this:
 *	Figure -> Object
 *			  Object
 *			  Object ->
 *					Polygon
 *					Polygon
 *					Polygon ->
 *						Point
 *						Point
*/
class IritFigure {
	int m_objects_nr;
	IritObject **m_objects_arr;

	void drawFrame(int *bitmap, int width, int height, struct State state, Matrix &transform);

public:

	Matrix world_mat;
	Matrix object_mat;

	Matrix backup_transformation_matrix;

	// Bounding frame params
	Vector max_bound_coord,
		min_bound_coord;

	IritFigure();

	~IritFigure();

	/* Saves the current object or world transformation matrix
	 * (depending on what is being modified at the moment).
	 * This is to save the original location when changing the figure's
	 * position using the mouse
	 */
	void backup_transformation(State &state);

	/* Receieves a pointer to an object and adds the object
	 * the objects' list.
	 * @p_object - a pointer to the object that needs to be added
	 * returns false on allocation error and true otherwise
	*/
	bool addObjectP(IritObject *p_object);

	/* Creates an empty object and returns a pointer to it.
	 * the object is added to the list of objects in the IritWorld.
	 * It is added as the last object
	 * return NULL on memory allocation error
	*/
	IritObject *createObject();

	void draw(int *bitmap, int width, int height, Matrix transform, State &state);

	bool isEmpty();
};

class IritWorld {
	int m_figures_nr;
	IritFigure **m_figures_arr;

	/* Returns the perspective matrix */
	Matrix getPerspectiveMatrix(const double &angleOfView, const double &near_z, const double &far_z);

	/* Creates a projection matrix */
	Matrix createProjectionMatrix();

	/* Project a point to screen space using the given transformation and
	 * the world's screen matrix.
	 */
	Vector projectPoint(Vector &td_point, Matrix &transformation);

public:

	// World state
	struct State state;

	// Bounding frame params
	Vector max_bound_coord,
		   min_bound_coord;

	IritWorld();

	IritWorld(Vector axes[NUM_OF_AXES], Vector &axes_origin);

	~IritWorld();

	/* Sets a coordinate system (with axes and origin point)
	 * @axes[3] - three orthonormal vectors in world space
	 * @axes_origin - origin point in world space
	*/
	void setScreenMat(Vector axes[NUM_OF_AXES], Vector &axes_origin, int screen_widht, int screen_height);

	void setOrthoMat(void);


	/* Creates an empty figure and returns a pointer to it.
	 * the figure is added to the list of figures in the IritWorld.
	 * It is added as the last object
	 * return NULL on memory allocation error
	*/
	IritFigure *createFigure();

	/* Receieves a pointer to a figure and adds the figure
	 * the figures' list.
	 * @p_figure - a pointer to the figure that needs to be added
	 * returns false on allocation error and true otherwise
	*/
	bool addFigureP(IritFigure *p_figure);

	/* Returns a reference to the last figure in the figures list */
	IritFigure &getLastFigure();

	/* Checks whether the point is inside the bounding box of one of the
	 * figures. If so, choose the first figure that matches.
	 * If the point isn't inside a figure's bounding box, return NULL
	 */
	IritFigure *getFigureInPoint(CPoint &point);

	bool isEmpty();

	void draw(int *bitmap, int width, int height);
};