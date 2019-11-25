#pragma once
#include <afxwin.h>
#include <assert.h>
#include <iritprsr.h>
#include "Vector.h"
#include "Matrix.h"

enum {
	X_AXIS,
	Y_AXIS,
	Z_AXIS,
	AXES_NUM,
};
struct IritPoint {
	Vector vertex;
	Vector normal;

	struct IritPoint *next_point;
};

class IritPolygon {
	int m_point_nr;
	struct IritPoint *m_points;
	IritPolygon *m_next_polygon;

public:
	IritPolygon() : m_point_nr(0), m_points(nullptr), m_next_polygon(nullptr) {
	}

	~IritPolygon() {
		struct IritPoint *next_point;
		while (m_points) {
			next_point = m_points->next_point;
			delete m_points;
			m_points = next_point;
		}
	}
	
	bool addPoint(struct IritPoint &point) {

		IritPoint *new_point = new IritPoint(point);
		if (!new_point)
			return false;
		new_point->next_point = nullptr;

		m_point_nr++;

		if (!m_points) {
			// first point
			m_points = new_point;
			return true;
		}

		IritPoint *last_point = m_points;
		while (last_point->next_point)
			last_point = last_point->next_point;
		last_point->next_point = new_point;

		return true;
	}

	/* Creates an homogenious with a normal */
	bool addPoint(double &x, double &y, double &z, double &normal_x, double &normal_y,
				  double &normal_z) {
		IritPoint new_point;
		new_point.vertex = Vector(x, y, z, 1);
		new_point.normal = Vector(normal_x, normal_y, normal_z, 1);

		return addPoint(new_point);
	}

	bool addPoint(IPVertexStruct *vertex) {
		IritPoint new_point;

		for (int i = 0; i < 3; i++) {
			new_point.vertex[i] = vertex->Coord[i];
			new_point.normal[i] = vertex->Coord[i];
		}

		// Homogeneous component
		new_point.vertex[3] = 1;
		new_point.normal[3] = 1;

		return addPoint(new_point);
	}

	IritPolygon *getNextPolygon() {
		return m_next_polygon;
	}

	void setNextPolygon(IritPolygon *polygon) {
		m_next_polygon = polygon;
	}

	/* Draws an polygon (draw lines between each of its points).
	 * Each of the points is multiplied by a transformation matrix.
	 * @pDCToUse - a pointer to the the DC with which the
	 *				polygon is drawn
	 * @transformation - the matrix which multiplies each point
	*/
	// TODO: The coordinates aren't adjusted to screen view and therefore
	//			cannot be drawn properly on screen. The drawing function needs
	//			therefore be adjusted.
	void draw(CDC *pDCToUse, Matrix &transformation) {
		struct IritPoint *current_point = m_points;
		Vector vertex = current_point->vertex * 30;

		/* "Draw" first point */
		vertex = transformation * vertex;
		pDCToUse->MoveTo((int)floor(vertex[0]), (int)floor(vertex[1]));
		current_point = current_point->next_point;

		/* Draw shape's lines */
		while (current_point) {
			vertex = current_point->vertex * 30;
			vertex = transformation * vertex;

			pDCToUse->LineTo((int)floor(vertex[0]), (int)floor(vertex[1]));
			current_point = current_point->next_point;
		}

		/* Draw last line from last vertex to first one */
		vertex = m_points->vertex * 30;
		vertex = transformation * vertex;

		pDCToUse->LineTo((int)floor(vertex[0]), (int)floor(vertex[1]));

		// TODO: draw normals
	}

	// Operators overriding
	IritPolygon &operator++() {
		return *m_next_polygon;
	}
};

/* This class represents an object in the IRIT world. An object is formed from
 * a list of polygons (each represented by its vertices)
 */
class IritObject {
	int m_polygons_nr;
	IritPolygon *m_polygons;
	IritPolygon *m_iterator;

public:
	IritObject() : m_polygons_nr(0), m_polygons(nullptr), m_iterator(nullptr) {
	}
	
