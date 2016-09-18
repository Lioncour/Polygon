#pragma once

#include <string>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

#include "pch.h"
#include "SpriteBatch.h"
#include "SimpleMath.h"
#include "ScrollingBackground.h"

using namespace std;
using Microsoft::WRL::ComPtr;

namespace RandomMesh
{
	// Renders the current FPS value in the bottom right corner of the screen using Direct2D and DirectWrite.
	class BackhroundRenderer
	{
	public:
		BackhroundRenderer(const shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();

	private:
		shared_ptr<DX::DeviceResources> m_deviceResources;
		
		CD3D11_TEXTURE2D_DESC catDesc;

		ComPtr<ID3D11InputLayout> m_inputLayout;
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_indexBuffer;
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;
		ComPtr<ID3D11Buffer> m_constantBuffer;
		ComPtr<ID3D11RasterizerState> m_rastarizerState;

		ComPtr<ID3D11SamplerState> m_backgroundSampler;
		ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;

		BackgroundConstantBuffer m_constantBufferData;

		bool m_loadingComplete;

		float m_backgroundWidth;
		float m_backgroundHeight;
	};
}