#include "glgeometry.h"
#include "glrectangle.h"
#include "glutil.h"
#include "glmath.h"

#include <math.h>

using namespace grinliz;
using namespace glm;

static const float EPSILON = 0.00001f;

void Plane::Init3Points(const glm::vec3* p)
{
	glm::vec3 v = glm::cross(p[1] - p[0], p[2] - p[0]);
	normal = glm::normalize(v);
	d = glm::dot(normal, p[0]);
}

bool Frustum::Contains(const glm::vec3 point) const
{
	for (int i = 0; i < nPlanes; ++i) {
		if (!PointPositiveOfPlane(plane[i], point))
			return false;
	}
	return true;
}

bool Frustum::Intersects(const Sphere& sphere) const
{
	for (int i = 0; i < nPlanes; ++i) {
		int c = IntersectPlaneSphere(plane[i], sphere);
		if (c < 0) return false;
	}
	return true;
}

void Frustum::FromEye(const glm::vec3* p)
{
	enum { EYE, UL, UR, LR, LL};

	glm::vec3 center = (p[LL] + p[UR]) * 0.5f;
	plane[0].normal = glm::normalize(center - p[0]);
	plane[0].d = -glm::length(p[EYE]);

	plane[1].Init3Points(p[EYE], p[UL], p[UR]);	// top		(normal inward)
	plane[2].Init3Points(p[EYE], p[UR], p[LR]); // right
	plane[3].Init3Points(p[EYE], p[LR], p[LL]); // bottom
	plane[4].Init3Points(p[EYE], p[LL], p[UL]); // left
	plane[5].Init3Points(p[LR], p[UR], p[UL]);  // far
	nPlanes = 6;
}


void Frustum::FromLine(const glm::vec3& dir, const glm::vec3& p)
{
	glm::vec3 right(0, 0, 0);
	if (fabsf(dir.x) > 0.3f)
		right.y = 1;
	else
		right.x = 1;

	vec3 normalA = glm::cross(dir, right);
	vec3 normalB = glm::cross(dir, normalA);
	plane[0].normal = normalA;
	plane[0].d = glm::dot(normalA, p);
	plane[1].normal = -normalA;
	plane[1].d = glm::dot(-normalA, p);
	plane[2].normal = normalB;
	plane[2].d = glm::dot(normalB, p);
	plane[3].normal = -normalB;
	plane[3].d = glm::dot(-normalB, p);
	nPlanes = 4;
}

void Frustum::Expand(const glm::vec3& p)
{
	for (int i = 0; i < nPlanes; ++i) {
		float d = PointToPlaneDistance(plane[i], p);
		if (d < 0) {
			plane[i].d = glm::dot(plane[i].normal, p);
		}
	}
}

bool grinliz::IntersectAABB(const glm::vec3& origin, const glm::vec3& dir, const Rect3F& aabb)
{
	float tMin = 0.0f;
	float tMax = FLT_MAX;

	vec3 aabbMin = aabb.Lower();
	vec3 aabbMax = aabb.Upper();

	for (int i = 0; i < 3; ++i) {
		float inv = 1.0f / dir[i];
		float t0 = (aabbMin[i] - origin[i]) * inv;
		float t1 = (aabbMax[i] - origin[i]) * inv;
		if (inv < 0.0f) {
			Swap(t0, t1);
		}

		tMin = Max(t0, tMin);
		tMax = Min(t1, tMax);

		// Has to be less that - not less equal - for flat plates.
		if (tMax < tMin)
			return false;
	}
	return true;
}

bool grinliz::IntersectAABB(const glm::vec3& origin, const glm::vec3& dir, const Rect3F& aabb, float* t)
{
	bool inside = true;
	vec3 minB = aabb.Lower();
	vec3 maxB = aabb.Upper();

	vec3 maxT(-1, -1, -1);
	vec3 coord;

	// Find candidate planes.
	for (int i = 0; i < 3; i++) {
		if (origin[i] < minB[i]) {
			coord[i] = minB[i];
			inside = false;

			if (dir[i] != 0.0f)
				maxT[i] = (minB[i] - origin[i]) / dir[i];
		}
		else if (origin[i] > maxB[i]) {
			coord[i] = maxB[i];
			inside = false;

			if (dir[i] != 0.0f)
				maxT[i] = (maxB[i] - origin[i]) / dir[i];
		}
	}

	// Ray origin inside bounding box
	if (inside)	{
		*t = 0;
		return true;
	}

	// Get largest of the maxT's for final choice of intersection
	int whichPlane = 0;
	if (maxT[1] > maxT[whichPlane])	whichPlane = 1;
	if (maxT[2] > maxT[whichPlane])	whichPlane = 2;

	// Check final candidate actually inside box
	if (maxT[whichPlane] < 0.0f)
		return false;

	for (int i = 0; i < 3; i++)	{
		if (i != whichPlane) {
			coord[i] = origin[i] + maxT[whichPlane] * dir[i];
			if (coord[i] < minB[i] || coord[i] > maxB[i])	
				return false;
		}
	}
	*t = maxT[whichPlane];
	return true;
}