	~IritObject() {
		IritPolygon *next_polygon;
		while (m_polygons) {
			next_polygon = m_polygons->getNextPolygon();
			delete m_polygons;
			m_polygons = next_polygon;
		}
	}

	/* Received a pointer to a new polygon, and adds this polygon to the list
	 * of polygons in the object. The polygon is always added as the last
	 * polygon.
	 * @polygon - a pointer to the polygon to add
	 */
	void addPolygonP(IritPolygon *polygon) {

		if (!m_polygons) {
			m_polygons = polygon;
		}
		else {
			if (!m_iterator)
				m_iterator = m_polygons;

			while (m_iterator->getNextPolygon())
				m_iterator = m_iterator->getNextPolygon();
			m_iterator->setNextPolygon(polygon);
		}
		m_polygons_nr++;
	}

	/* Creates an empty polygon and returns a pointer to it.
	 * the polygon is added to the list of polygons of the object
	 * as the last polygon.
	 * returns null pointer on memory failure
	 */
	IritPolygon *createPolygon() {
		IritPolygon *new_polygon = new IritPolygon();
		if (!new_polygon)
			return nullptr;
		addPolygonP(new_polygon);
		return new_polygon;
	}

	/* Draws an object (each of its polygons at a time). Each
	 * of the points of the object are multiplied by a transformation
	 * matrix.
	 * @pDCToUse - a pointer to the the DC with which the
	 *				object is drawn
	 * @transformation - the transformation matrix, by default the identity
	*/
	void draw(CDC *pDCToUse, Matrix &transformation = Matrix::Identity()) {
		m_iterator = m_polygons;
		while (m_iterator) {
			m_iterator->draw(pDCToUse, transformation);
			m_iterator = m_iterator->getNextPolygon();
		}
	}
};

class IritWorld {
	int m_objects_nr;
	IritObject **m_objects_arr;

	/* Scene arguments */
	Vector m_axes[3];
	Vector m_axes_origin;

public:
	IritWorld() : m_objects_nr(0), m_objects_arr(nullptr) {
	}

	IritWorld(Vector axes[AXES_NUM], Vector &axes_origin) : m_objects_nr(0), m_objects_arr(nullptr) {
		for (int i = 0; i < 3; i++)
			m_axes[i] = axes[i];
		m_axes_origin = axes_origin;
	}

	~IritWorld() {
		delete[] m_objects_arr;
	}

	/* Sets a coordinate system (with axes and origin point)
	 * @axes[3] - three orthonormal vectors in world space
	 * @axes_origin - origin point in world space
	*/
	void setSceneCoordinateSystem(Vector axes[AXES_NUM], Vector &axes_origin) {
		for (int i = 0; i < 3; i++)
			m_axes[i] = axes[i];
		m_axes_origin = axes_origin;
	}

	/* Creates an empty object and returns a pointer to it.
	 * the object is added to the list of objects in the IritWorld.
	 * It is added as the last object
	 * return NULL on memory allocation error
	*/
	IritObject *createObject() {
		IritObject *new_object = new IritObject();
		if (!new_object)
			return nullptr;

		if (!addObjectP(new_object))
			return nullptr;
		
		return new_object;
	}

	/* Receieves a pointer to an object and adds the object
	 * the objects' list.
	 * @p_object - a pointer to the object that needs to be added
	 * returns false on allocation error and true otherwise
	*/
	bool addObjectP(IritObject *p_object) {

		IritObject **new_objects_array = new IritObject*[m_objects_nr + 1];
		if (!new_objects_array)
			return false;

		for (int i = 0; i < m_objects_nr; i++)
			new_objects_array[i] = m_objects_arr[i];
		new_objects_array[m_objects_nr] = p_object;

		delete m_objects_arr;
		m_objects_arr = new_objects_array;
		m_objects_nr++;

		return true;
	}

	bool isEmpty() {
		return m_objects_nr == 0;
	}

	void draw(CDC *pDCToUse);
};