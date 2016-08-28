#pragma once

#include "Content\ShaderStructures.h"

using namespace std;
using namespace DirectX;

namespace RandomMesh
{
	struct VertexDummy
	{
		VertexDummy() {}
		VertexDummy(float x, float y, float z, float phi, float theta)
			: Position(x, y, z), Phi(phi), Theta(theta)
		{}

		DirectX::XMFLOAT3 Position;

		float Phi;
		float Theta;
	};

	class Mesh
	{
	private:

	public:
		Mesh(size_t vertexNum);
		//std::vector<MeshVertex> GetVertexes();
		void Delaunay(vector<VertexDummy>& seq);
	};

	/*class Delaunay {

		List<PVector> vertices;
		List<Tetrahedron> tetras;

		public List<Line> edges;

		public List<Line> surfaceEdges;
		public List<Triangle> triangles;


		public Delaunay() {
			vertices = new CopyOnWriteArrayList<PVector>();
			tetras = new CopyOnWriteArrayList<Tetrahedron>();
			edges = new CopyOnWriteArrayList<Line>();
			surfaceEdges = new CopyOnWriteArrayList<Line>();
			triangles = new CopyOnWriteArrayList<Triangle>();
		}

		public void SetData(List<PVector> seq) {

			tetras.clear();
			edges.clear();

			PVector vMax = new PVector(-999, -999, -999);
			PVector vMin = new PVector(999, 999, 999);
			for (PVector v : seq) {
				if (vMax.x < v.x) vMax.x = v.x;
				if (vMax.y < v.y) vMax.y = v.y;
				if (vMax.z < v.z) vMax.z = v.z;
				if (vMin.x > v.x) vMin.x = v.x;
				if (vMin.y > v.y) vMin.y = v.y;
				if (vMin.z > v.z) vMin.z = v.z;
			}

			PVector center = new PVector();
			center.x = 0.5f * (vMax.x - vMin.x);
			center.y = 0.5f * (vMax.y - vMin.y);
			center.z = 0.5f * (vMax.z - vMin.z);
			float r = -1;
			for (PVector v : seq) {
				if (r < PVector.dist(center, v)) r = PVector.dist(center, v);
			}
			r += 0.1f;


			PVector v1 = new PVector();
			v1.x = center.x;
			v1.y = center.y + 3.0f*r;
			v1.z = center.z;

			PVector v2 = new PVector();
			v2.x = center.x - 2.0f*(float)Math.sqrt(2)*r;
			v2.y = center.y - r;
			v2.z = center.z;

			PVector v3 = new PVector();
			v3.x = center.x + (float)Math.sqrt(2)*r;
			v3.y = center.y - r;
			v3.z = center.z + (float)Math.sqrt(6)*r;

			PVector v4 = new PVector();
			v4.x = center.x + (float)Math.sqrt(2)*r;
			v4.y = center.y - r;
			v4.z = center.z - (float)Math.sqrt(6)*r;

			PVector[] outer = { v1, v2, v3, v4 };
			tetras.add(new Tetrahedron(v1, v2, v3, v4));


			ArrayList<Tetrahedron> tmpTList = new ArrayList<Tetrahedron>();
			ArrayList<Tetrahedron> newTList = new ArrayList<Tetrahedron>();
			ArrayList<Tetrahedron> removeTList = new ArrayList<Tetrahedron>();
			for (PVector v : seq) {
				tmpTList.clear();
				newTList.clear();
				removeTList.clear();
				for (Tetrahedron t : tetras) {
					if ((t.o != null) && (t.r > PVector.dist(v, t.o))) {
						tmpTList.add(t);
					}
				}

				for (Tetrahedron t1 : tmpTList) {

					tetras.remove(t1);

					v1 = t1.vertices[0];
					v2 = t1.vertices[1];
					v3 = t1.vertices[2];
					v4 = t1.vertices[3];
					newTList.add(new Tetrahedron(v1, v2, v3, v));
					newTList.add(new Tetrahedron(v1, v2, v4, v));
					newTList.add(new Tetrahedron(v1, v3, v4, v));
					newTList.add(new Tetrahedron(v2, v3, v4, v));
				}

				boolean[] isRedundancy = new boolean[newTList.size()];
				for (int i = 0; i < isRedundancy.length; i++) isRedundancy[i] = false;
				for (int i = 0; i < newTList.size() - 1; i++) {
					for (int j = i + 1; j < newTList.size(); j++) {
						if (newTList.get(i).equals(newTList.get(j))) {
							isRedundancy[i] = isRedundancy[j] = true;
						}
					}
				}
				for (int i = 0; i < isRedundancy.length; i++) {
					if (!isRedundancy[i]) {
						tetras.add(newTList.get(i));
					}

				}

			}


			boolean isOuter = false;
			for (Tetrahedron t4 : tetras) {
				isOuter = false;
				for (PVector p1 : t4.vertices) {
					for (PVector p2 : outer) {
						if (p1.x == p2.x && p1.y == p2.y && p1.z == p2.z) {
							isOuter = true;
						}
					}
				}
				if (isOuter) {
					tetras.remove(t4);
				}
			}

			triangles.clear();
			boolean isSame = false;
			for (Tetrahedron t : tetras) {
				for (Line l1 : t.getLines()) {
					isSame = false;
					for (Line l2 : edges) {
						if (l2.equals(l1)) {
							isSame = true;
							break;
						}
					}
					if (!isSame) {
						edges.add(l1);
					}
				}
			}




			ArrayList<Triangle> triList = new ArrayList<Triangle>();
			for (Tetrahedron t : tetras) {
				v1 = t.vertices[0];
				v2 = t.vertices[1];
				v3 = t.vertices[2];
				v4 = t.vertices[3];

				Triangle tri1 = new Triangle(v1, v2, v3);
				Triangle tri2 = new Triangle(v1, v3, v4);
				Triangle tri3 = new Triangle(v1, v4, v2);
				Triangle tri4 = new Triangle(v4, v3, v2);

				PVector n;

				n = tri1.getNormal();
				if (n.dot(v1) > n.dot(v4)) tri1.turnBack();

				n = tri2.getNormal();
				if (n.dot(v1) > n.dot(v2)) tri2.turnBack();

				n = tri3.getNormal();
				if (n.dot(v1) > n.dot(v3)) tri3.turnBack();

				n = tri4.getNormal();
				if (n.dot(v2) > n.dot(v1)) tri4.turnBack();

				triList.add(tri1);
				triList.add(tri2);
				triList.add(tri3);
				triList.add(tri4);
			}
			boolean[] isSameTriangle = new boolean[triList.size()];
			for (int i = 0; i < triList.size() - 1; i++) {
				for (int j = i + 1; j < triList.size(); j++) {
					if (triList.get(i).equals(triList.get(j))) isSameTriangle[i] = isSameTriangle[j] = true;
				}
			}
			for (int i = 0; i < isSameTriangle.length; i++) {
				if (!isSameTriangle[i]) triangles.add(triList.get(i));
			}

			surfaceEdges.clear();
			ArrayList<Line> surfaceEdgeList = new ArrayList<Line>();
			for (Triangle tri : triangles) {
				surfaceEdgeList.addAll(Arrays.asList(tri.getLines()));
			}
			boolean[] isRedundancy = new boolean[surfaceEdgeList.size()];
			for (int i = 0; i < surfaceEdgeList.size() - 1; i++) {
				for (int j = i + 1; j < surfaceEdgeList.size(); j++) {
					if (surfaceEdgeList.get(i).equals(surfaceEdgeList.get(j))) isRedundancy[j] = true;
				}
			}

			for (int i = 0; i < isRedundancy.length; i++) {
				if (!isRedundancy[i]) surfaceEdges.add(surfaceEdgeList.get(i));
			}

		}
	}*/