bool grinliz::IntersectTriangle(
	const vec3& orig, const vec3& dir,
	const vec3& v0, const vec3& v1, const vec3& v2,
	float* t, bool cull)
{
	vec3 v0v1 = v1 - v0;
	vec3 v0v2 = v2 - v0;
	vec3 pvec = glm::cross(dir, v0v2);
	float det = glm::dot(v0v1, pvec);

	if (cull) {
		// if the determinant is negative the triangle is backfacing
		// if the determinant is close to 0, the ray misses the triangle
		if (det < EPSILON) return false;
	}
	else {
		// ray and triangle are parallel if det is close to 0
		if (fabsf(det) < EPSILON) return false;
	}
	float invDet = 1.0f / det;

	vec3 tvec = orig - v0;
	float u = glm::dot(tvec, pvec) * invDet;
	if (u < 0 || u > 1) return false;

	vec3 qvec = glm::cross(tvec, v0v1);
	float v = glm::dot(dir, qvec) * invDet;
	if (v < 0 || u + v > 1) return false;

	*t = glm::dot(v0v2, qvec) * invDet;
	return true;
}


int grinliz::IntersectPlaneAABB(const Plane& plane, const Rect3F& aabb)
{
	vec3 inner = aabb.pos;
	vec3 outer = aabb.Upper();

	for (int i = 0; i < 3; ++i) {
		if (plane.normal[i] < 0) grinliz::Swap(inner[i], outer[i]);
	}
	if (PointPositiveOfPlane(plane, inner))
		return 1;
	if (!PointPositiveOfPlane(plane, outer))
		return -1;
	return 0;
}


bool grinliz::IntersectRayPlane(const Plane& plane, const glm::vec3& origin, const glm::vec3& dir, float* t)
{
	// At the intersection, the dotproduct from the point to the plane is normal.
	float denom = glm::dot(dir, plane.normal);
	if (fabsf(denom) < EPSILON)
		return false;

	float num = plane.d - glm::dot(plane.normal, origin);
	*t = num / denom;

	return *t >= 0;
}


bool ExitAABB(const glm::vec3& origin, const glm::vec3 dir, const grinliz::Rect3F& aabb, float* t) 
{
	// Clumsy algorithm, but want to re-use existing code.
	int axis = MajorAxis(dir);

	float tExit = fabsf(aabb.size[axis] / dir[axis]);	// we will leave the aabb at this length or less.
	tExit += 1.0f;	// clear rounding, corners, etc.

	float tBack = 0;
	if (IntersectAABB(origin + dir * tExit, -dir, aabb, &tBack)) {
		*t = tExit - tBack;
		return true;
	}
	return false;
}


bool grinliz::SegmentInAABB(const glm::vec3& origin, const glm::vec3 dir, float length, const grinliz::Rect3F& aabb, glm::vec3* a, glm::vec3* b)
{
	const glm::vec3 dst = origin + dir * length;
	float t, dt;

	if (aabb.Contains(origin)) {
		*a = origin;
		*b = dst;
		if (!aabb.Contains(dst)) {
			if (ExitAABB(origin, dir, aabb, &t)) {
				*b = origin + dir * t;
			}
		}		
		return true;
	}

	if (IntersectAABB(origin, dir, aabb, &t) && t < length) {
		*a = *b = origin + dir * t;
		if (ExitAABB(*a, dir, aabb, &dt)) {
			float t2 = t + dt;
			if (t2 <= length)
				*b = origin + dir * t2;
			else
				*b = origin + dir * length;
		}
		return true;
	}

	return false;
}


