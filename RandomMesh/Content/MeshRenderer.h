#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "Mesh.h"

namespace RandomMesh
{
	// This sample renderer instantiates a basic rendering pipeline.
	class MeshRenderer
	{
	public:
		MeshRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking(float x, float y);
		void TrackingUpdate(float x, float y);
		void StopTracking();
		bool IsTracking() { return m_tracking; }		
		void SetMesh(unique_ptr<Mesh> mesh);

	private:
		void Rotate(float rotationX, float rotationY);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		unique_ptr<Mesh> m_mesh;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_blackPixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rastarizerState;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireRastarizerState;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32_t	m_indexCount;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		
		float	m_degreesPerSecond;		
		
		bool	m_tracking;
		
		float	m_baseTrackingX;
		float	m_baseTrackingY;
		
		float	m_rotationX;
		float	m_rotationY;		
	};
}