	float Random(float maxValue);

	float Random(float minValue, float maxValue);

	bool Equals(const XMFLOAT3& first, const XMFLOAT3& second);

	float Distance(const XMFLOAT3& first, const XMFLOAT3& second);

	float Dot(const XMFLOAT3& first, const XMFLOAT3& second);

	class Line
	{
	public:
		XMFLOAT3 Start, End;

		Line(const XMFLOAT3& start, const XMFLOAT3& end)
		{
			Start = start;
			End = end;
		}

		void Reverse()
		{
			XMFLOAT3 tmp = Start;
			Start = End;
			End = tmp;
		}

		bool operator==(const Line& other) const
		{
			return Equals(other);
		}

		bool Equals(const Line& other) const
		{
			if (RandomMesh::Equals(Start, other.Start) && RandomMesh::Equals(End, other.End))
			{
				return true;
			}

			if (RandomMesh::Equals(Start, other.End) && RandomMesh::Equals(End, other.Start))
			{
				return true;
			}

			return false;
		}
	};

	class Triangle
	{
	public:
		XMFLOAT3 v1, v2, v3;

		Triangle(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3)
		{
			this->v1 = v1;
			this->v2 = v2;
			this->v3 = v3;
		}

		XMFLOAT3 GetNormal() const
		{
			XMVECTOR edge1 = XMVectorSet(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z, 0.0f);
			XMVECTOR edge2 = XMVectorSet(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z, 0.0f);

			auto cross = XMVector3Normalize(XMVector3Cross(edge1, edge2));

			XMFLOAT3 normal;
			XMStoreFloat3(&normal, cross);

			return normal;
		}