float grinliz::PointToLineDistance(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b)
{
	// dist = || (a - p) X n || / || n ||
	glm::vec3 n = b - a;
	float num = glm::length(glm::cross((a - p), n));
	float den = glm::length(n);
	return num / den;
}


float grinliz::PointToSegmentDistance(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b)
{
	if (glm::dot(b - a, p - a) <= 0) {
		return glm::length(a - p);
	}
	else if (glm::dot(a - b, p - b) <= 0) {
		return glm::length(p - b);
	}
	return PointToLineDistance(p, a, b);
#if false
	float mid = PointToLineDistance(p, a, b);
	float aDist = glm::length(a - p);
	float bDist = glm::length(b - p);
	return grinliz::Min(mid, aDist, bDist);
#endif
}

void Frustum::Test()
{
	// at (0, 0, 10) looking to (0, 0, 0)
	// eyeDir=(0, 0, -1)
	glm::vec3 p[5] = {
		{ 0, 0, 10},	// eye
		{ -5, 5, 0 },	// UL
		{ 5, 5, 0},		// UR
		{ 5, -5, 0},	// LR
		{ -5, -5, 0}	// LL
	};
	{
		Frustum f;
		f.FromEye(p);

		// Check the plains
		GLASSERT(Equal(f.plane[0].normal, glm::vec3(0, 0, -1), 0.01f));
		GLASSERT(glm::dot(f.plane[1].normal, glm::vec3(0, -1, 0)) > 0);
		GLASSERT(glm::dot(f.plane[2].normal, glm::vec3(-1, 0, 0)) > 0);
		GLASSERT(glm::dot(f.plane[3].normal, glm::vec3(0, 1, 0)) > 0);
		GLASSERT(glm::dot(f.plane[4].normal, glm::vec3(1, 0, 0)) > 0);
		GLASSERT(glm::dot(f.plane[5].normal, glm::vec3(0, 0, 1)) > 0);

		glm::vec3 a(0, 0, 5);
		GLASSERT(Equal(5.0f, PointToPlaneDistance(f.plane[0], a), 0.01f));
		GLASSERT(Equal(5.0f, PointToPlaneDistance(f.plane[5], a), 0.01f));

		GLASSERT(Equal(2.24f, PointToPlaneDistance(f.plane[1], a), 0.1f));
		GLASSERT(Equal(2.24f, PointToPlaneDistance(f.plane[2], a), 0.1f));
		GLASSERT(Equal(2.24f, PointToPlaneDistance(f.plane[3], a), 0.1f));
		GLASSERT(Equal(2.24f, PointToPlaneDistance(f.plane[4], a), 0.1f));

		glm::vec3 b(0, 0, 15);
		GLASSERT(Equal(-5.0f, PointToPlaneDistance(f.plane[0], b), 0.01f));

		// Points
		GLASSERT(f.Contains(glm::vec3(0, 0, 5)));
		GLASSERT(!f.Contains(glm::vec3(0, 0, -5)));

		GLASSERT(!f.Contains(glm::vec3(10, 0, 5)));
		GLASSERT(!f.Contains(glm::vec3(-10, 0, 5)));
		GLASSERT(!f.Contains(glm::vec3(0, 10, 5)));
		GLASSERT(!f.Contains(glm::vec3(0, -10, 5)));

		// Sphere
		grinliz::Sphere s = { 2.0f, glm::vec3(0, 0, 10) };
		GLASSERT(f.Intersects(s));
		s.origin = glm::vec3(0, 0, 9);
		GLASSERT(f.Intersects(s));
		s.origin = glm::vec3(0, 0, 11);
		GLASSERT(f.Intersects(s));
		s.origin = glm::vec3(0, 0, 13);
		GLASSERT(!f.Intersects(s));

		s.origin = glm::vec3(6, 0, 0);
		GLASSERT(f.Intersects(s));
		s.origin = glm::vec3(8, 0, 0);
		GLASSERT(!f.Intersects(s));
		s.origin = glm::vec3(0, 0, 0);
		GLASSERT(f.Intersects(s));
	}
	{
		Frustum f;
		glm::vec3 dir(0, 1, 0);
		f.FromLine(dir, glm::vec3(2, 0, 0));
		// expand out to (0,0) to (4,4)
		f.Expand(glm::vec3(0, -2, 0));
		f.Expand(glm::vec3(3, 1, 4));

		GLASSERT(!f.Contains(glm::vec3(-1, 0, 0)));
		GLASSERT(f.Contains(glm::vec3(1, 3, 1)));
		GLASSERT(!f.Contains(glm::vec3(5, -3, 5)));
	}
}


