#include "pch.h"
#include "Mesh.h"

using namespace std;

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

	Mesh::Mesh(size_t vertexNum)
	{
		auto vertexes = make_shared<vector<VertexDummy>>();
		vertexes->reserve(vertexNum);
				
		// I decided to use Delaunay triangularization for creating triangles from cloud of random points on a sphere - this ensures us that resulting figure makes sense 
		//Delaunay d = new Delaunay();
		//vertexes = new ArrayList<PVector>();
		//float[][] vertexAngles = new float[vertexNum][2];
		//	
		//// here are created random points. They are laying on sphere. 
		for (int i = 0; i < vertexNum; i++) {
			// we add some small random number to radius in order to avoid computational bugs 
			float r = 0.25;
			float phi = DirectX::XMConvertToRadians(Random(-90, 90));
			float theta = DirectX::XMConvertToRadians(Random(360));
			
			vertexes->push_back(VertexDummy(r*cos(phi)*cos(theta), r*sin(phi), r*cos(phi)*sin(theta), phi, theta));
		}

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
}