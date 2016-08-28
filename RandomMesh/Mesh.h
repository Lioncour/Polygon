#pragma once

#include "Content\ShaderStructures.h"

namespace RandomMesh
{
	struct VertexDummy
	{
		VertexDummy() {}
		VertexDummy(float x, float y, float z, float phi, float theta)
			: Position(x,y,z), Phi(phi), Theta(theta)
		{}

		DirectX::XMFLOAT3 Position;

		float Phi;
		float Theta;
	};

	class Mesh
	{
	private:

	public:
		Mesh(uint32_t vertexNum);
		std::vector<MeshVertex> GetVertexes();
	};
}