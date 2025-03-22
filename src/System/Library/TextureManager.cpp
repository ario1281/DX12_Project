#include "TextureManager.h"

bool TextureManager::Load(const std::string &filename)
{
	HRESULT hr; 

	std::wstring _path = stow(filename);

	// WIC�e�N�X�`�����[�h
	TexMetadata  _metaData;
	ScratchImage _scratchImg;
	hr = LoadFromWICFile(_path.c_str(), WIC_FLAGS_NONE, &_metaData, _scratchImg);
	if (FAILED(hr)) { return false; }

	m_pImage = _scratchImg.GetImage(0, 0, 0);

	// ��`�̒����Z�b�g
	SetRect(&m_rect, 0, 0, m_pImage->width, m_pImage->height);

	// Texture�p��HeapProp���쐬
	D3D12_HEAP_PROPERTIES texProp{};
	texProp.Type                 = D3D12_HEAP_TYPE_CUSTOM;
	texProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texProp.CreationNodeMask     = 0;
	texProp.VisibleNodeMask      = 0;

	// Texture�p��ResourceDesc���쐬
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

	// Texture�pResourceBuffer���쐬
	hr = DEV->CreateCommittedResource(
		&texProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(&m_pBuffer)
	);
	if (FAILED(hr)) {
		assert(0 && "�e�N�X�`���o�b�t�@�쐬���s");
		return false;
	}

	hr = m_pBuffer->WriteToSubresource(
		0, nullptr,
		m_pImage->pixels,
		static_cast<UINT>(m_pImage->rowPitch),
		static_cast<UINT>(m_pImage->slicePitch) //�S�T�C�Y
	);
	if (FAILED(hr)) {
		assert(0 && "�o�b�t�@�Ƀe�N�X�`���f�[�^�̏������ݎ��s");
		return false;
	}

	m_srvNumber = D3D.GetCSUHeap()->CreateSRV(m_pBuffer.Get());

	return true;
}

void TextureManager::Set(int index)
{
	CMD->SetGraphicsRootDescriptorTable(
		index, D3D.GetCSUHeap()->GetGPUHandle(m_srvNumber)
	);
}