void grinliz::TestIntersect()
{
	{
		Rect3F aabb;
		aabb.pos = vec3(0, 0, 0);
		aabb.size = vec3(1, 1, 1);

		bool hit = IntersectAABB(vec3(10, 0.5f, 0), vec3(-1, 0, 0), aabb);
		GLASSERT(hit);

		hit = IntersectAABB(vec3(0, 0.5f, -10), vec3(0, 0, 1), aabb);
		GLASSERT(hit);

		hit = IntersectAABB(vec3(10, 1.5f, 0), vec3(-1, 0, 0), aabb);
		GLASSERT(!hit);
	}
	{
		Plane plane;
		plane.d = 10.0f;
		plane.normal = vec3(0, 1, 0);

		GLASSERT(PointPositiveOfPlane(plane, vec3(0, 10.0f, 0)));
		GLASSERT(PointPositiveOfPlane(plane, vec3(1, 11.0f, 1)));
		GLASSERT(!PointPositiveOfPlane(plane, vec3(-1, 9.0f, 1)));
	}
	{
		Plane plane;
		plane.d = 10.0f;
		plane.normal = vec3(0, -1, 0);

		Rect3F neg({ 0, 0, 0 }, { 1, 1, 1 }),
			zero({ -1, -11, -1 }, { 2, 2, 2 }),
			pos({ 2, -20, 2 }, { 1, 1, 1 });

		GLASSERT(IntersectPlaneAABB(plane, neg) < 0);
		GLASSERT(IntersectPlaneAABB(plane, zero) == 0);
		GLASSERT(IntersectPlaneAABB(plane, pos) > 0);
	}

	{
		Plane plane;
		plane.d = SQRT2OVER2;
		plane.normal = vec3(SQRT2OVER2, 0.0f, SQRT2OVER2);

		float t = 0;
		(void)t;

		GLASSERT(IntersectRayPlane(plane, vec3(0, 0, 0), vec3(1, 0, 0), &t));
		GLASSERT(Equals(t, 1.0f));
		GLASSERT(IntersectRayPlane(plane, vec3(0, 0, 0), vec3(0, 0, 1), &t));
		GLASSERT(Equals(t, 1.0f));
		GLASSERT(!IntersectRayPlane(plane, vec3(0.5f, 0, 0), vec3(0, -1, 0), &t));
	}

	{
		Rect3F aabb({ -1, -1, -1 }, { 2, 2, 2 });
		vec3 a, b;

		// Outside, plenty of length.
		bool hit = SegmentInAABB(vec3(2, 0, 0), vec3(-1, 0, 0), 10.0f, aabb, &a, &b);
		GLASSERT(hit);
		GLASSERT(Equals(a, vec3(1, 0, 0)));
		GLASSERT(Equals(b, vec3(-1, 0, 0)));

		// Outside, Length in middle
		hit = SegmentInAABB(vec3(2, 0, 0), vec3(-1, 0, 0), 2.0f, aabb, &a, &b);
		GLASSERT(hit);
		GLASSERT(Equals(a, vec3(1, 0, 0)));
		GLASSERT(Equals(b, vec3(0, 0, 0)));

		// Outside, up short
		hit = SegmentInAABB(vec3(2, 0, 0), vec3(-1, 0, 0), 0.9f, aabb, &a, &b);
		GLASSERT(!hit);

		// Miss
		hit = SegmentInAABB(vec3(2, 0, 2), vec3(-1, 0, 0), 0.9f, aabb, &a, &b);
		GLASSERT(!hit);

		// Both inside
		hit = SegmentInAABB(vec3(0.5f, 0, 0), vec3(-1, 0, 0), 1.0f, aabb, &a, &b);
		GLASSERT(hit);
		GLASSERT(Equals(a, vec3(0.5f, 0, 0)));
		GLASSERT(Equals(b, vec3(-0.5f, 0, 0)));

		// One inside
		hit = SegmentInAABB(vec3(0.5f, 0, 0), vec3(-1, 0, 0), 10.0f, aabb, &a, &b);
		GLASSERT(hit);
		GLASSERT(Equals(a, vec3(0.5f, 0, 0)));
		GLASSERT(Equals(b, vec3(-1.0f, 0, 0)));
	}
}


