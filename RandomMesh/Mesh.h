#pragma once

#include "Content\ShaderStructures.h"

using namespace std;
using namespace DirectX;

namespace RandomMesh
{
	float Random(float maxValue);

	float Random(float minValue, float maxValue);

	bool Equals(const XMFLOAT3& first, const XMFLOAT3& second);

	bool static operator== (const XMFLOAT3& first, const XMFLOAT3& second)
	{
		return Equals(first, second);
	}

	float Distance(const XMFLOAT3& first, const XMFLOAT3& second);

	float Dot(const XMFLOAT3& first, const XMFLOAT3& second);

	struct VertexDummy
	{
		VertexDummy() {}
		VertexDummy(float x, float y, float z, float phi, float theta)
			: Position(x, y, z), Phi(phi), Theta(theta)
		{}

		XMFLOAT3 Position;		

		float Phi;
		float Theta;
	};

	struct XmFloat3Equal
	{	
		bool operator()(const XMFLOAT3& _Left, const XMFLOAT3& _Right) const
		{	
			return _Left == _Right;
		}
	};

	struct XmFloat3Hash
	{
		size_t operator()(const XMFLOAT3& value) const
		{
			hash<float> floatHasher;

			size_t hash = 2166136261U;

			hash ^= floatHasher(value.x);
			hash *= 16777619U;

			hash ^= floatHasher(value.y);
			hash *= 16777619U;

			hash ^= floatHasher(value.z);			

			return hash;
		}
	};

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

	class LineDummy
	{
	public:
		VertexDummy& Start, End;

		LineDummy(VertexDummy& start, VertexDummy& end)
			: Start(start), End(end)
		{
		}
	};

	class TriangleDummy
	{
	public:
		VertexDummy *V1, *V2, *V3;
		
		TriangleDummy()
			: V1(nullptr), V2(nullptr), V3(nullptr)
		{
		}

		TriangleDummy(VertexDummy* v1, VertexDummy* v2, VertexDummy* v3)
			: V1(v1), V2(v2), V3(v3)
		{
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

	class Mesh
	{
	private:
		vector<VertexPositionColor> _gradientVertexes;
		vector<VertexPositionColor> _solidVertexes;
		vector<uint16_t> _triangleIndices;

	public:
		Mesh(size_t vertexNum);
		vector<VertexPositionColor> GetVertices();
		vector<uint16_t> GetIndices();

	private:
		void Delaunay(const vector<VertexDummy>& seq, vector<Triangle>& triangles, vector<Line>& surfaceEdges);
		vector<TriangleDummy> Mesh::DoSmth(const vector<Triangle>& triangles, vector<VertexDummy>& vertexes);
		float RandomizeVertexes(vector<VertexDummy>& vertexes);
		XMFLOAT3 GetColor(VertexDummy& vertex, XMFLOAT3& color1, XMFLOAT3& color2, XMFLOAT3 color3, float averageRadius);
	};
}