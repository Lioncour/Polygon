#include "pch.h"
#include "MeshRenderer.h"
#include "Mesh.h"
#include "..\Common\DirectXHelper.h"

using namespace RandomMesh;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
MeshRenderer::MeshRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_degreesPerSecond(45),
	m_indexCount(0),
	m_tracking(false),
	m_deviceResources(deviceResources),
	m_rotationX(0.0),
	m_rotationY(0.0),
	m_baseTrackingX(0.0),
	m_baseTrackingY(0.0)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void MeshRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	/*if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}*/

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
		);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
		);

	// Eye is at (0,0.7,1.5), looking at point (0,-0.1,0) with the up-vector along the y-axis.
	static const XMVECTORF32 eye = { 0.0f, 0.7f, 1.5f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, 0.0f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void MeshRenderer::Update(DX::StepTimer const& timer)
{
	return;

	if (!m_tracking)
	{
		auto elapsed = timer.GetElapsedSeconds();

		float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
		m_rotationY += static_cast<float>(elapsed * radiansPerSecond);
				
		m_rotationY = static_cast<float>(fmod(m_rotationY, XM_2PI));

		Rotate(m_rotationX, m_rotationY);
	}
}

// Rotate the 3D cube model a set amount of radians.
void MeshRenderer::Rotate(float rotationX, float rotationY)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationX(rotationX) * XMMatrixRotationY(rotationY)));
}

void MeshRenderer::StartTracking(float x, float y)
{
	m_tracking = true;
	m_baseTrackingX = x;
	m_baseTrackingY = y;
}

// When tracking, the 3D cube can be rotated around its Y axis by tracking pointer position relative to the output screen width.
void MeshRenderer::TrackingUpdate(float x, float y)
{
	if (m_tracking)
	{
		auto deltaX = x - m_baseTrackingX;
		auto deltaY = y - m_baseTrackingY;

		auto size = fmin(m_deviceResources->GetOutputSize().Height, m_deviceResources->GetOutputSize().Width);

		m_rotationX += XM_2PI * 2.0f * deltaY / size;
		m_rotationY += XM_2PI * 2.0f * deltaX / size;

		m_rotationX = static_cast<float>(fmod(m_rotationX, XM_2PI));
		m_rotationY = static_cast<float>(fmod(m_rotationY, XM_2PI));

		Rotate(m_rotationX, m_rotationY);

		m_baseTrackingX = x;
		m_baseTrackingY = y;
	}
}

void MeshRenderer::StopTracking()
{
	m_tracking = false;
}

// Renders one frame using the vertex and pixel shaders.
void MeshRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(
		m_constantBuffer.Get(),
		0,
		NULL,
		&m_constantBufferData,
		0,
		0,
		0
		);

	// Each vertex is one instance of the VertexPositionColor struct.
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	context->IASetIndexBuffer(
		m_indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(m_inputLayout.Get());
	
	// Attach our vertex shader.
	context->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
		);

	// Attach our pixel shader.
	context->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	context->PSSetConstantBuffers1(
		0,
		1,
		m_constantBuffer.GetAddressOf(),
		nullptr,
		nullptr
	);

	context->RSSetState(m_rastarizerState.Get());

	// Draw the objects.
	context->DrawIndexed(
		m_indexCount,
		0,
		0
		);

	context->PSSetShader(m_blackPixelShader.Get(), nullptr, 0);
	context->RSSetState(m_wireRastarizerState.Get());

	context->DrawIndexed(m_indexCount, 0, 0);
}

void MeshRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");
	auto loadBlackPSTask = DX::ReadDataAsync(L"BlackPixelShader.cso");

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

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc [] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

		auto bufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 16) & ~0x0F;
		CD3D11_BUFFER_DESC constantBufferDesc(bufferSize, D3D11_BIND_CONSTANT_BUFFER);

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});

	auto createBlackPSTask = loadBlackPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_blackPixelShader
			)
		);
	});


	// Once both shaders are loaded, create the mesh.
	auto createCubeTask = (createPSTask && createVSTask && createBlackPSTask).then([this] () {

		RandomMesh::Mesh mesh(static_cast<size_t>(Random(10, 50)));

		m_constantBufferData.light = XMFLOAT3(Random(-1.0, 1.0), Random(-1.0, 1.0), Random(-1.0, 1.0));

		auto vertices = mesh.GetVertices();
		auto indices = mesh.GetIndices();

		D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
		vertexBufferData.pSysMem = vertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPositionColor) * vertices.size(), D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
				)
			);

		m_indexCount = indices.size();

		D3D11_SUBRESOURCE_DATA indexBufferData = {0};
		indexBufferData.pSysMem = indices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(sizeof(uint16_t) * indices.size(), D3D11_BIND_INDEX_BUFFER);
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

		D3D11_RASTERIZER_DESC wireDesc = {};
		wireDesc.FillMode = D3D11_FILL_WIREFRAME;
		wireDesc.CullMode = D3D11_CULL_BACK;
		wireDesc.DepthClipEnable = true;
		wireDesc.DepthBias = -500;		
						
		DX::ThrowIfFailed(m_deviceResources->GetD3DDevice()->CreateRasterizerState(&wireDesc, &m_wireRastarizerState));
	});

	// Once the cube is loaded, the object is ready to be rendered.
	createCubeTask.then([this] () {
		m_loadingComplete = true;
	});
}

void MeshRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBuffer.Reset();
	m_rastarizerState.Reset();
}