		void TurnBack()
		{
			XMFLOAT3 tmp = v3;
			v3 = v1;
			v1 = tmp;
		}

		std::vector<Line> GetLines() const
		{
			vector<Line> lines;

			lines.push_back(Line(v1, v2));
			lines.push_back(Line(v2, v3));
			lines.push_back(Line(v3, v1));

			return lines;
		}

		bool operator==(const Triangle& other) const
		{
			return Equals(other);
		}

		bool Equals(const Triangle& other) const
		{
			auto lines1 = this->GetLines();
			auto lines2 = other.GetLines();

			int cnt = 0;
			for (auto line1 : lines1)
			{
				for (auto line2 : lines2)
				{
					if (line1.Equals(line2))
					{
						cnt++;
						break;
					}
				}
			}

			return cnt == 3;
		}
	};

	class Tetrahedron
	{
	public:
		vector<XMFLOAT3> vertexes;
		XMFLOAT3 o;
		float r;
		bool isCorrect;

	public:
		Tetrahedron(vector<XMFLOAT3> v)
		{
			vertexes = v;
			this->CalculateCenterCircumcircle();
		}

		Tetrahedron(const XMFLOAT3& v1, const XMFLOAT3& v2, const XMFLOAT3& v3, const XMFLOAT3& v4)
		{
			vertexes.push_back(v1);
			vertexes.push_back(v2);
			vertexes.push_back(v3);
			vertexes.push_back(v4);

			CalculateCenterCircumcircle();
		}

		bool operator==(const Tetrahedron& other) const
		{
			return Equals(other);
		}

		bool Equals(const Tetrahedron& other) const
		{
			int count = 0;
			for (auto p1 : vertexes)
			{
				for (auto p2 : other.vertexes)
				{
					if (RandomMesh::Equals(p1, p2))
					{
						count++;
						break;
					}
				}
			}

			return count == 4;
		}

