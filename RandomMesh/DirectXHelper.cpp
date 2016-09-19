#include "pch.h"
#include "Common\DirectXHelper.h"

namespace DX
{
	void CreateBuffer(_In_ ID3D11Device* device, size_t bufferSize, D3D11_BIND_FLAG bindFlag, _Out_ ID3D11Buffer** pBuffer)
	{
		D3D11_BUFFER_DESC desc = { 0 };

		desc.ByteWidth = (UINT)bufferSize;
		desc.BindFlags = bindFlag;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, pBuffer));
	}
}