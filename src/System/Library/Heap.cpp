#include "define.h"
#include "Heap.h"

#pragma region RTVHeap

int RTVHeap::CreateRTV(ID3D12Resource* pBuffer)
{
	if (m_useCount < m_nextRegistNumber)
	{
		assert(0 && "確保済みのヒープ領域を超えました。");
		return 0;
	}

	auto handle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (UINT64)m_nextRegistNumber * m_incrementSize;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	DEV->CreateRenderTargetView(pBuffer, &rtvDesc, handle);

	return m_nextRegistNumber++;
}

#pragma endregion

#pragma region DSVHeap

int DSVHeap::CreateDSV(ID3D12Resource* pBuffer, DXGI_FORMAT format)
{
	if (m_useCount < m_nextRegistNumber)
	{
		assert(0 && "確保済みのヒープ領域を超えました。");
		return 0;
	}

	auto handle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += (UINT64)m_nextRegistNumber * m_incrementSize;

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format        = format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	DEV->CreateDepthStencilView(pBuffer, &dsvDesc, handle);

	return m_nextRegistNumber++;
}

#pragma endregion

#pragma region CSUHeap

int CSUHeap::CreateSRV(ID3D12Resource* pBuffer)
{
	if (m_useCount.y < m_nextRegistNumber)
	{
		assert(0 && "確保済みのヒープ領域を超えました。");
		return 0;
	}

	auto handle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += ((UINT64)m_useCount.x + 1) * m_incrementSize + (UINT64)m_nextRegistNumber * m_incrementSize;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format                  = pBuffer->GetDesc().Format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension           = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels     = 1;

	if (pBuffer->GetDesc().Format == DXGI_FORMAT_R32_TYPELESS)
	{
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	}

	DEV->CreateShaderResourceView(pBuffer, &srvDesc, handle);

	return m_nextRegistNumber++;
}

const D3D12_GPU_DESCRIPTOR_HANDLE
CSUHeap::GetGPUHandle(int number)
{
	auto handle = m_pHeap->GetGPUDescriptorHandleForHeapStart();
	handle.ptr += (UINT64)m_incrementSize * ((UINT64)m_useCount.x + 1);
	handle.ptr += (UINT64)m_incrementSize * number;

	return handle;
}

void CSUHeap::SetHeap()
{
	ID3D12DescriptorHeap* ppHeaps[] = { m_pHeap.Get() };
	CMD->SetDescriptorHeaps(1, ppHeaps);
}

#pragma endregion