		vector<Line> GetLines()
		{
			XMFLOAT3 v1 = vertexes[0];
			XMFLOAT3 v2 = vertexes[1];
			XMFLOAT3 v3 = vertexes[2];
			XMFLOAT3 v4 = vertexes[3];

			vector<Line> lines;

			lines.push_back(Line(v1, v2));
			lines.push_back(Line(v1, v3));
			lines.push_back(Line(v1, v4));
			lines.push_back(Line(v2, v3));
			lines.push_back(Line(v2, v4));
			lines.push_back(Line(v3, v4));

			return lines;
		}

	private:

		void CalculateCenterCircumcircle()
		{
			XMFLOAT3 v1 = vertexes[0];
			XMFLOAT3 v2 = vertexes[1];
			XMFLOAT3 v3 = vertexes[2];
			XMFLOAT3 v4 = vertexes[3];

			double A[3][3] = {
				{ v2.x - v1.x, v2.y - v1.y, v2.z - v1.z },
				{ v3.x - v1.x, v3.y - v1.y, v3.z - v1.z },
				{ v4.x - v1.x, v4.y - v1.y, v4.z - v1.z }
			};

			double b[] = {
				0.5 * (v2.x*v2.x - v1.x*v1.x + v2.y*v2.y - v1.y*v1.y + v2.z*v2.z - v1.z*v1.z),
				0.5 * (v3.x*v3.x - v1.x*v1.x + v3.y*v3.y - v1.y*v1.y + v3.z*v3.z - v1.z*v1.z),
				0.5 * (v4.x*v4.x - v1.x*v1.x + v4.y*v4.y - v1.y*v1.y + v4.z*v4.z - v1.z*v1.z)
			};

			double x[3];

			isCorrect = gauss(A, b, x) != 0;

			if (!isCorrect)
			{
				o = XMFLOAT3();
				r = -1;
			}
			else
			{
				o = XMFLOAT3((float)x[0], (float)x[1], (float)x[2]);
				r = Distance(o, v1);
			}
		}

		double lu(double a[3][3], int ip[])
		{
			auto n = 3;
			double weight[3];

			for (int k = 0; k < n; k++)
			{
				ip[k] = k;
				double u = 0;

				for (int j = 0; j < n; j++)
				{
					double t = abs(a[k][j]);
					if (t > u)
					{
						u = t;
					}
				}

				if (u == 0)
				{
					return 0;
				}

				weight[k] = 1 / u;
			}

			double det = 1;
			for (int k = 0; k < n; k++)
			{
				double u = -1;
				int m = 0;
				for (int i = k; i < n; i++)
				{
					int ii = ip[i];
					double t = abs(a[ii][k]) * weight[ii];
					if (t > u)
					{
						u = t;
						m = i;
					}
				}

				int ik = ip[m];
				if (m != k)
				{
					ip[m] = ip[k]; ip[k] = ik;
					det = -det;
				}

				u = a[ik][k];
				det *= u;

				if (u == 0)
				{
					return 0;
				}

				for (int i = k + 1; i < n; i++)
				{
					int ii = ip[i];
					double t = (a[ii][k] /= u);

					for (int j = k + 1; j < n; j++)
					{
						a[ii][j] -= t * a[ik][j];
					}
				}
			}

			return det;
		}

		void solve(double a[3][3], double b[3], int ip[3], double x[3])
		{
			auto n = 3;

			for (int i = 0; i < n; i++)
			{
				int ii = ip[i];
				double t = b[ii];

				for (int j = 0; j < i; j++)
				{
					t -= a[ii][j] * x[j];
				}

				x[i] = t;
			}

			for (int i = n - 1; i >= 0; i--)
			{
				double t = x[i];
				int ii = ip[i];

				for (int j = i + 1; j < n; j++)
				{
					t -= a[ii][j] * x[j];
				}

				x[i] = t / a[ii][i];
			}
		}

		double gauss(double a[3][3], double b[3], double x[3])
		{
			auto n = 3;

			int ip[3];
			double det = lu(a, ip);

			if (det != 0)
			{
				solve(a, b, ip, x);
			}

			return det;
		}
	};
}