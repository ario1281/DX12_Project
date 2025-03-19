#include "SquarePolygon.h"

void SquarePolygon::Init(float w, float h, DWORD color)
{
	// 頂点座標
	m_vertex[0].pos = { -w/2,-h/2,0 };
	m_vertex[1].pos = { -w/2, h/2,0 };
	m_vertex[2].pos = {  w/2,-h/2,0 };
	m_vertex[3].pos = {  w/2, h/2,0 };
	// 頂点色
	m_vertex[0].color = color;
	m_vertex[1].color = color;
	m_vertex[2].color = color;
	m_vertex[3].color = color;
	// UV座標
	m_vertex[0].uv = { 0,1 };
	m_vertex[1].uv = { 0,0 };
	m_vertex[2].uv = { 1,1 };
	m_vertex[3].uv = { 1,0 };

	//
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type                 = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width            = sizeof(m_vertex);
	desc.Height           = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels        = 1;
	desc.Format           = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.Flags            = D3D12_RESOURCE_FLAG_NONE;
	desc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	DEVICE->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pVBuffer)
	);

	VERTEX* vbMap = nullptr;
	{
		m_pVBuffer->Map(0, nullptr, (void**)&vbMap);
		std::copy(std::begin(m_vertex), std::end(m_vertex), vbMap);
		m_pVBuffer->Unmap(0, nullptr);
	}
}

void SquarePolygon::Draw()
{
	COMMAND->IASetVertexBuffers(0, 1, &m_vbView);

	COMMAND->DrawInstanced(3, 1, 0, 0);
}
