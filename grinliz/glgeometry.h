#pragma once
#include <glm/glm.hpp>
#include "glrectangle.h"	// which means a rectangle is "more primitive" than a sphere

namespace grinliz 
{
	struct Plane
	{
		float d;	// distance from origin
		glm::vec3 normal;

		void Init3Points(const glm::vec3* points);
		void Init3Points(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) {
			glm::vec3 arr[3] = { p0, p1, p2 };
			Init3Points(arr);
		}
	};

	struct Sphere
	{
		float r;
		glm::vec3 origin;

		inline static Sphere ToSphere(const Rect3F& r) {
			GLASSERT(r.size.length() > 0);
			Sphere s;
			s.origin = r.Center();
			s.r = glm::length((s.origin - r.Select(0, 0, 0)));
			return s;
		}
	};


	// Planes face inward.
	struct Frustum {
		static constexpr int MAX_PLANES = 6;
		int nPlanes = 0;

		Plane plane[MAX_PLANES];

		bool Contains(const glm::vec3 point) const;
		bool Intersects(const Sphere& sphere) const;

		// eye, UL, UR, LR, LL, far
		// creates: front, top, right, bottom, left, back
		void FromEye(const glm::vec3* p);
		void FromLine(const glm::vec3& dir, const glm::vec3& p);
		void Expand(const glm::vec3& p);

		static void Test();
	};

	inline bool Equals(float a, float b, float eps = 0.00001f) {
		return fabsf(a - b) < eps;
	}

	inline bool Equals(const glm::vec3& a, const glm::vec3& b, float eps = 0.00001f) {
		for (int i = 0; i < 3; ++i)
			if (fabsf(a[i] - b[i]) > eps)
				return false;
		return true;
	}

	inline bool Equals(const glm::vec4& a, const glm::vec4 b, float eps = 0.00001f) {
		for (int i = 0; i < 4; ++i)
			if (fabsf(a[i] - b[i]) > eps)
				return false;
		return true;
	}

	inline int MajorAxis(const glm::vec3& dir) {
		if (fabsf(dir.x) >= fabs(dir.y) && fabsf(dir.x) >= fabs(dir.z))
			return 0;
		if (fabsf(dir.y) >= fabsf(dir.z))
			return 1;
		return 2;
	}

	bool IntersectAABB(const glm::vec3& origin, const glm::vec3& dir, const Rect3F& aabb);

	// Possibly slightly slower - certainly more code. 
	// Returns intersection point. If origin is inside, this is considered an intersection at the origin.
	bool IntersectAABB(const glm::vec3& origin, const glm::vec3& dir, const Rect3F& aabb, float* t);

	bool IntersectTriangle(
		const glm::vec3& orig, const glm::vec3& dir,
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
		float* t, bool cull = true);

	// returns SIGNED distance
	inline float PointToPlaneDistance(const Plane& plane, const glm::vec3& p)
	{
		// This works out geometrically. Shockingly concise.
		return glm::dot(p, plane.normal) - plane.d;
	}

	// On plane is considered positive.
	inline bool PointPositiveOfPlane(const Plane& plane, const glm::vec3& p)
	{
		return PointToPlaneDistance(plane, p) >= 0;
	}

	// Returns  1 if the aabb is positive of the plane.
	//		   -1 if the aabb is negative of the plane
	//			0 if they intersect
	int IntersectPlaneAABB(const Plane& plane, const Rect3F& aabb);

	// -1, 0, 1
	inline int IntersectPlaneSphere(const Plane& plane, const Sphere& sphere) {
		float d = PointToPlaneDistance(plane, sphere.origin);
		if (d > sphere.r) return 1;
		if (d < -sphere.r) return -1;
		return 0;
	}

	// Returns true of the ray intersects the plane.
	bool IntersectRayPlane(const Plane& plane, const glm::vec3& origin, const glm::vec3& dir, float* t);

	bool SegmentInAABB(const glm::vec3& origin, const glm::vec3 dir, float length, const grinliz::Rect3F& aabb, glm::vec3* a, glm::vec3* b);

	float PointToSegmentDistance(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b);
	float PointToLineDistance(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b);

	void TestIntersect();
}
