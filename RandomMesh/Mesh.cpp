#include "pch.h"
#include "Mesh.h"
#include <chrono>

#include "DirectXCollision.h"

using namespace std;
using namespace std::chrono;
using namespace DirectX;

namespace RandomMesh
{
	float Random(float maxValue)
	{
		auto value = Windows::Security::Cryptography::CryptographicBuffer::GenerateRandomNumber();
		return (float)value / (numeric_limits<unsigned int>::max)() * maxValue;
	}

	float Random(float minValue, float maxValue)
	{
		auto value = Windows::Security::Cryptography::CryptographicBuffer::GenerateRandomNumber();

		return minValue + (float)value / (numeric_limits<unsigned int>::max)() * (maxValue - minValue);
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

		for (size_t i = 0; i < vertexNum; i++)
		{
			float r = 0.25f + Random(0.1);
			float phi = XMConvertToRadians(Random(-90, 90));
			float theta = XMConvertToRadians(Random(360));

			vertexes.push_back(VertexDummy(r*cos(phi)*cos(theta), r*sin(phi), r*cos(phi)*sin(theta), phi, theta));
		}

		vector<Triangle> triangles;
		vector<Line> edges;

		Delaunay(vertexes, triangles, edges);
		
		auto triangleDummies = CreateTrianglesMappedToSourceVertices(triangles, vertexes);
		auto averageRadius = RandomizeVertexes(vertexes);

		XMFLOAT3 color1(Random(1.0), Random(1.0), Random(1.0));
		XMFLOAT3 color2(Random(1.0), Random(1.0), Random(1.0));
		XMFLOAT3 color3(Random(1.0), Random(1.0), Random(1.0));

		/*XMFLOAT3 color1(1.0, 0.0, 0.0);
		XMFLOAT3 color2(0.0, 1.0, 0.0);
		XMFLOAT3 color3(0.0, 0.0, 1.0);*/

		for (auto triangle : triangleDummies)
		{
			auto normal = triangle.Normal;
			
			_triangleIndices.push_back(static_cast<uint16_t>(_gradientVertexes.size()));
			_gradientVertexes.push_back({ triangle.V1->Position, normal, GetColor(*triangle.V1, color1, color2, color3, averageRadius) });
			
			_triangleIndices.push_back(static_cast<uint16_t>(_gradientVertexes.size()));
			_gradientVertexes.push_back({ triangle.V2->Position, normal, GetColor(*triangle.V2, color1, color2, color3, averageRadius) });
			
			_triangleIndices.push_back(static_cast<uint16_t>(_gradientVertexes.size()));
			_gradientVertexes.push_back({ triangle.V3->Position, normal, GetColor(*triangle.V3, color1, color2, color3, averageRadius) });			
		}
	}

	std::vector<VertexPositionColor> Mesh::GetVertices()
	{
		return _gradientVertexes;
	}

	vector<uint16_t> Mesh::GetIndices()
	{
		return _triangleIndices;
	}

	vector<TriangleDummy> Mesh::CreateTrianglesMappedToSourceVertices(const vector<Triangle>& triangles, vector<VertexDummy>& vertexes)
	{
		vector<TriangleDummy> result;
		result.reserve(triangles.size());

		for (auto triangle : triangles)
		{
			auto v1 = triangle.v1;
			auto v2 = triangle.v2;
			auto v3 = triangle.v3;

			VertexDummy* vert1 = nullptr;
			VertexDummy* vert2 = nullptr;
			VertexDummy* vert3 = nullptr;

			for (auto& vertex : vertexes)
			{
				if (vertex.Position == v1)
				{
					vert1 = &vertex;
				}
				else if (vertex.Position == v2)
				{
					vert2 = &vertex;
				}
				else if (vertex.Position == v3)
				{
					vert3 = &vertex;
				}
			}

			result.push_back(TriangleDummy(vert1, vert2, vert3, triangle.GetNormal()));
		}

		return result;
	}

	float Mesh::RandomizeVertexes(vector<VertexDummy>& vertexes)
	{
		float sumRadius = 0;

		for (auto& vertex : vertexes)
		{
			float phi = vertex.Phi;
			float theta = vertex.Theta;

			auto r = Random(0.25, 0.5);
			vertex.Position = XMFLOAT3(r*cos(phi)*cos(theta), r*sin(phi), r*cos(phi)*sin(theta));

			sumRadius += r;
		}

		return sumRadius / vertexes.size();
	}

	float sq(float number)
	{
		return number*number;
	}

	XMFLOAT3 Mesh::GetColor(VertexDummy& vertex, XMFLOAT3& color1, XMFLOAT3& color2, XMFLOAT3 color3, float averageRadius)
	{
		auto x = vertex.Position.x;
		auto y = vertex.Position.y;

		float x_c1 = -averageRadius;
		float y_c1 = 0;
		float x_c2 = sin(XMConvertToRadians(30))*averageRadius;
		float y_c2 = cos(XMConvertToRadians(30))*averageRadius;
		float x_c3 = sin(XMConvertToRadians(30))*averageRadius;
		float y_c3 = -cos(XMConvertToRadians(30))*averageRadius;

		float d1 = pow(sqrt(sq(x_c1 - x) + sq(y_c1 - y)), 1);
		float d2 = pow(sqrt(sq(x_c2 - x) + sq(y_c2 - y)), 1);
		float d3 = pow(sqrt(sq(x_c3 - x) + sq(y_c3 - y)), 1);

		float r = (color1.x * d1 + color2.x * d2 + color3.x * d3) / (d1 + d2 + d3);
		float g = (color1.y * d1 + color2.y * d2 + color3.y * d3) / (d1 + d2 + d3);
		float b = (color1.z * d1 + color2.z * d2 + color3.z * d3) / (d1 + d2 + d3);
		return XMFLOAT3(r, g, b);
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
		v3.x = center.x + (float)sqrt(2) * r;
		v3.y = center.y - r;
		v3.z = center.z + (float)sqrt(6) * r;

		XMFLOAT3 v4;
		v4.x = center.x + (float)sqrt(2) * r;
		v4.y = center.y - r;
		v4.z = center.z - (float)sqrt(6) * r;

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

			for (size_t index = tetras.size(); index > 0; index--)
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

			for (size_t i = 0; i < newTList.size(); i++)
			{
				for (size_t j = i + 1; j < newTList.size(); j++)
				{
					if (newTList[i].Equals(newTList[j]))
					{
						isRedundancy[i] = true;
						isRedundancy[j] = true;
					}
				}
			}

			for (size_t i = 0; i < isRedundancy.size(); i++)
			{
				if (!isRedundancy[i])
				{
					tetras.push_back(newTList[i]);
				}
			}
		}

		for (auto index = tetras.size(); index > 0; index--)
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
		for (size_t i = 0; i < surfaceEdgeList.size(); i++)
		{
			for (size_t j = i + 1; j < surfaceEdgeList.size(); j++)
			{
				if (surfaceEdgeList[i].Equals(surfaceEdgeList[j]))
				{
					isRedundancy[j] = true;
				}
			}
		}

		for (size_t i = 0; i < isRedundancy.size(); i++)
		{
			if (!isRedundancy[i])
			{
				surfaceEdges.push_back(surfaceEdgeList[i]);
			}
		}
	}
}