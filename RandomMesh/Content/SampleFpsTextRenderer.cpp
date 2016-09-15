#include "pch.h"
#include "SpriteBatch.h"
#include "SampleFpsTextRenderer.h"

#include "Common/DirectXHelper.h"

using namespace RandomMesh;
using namespace Microsoft::WRL;

SampleFpsTextRenderer::SampleFpsTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources),
	m_loadingComplete(false)	
{
	CreateDeviceDependentResources();

	m_constantBufferData.bias = 0;
	m_constantBufferData.ratio = 1;
}

void SampleFpsTextRenderer::Update(DX::StepTimer const& timer)
{
	auto time = timer.GetElapsedSeconds();

	m_constantBufferData.bias += time / 5;
	m_constantBufferData.bias = fmod(m_constantBufferData.bias, 2.0f);
}

void SampleFpsTextRenderer::Render()
{
	if (m_loadingComplete == false)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	auto outputSize = m_deviceResources->GetOutputSize();
	
	auto backgroundRatio = m_backgroundWidth / m_backgroundHeight;
	auto resultWidth = outputSize.Height * backgroundRatio;
	auto texRatio = outputSize.Width / resultWidth;

	m_constantBufferData.ratio = texRatio;

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(BackgroundVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	context->PSSetShaderResources(0, 1, m_backgroundTexture.GetAddressOf());
	context->PSSetSamplers(0, 1, m_backgroundSampler.GetAddressOf());

	context->RSSetState(m_rastarizerState.Get());

	// Draw the objects.
	context->DrawIndexed(6, 0, 0);
}

void SampleFpsTextRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"BackgroundVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"BackgroundPixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
	});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		auto bufferSize = (sizeof(BackgroundConstantBuffer) + 16) & ~0x0F;
		CD3D11_BUFFER_DESC constantBufferDesc(bufferSize, D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
	});

	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask).then([this]()
	{
		using namespace DirectX;

		static const BackgroundVertex vertices[] =
		{
			{ XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT2(0,0) },
			{ XMFLOAT3(1.0f, 1.0f,  0.0f), XMFLOAT2(1,0) },
			{ XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT2(1,1) },
			{ XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT2(0,1) }
		};

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = vertices;
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(vertices), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);

		static const unsigned short cubeIndices[] =
		{
			0,1,3,
			1,2,3,
		};

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = cubeIndices;
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(cubeIndices), D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBuffer
			)
		);

		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.DepthClipEnable = true;

		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&desc, &m_rastarizerState));
	});

	ComPtr<ID3D11Resource> resource;
	DX::ThrowIfFailed(DirectX::CreateWICTextureFromFile(m_deviceResources->GetD3DDevice(), L"Assets\\background.png", resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf()));
	
	ComPtr<ID3D11Texture2D> cat;
	DX::ThrowIfFailed(resource.As(&cat));

	cat->GetDesc(&catDesc);

	m_backgroundWidth = catDesc.Width;
	m_backgroundHeight = catDesc.Height;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));

	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	sampDesc.BorderColor[0] = DirectX::Colors::Brown.f[0];
	sampDesc.BorderColor[1] = DirectX::Colors::Brown.f[1];
	sampDesc.BorderColor[2] = DirectX::Colors::Brown.f[2];
	sampDesc.BorderColor[3] = DirectX::Colors::Brown.f[3];

	DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateSamplerState(&sampDesc, &m_backgroundSampler));

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this]() {
		m_loadingComplete = true;
	});
}

void SampleFpsTextRenderer::ReleaseDeviceDependentResources()
{
	m_backgroundTexture.Reset();

	m_loadingComplete = false;
}