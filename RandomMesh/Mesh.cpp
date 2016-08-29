#include "pch.h"
#include "Mesh.h"

#include "DirectXCollision.h"

using namespace std;
using namespace DirectX;

namespace RandomMesh
{
	float Random(float maxValue)
	{
		return (float)rand() / RAND_MAX * maxValue;
	}

	float Random(float minValue, float maxValue)
	{
		return minValue + (float)rand() / RAND_MAX * (maxValue - minValue);
	}

	bool Equals(const XMFLOAT3& first, const XMFLOAT3& second)
	{
		return first.x == second.x
			&& first.y == second.y
			&& first.z == second.z;
	}

	float Distance(const XMFLOAT3& first, const XMFLOAT3& second)
	{
		XMVECTOR vector1 = XMLoadFloat3(&first);
		XMVECTOR vector2 = XMLoadFloat3(&second);
		XMVECTOR vectorSub = XMVectorSubtract(vector1, vector2);
		XMVECTOR length = XMVector3Length(vectorSub);

		float distance;
		XMStoreFloat(&distance, length);

		return distance;
	}

	float Dot(const XMFLOAT3& first, const XMFLOAT3& second)
	{
		XMVECTOR vector1 = XMLoadFloat3(&first);
		XMVECTOR vector2 = XMLoadFloat3(&second);

		float dot;
		XMStoreFloat(&dot, XMVector3Dot(vector1, vector2));

		return dot;
	}

