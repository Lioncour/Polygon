﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "Mesh.h"

namespace RandomMesh
{
	__declspec(align(16)) class MeshRenderer
	{
	public:
		MeshRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void StartTracking(float x, float y, float zAngle, float scale);
		void TrackingUpdate(float x, float y, float zAngle, float scale);
		void StopTracking();
		bool IsTracking() { return m_tracking; }		
		void SetMesh(shared_ptr<Mesh> mesh);

		void* operator new(size_t request)
		{
			return _aligned_malloc(request, 16);
		}

		void operator delete(void * ptr)
		{
			_aligned_free(ptr);
		}

	private:
		void Transform(float rotationX, float rotationY, float rotationZ, float scale);

	private:
		__declspec(align(16)) XMVECTOR m_lastRotation;

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		shared_ptr<Mesh> m_mesh;

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
		float	m_baseZAngle;
		float	m_baseScale;

		float	m_scale;		
	};
}

