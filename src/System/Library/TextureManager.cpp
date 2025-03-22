#include "TextureManager.h"

bool TextureManager::Load(const std::wstring &filename)
{
	HRESULT hr; 

	// WICテクスチャロード
	TexMetadata  _metaData;
	ScratchImage _scratchImg;
	hr = LoadFromWICFile(filename.c_str(), WIC_FLAGS_NONE, &_metaData, _scratchImg);
	if (FAILED(hr)) { return false; }

	m_img = *_scratchImg.GetImage(0, 0, 0);

	// 矩形の長さセット
	SetRect(&m_rect, 0, 0, m_img.width, m_img.height);

	// Texture用のHeapPropを作成
	D3D12_HEAP_PROPERTIES texProp{};
	texProp.Type                 = D3D12_HEAP_TYPE_CUSTOM;
	texProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texProp.CreationNodeMask     = 0;
	texProp.VisibleNodeMask      = 0;

	// Texture用のResourceDescを作成
	D3D12_RESOURCE_DESC resDesc{};
	resDesc.Format             = _metaData.format;
	resDesc.Width              = static_cast<UINT>(_metaData.width);
	resDesc.Height             = static_cast<UINT>(_metaData.height);
	resDesc.DepthOrArraySize   = static_cast<uint16_t>(_metaData.arraySize);
	resDesc.SampleDesc.Count   = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels          = static_cast<uint16_t>(_metaData.mipLevels);
	resDesc.Dimension          = static_cast<D3D12_RESOURCE_DIMENSION>(_metaData.dimension);
	resDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

	// Texture用ResourceBufferを作成
	hr = DEV->CreateCommittedResource(
		&texProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(m_texBuffer.GetAddressOf())
	);
	if (FAILED(hr)) { return false; }


	hr = m_texBuffer->WriteToSubresource(
		0, nullptr,
		m_img.pixels,
		static_cast<UINT>(m_img.rowPitch),
		static_cast<UINT>(m_img.slicePitch) //全サイズ
	);
	if (FAILED(hr)) { return false; }

	return true;
}