	Mesh::Mesh(size_t vertexNum)
	{
		auto vertexes = vector<VertexDummy>();
		vertexes.reserve(vertexNum);

		// I decided to use Delaunay triangularization for creating triangles from cloud of random points on a sphere - this ensures us that resulting figure makes sense 
		//Delaunay d = new Delaunay();
		//vertexes = new ArrayList<PVector>();
		//float[][] vertexAngles = new float[vertexNum][2];
		//	
		//// here are created random points. They are laying on sphere. 
		for (size_t i = 0; i < vertexNum; i++) {
			// we add some small random number to radius in order to avoid computational bugs 
			float r = 0.25;
			float phi = DirectX::XMConvertToRadians(Random(-90, 90));
			float theta = DirectX::XMConvertToRadians(Random(360));

			vertexes.push_back(VertexDummy(r*cos(phi)*cos(theta), r*sin(phi), r*cos(phi)*sin(theta), phi, theta));
		}

		vertexes.clear();

		vertexes.push_back(VertexDummy(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(-0.5f,  0.5f, -0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(-0.5f,  0.5f,  0.5f, 0.0f, 0.0f));
		/*vertexes.push_back(VertexDummy(0.5f, -0.5f, -0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(0.5f, -0.5f,  0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(0.5f,  0.5f, -0.5f, 0.0f, 0.0f));
		vertexes.push_back(VertexDummy(0.5f,  0.5f,  0.5f, 0.0f, 0.0f));*/

		Delaunay(vertexes);

		auto a = 1;

		//// here Delaunay triangularization take place 
		//// I use external code to perform it
		//d.SetData(vertexes);

		//// after creating list of points we map achieved triangles to saved points so each triangle vertex is saved by reference to each vertex rather than absolute position
		//// this enables us quick movement of vertex without changes in triangles lists 
		//mapTriangles(d);
		//// than we randomize vertexes changing only the radius of each vertex. That way it's sure that resulting figure makes sense. 
		//randomizeVertexes(vertexAngles);
	}

	void Mesh::Delaunay(const vector<VertexDummy>& seq, vector<Triangle>& triangles, vector<Line>& surfaceEdges)
	{
		vector<Tetrahedron> tetras;
		vector<Line> edges;

		XMFLOAT3 vMax(-999, -999, -999);
		XMFLOAT3 vMin(999, 999, 999);

		for (auto v : seq)
		{
			if (vMax.x < v.Position.x)
			{
				vMax.x = v.Position.x;
			}

			if (vMax.y < v.Position.y)
			{
				vMax.y = v.Position.y;
			}

			if (vMax.z < v.Position.z)
			{
				vMax.z = v.Position.z;
			}

			if (vMin.x > v.Position.x)
			{
				vMin.x = v.Position.x;
			}

			if (vMin.y > v.Position.y)
			{
				vMin.y = v.Position.y;
			}

			if (vMin.z > v.Position.z)
			{
				vMin.z = v.Position.z;
			}
		}

		XMFLOAT3 center;
		center.x = vMin.x + 0.5f * (vMax.x - vMin.x);
		center.y = vMin.y + 0.5f * (vMax.y - vMin.y);
		center.z = vMin.z + 0.5f * (vMax.z - vMin.z);

		vector<XMFLOAT3> points;

		float r = -1;
		for (auto v : seq)
		{
			points.push_back(v.Position);

			auto distance = Distance(center, v.Position);
			if (r < distance)
			{
				r = distance;
			}
		}
		r += 0.01f;

		XMFLOAT3 v1;
		v1.x = center.x;
		v1.y = center.y + 3.0f * r;
		v1.z = center.z;

		XMFLOAT3 v2;
		v2.x = center.x - 2.0f * (float)sqrt(2) * r;
		v2.y = center.y - r;
		v2.z = center.z;

		XMFLOAT3 v3;
		v3.x = center.x + (float)sqrt(2)*r;
		v3.y = center.y - r;
		v3.z = center.z + (float)sqrt(6)*r;

		XMFLOAT3 v4;
		v4.x = center.x + (float)sqrt(2)*r;
		v4.y = center.y - r;
		v4.z = center.z - (float)sqrt(6)*r;

		XMFLOAT3 outer[] = { v1, v2, v3, v4 };
		tetras.push_back(Tetrahedron(v1, v2, v3, v4));

		vector<Tetrahedron> tmpTList;
		vector<Tetrahedron> newTList;
		vector<Tetrahedron> removeTList;

		for (auto v : seq)
		{
			tmpTList.clear();
			newTList.clear();
			removeTList.clear();

			for(size_t index = tetras.size(); index > 0; index--)
			{
				auto t = tetras[index - 1];
				if (t.isCorrect && t.r > Distance(v.Position, t.o))
				{
					tetras.erase(tetras.begin() + (index - 1));
					tmpTList.push_back(t);
				}
			}

			for (Tetrahedron t1 : tmpTList)
			{
				v1 = t1.vertexes[0];
				v2 = t1.vertexes[1];
				v3 = t1.vertexes[2];
				v4 = t1.vertexes[3];

				newTList.push_back(Tetrahedron(v1, v2, v3, v.Position));
				newTList.push_back(Tetrahedron(v1, v2, v4, v.Position));
				newTList.push_back(Tetrahedron(v1, v3, v4, v.Position));
				newTList.push_back(Tetrahedron(v2, v3, v4, v.Position));
			}

			vector<bool> isRedundancy(newTList.size(), false);

			for (int i = 0; i < newTList.size(); i++)
			{
				for (int j = i + 1; j < newTList.size(); j++)
				{
					if (newTList[i].Equals(newTList[j]))
					{
						isRedundancy[i] = true;
						isRedundancy[j] = true;
					}
				}
			}

			for (int i = 0; i < isRedundancy.size(); i++)
			{
				if (!isRedundancy[i])
				{
					tetras.push_back(newTList[i]);
				}
			}
		}

		for(auto index = tetras.size(); index > 0; index--)
		{
			auto t4 = tetras[index - 1];
			auto isOuter = false;

			for (XMFLOAT3 p1 : t4.vertexes)
			{
				for (XMFLOAT3 p2 : outer)
				{
					if (Equals(p1, p2))
					{
						isOuter = true;
						break;
					}
				}

				if (isOuter)
				{
					break;
				}
			}

			if (isOuter)
			{
				tetras.erase(tetras.begin() + (index - 1));
			}
		}

		triangles.clear();

		for (Tetrahedron t : tetras)
		{
			for (Line l1 : t.GetLines())
			{
				auto isSame = false;
				for (Line l2 : edges)
				{
					if (l2.Equals(l1))
					{
						isSame = true;
						break;
					}
				}

				if (!isSame)
				{
					edges.push_back(l1);
				}
			}
		}

		vector<Triangle> triList;
		for (Tetrahedron t : tetras)
		{
			v1 = t.vertexes[0];
			v2 = t.vertexes[1];
			v3 = t.vertexes[2];
			v4 = t.vertexes[3];

			Triangle tri1(v1, v2, v3);
			Triangle tri2(v1, v3, v4);
			Triangle tri3(v1, v4, v2);
			Triangle tri4(v4, v3, v2);

			XMFLOAT3 n;

			n = tri1.GetNormal();
			if (Dot(n, v1) > Dot(n, v4))
			{
				tri1.TurnBack();
			}

			n = tri2.GetNormal();
			if (Dot(n, v1) > Dot(n, v2))
			{
				tri2.TurnBack();
			}

			n = tri3.GetNormal();
			if (Dot(n, v1) > Dot(n, v3))
			{
				tri3.TurnBack();
			}

			n = tri4.GetNormal();
			if (Dot(n, v2) > Dot(n, v1))
			{
				tri4.TurnBack();
			}

			triList.push_back(tri1);
			triList.push_back(tri2);
			triList.push_back(tri3);
			triList.push_back(tri4);
		}

		vector<bool> isSameTriangle(triList.size(), false);
		for (int i = 0; i < triList.size(); i++)
		{
			for (int j = i + 1; j < triList.size(); j++)
			{
				if (triList[i].Equals(triList[j]))
				{
					isSameTriangle[i] = isSameTriangle[j] = true;
				}
			}
		}

		for (int i = 0; i < isSameTriangle.size(); i++)
		{
			if (!isSameTriangle[i])
			{
				triangles.push_back(triList[i]);
			}
		}

		surfaceEdges.clear();
		vector<Line> surfaceEdgeList;
		for (Triangle tri : triangles)
		{
			auto lines = tri.GetLines();
			surfaceEdgeList.insert(surfaceEdgeList.end(), lines.begin(), lines.end());
		}

		vector<bool> isRedundancy(surfaceEdgeList.size(), false);
		for (int i = 0; i < surfaceEdgeList.size(); i++)
		{
			for (int j = i + 1; j < surfaceEdgeList.size(); j++)
			{
				if (surfaceEdgeList[i].Equals(surfaceEdgeList[j]))
				{
					isRedundancy[j] = true;
				}
			}
		}

		for (int i = 0; i < isRedundancy.size(); i++)
		{
			if (!isRedundancy[i])
			{
				surfaceEdges.push_back(surfaceEdgeList[i]);
			}
		}
	}
}