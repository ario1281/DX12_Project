#include "Buffer.h"

#pragma region DepthStencil

bool DepthStencil::Create(const Vector2 _resolute, DepthStencilFormat _format)
{
	HRESULT hr;
	// デプスバッファ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width            = static_cast<UINT>(_resolute.x);
	resDesc.Height           = static_cast<UINT>(_resolute.y);
	resDesc.DepthOrArraySize = 1;
	resDesc.Format           = static_cast<DXGI_FORMAT>(_format);
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// デプスバッファのフォーマットと最大深度値を設定
	D3D12_CLEAR_VALUE depthValue = {};
	depthValue.DepthStencil.Depth = 1.0f;

	switch (_format)
	{
	case DepthStencilFormat::DepthLowQuality:
		depthValue.Format = DXGI_FORMAT_D16_UNORM;
		break;
	case DepthStencilFormat::DepthHighQuality:
		depthValue.Format = DXGI_FORMAT_D32_FLOAT;
		break;
	case DepthStencilFormat::DepthHighQualityAndStencil:
		depthValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	default:
		break;
	}

	// 設定を元にデプスバッファを生成
	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthValue, IID_PPV_ARGS(&m_pBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "デプスステンシルバッファの作成失敗");
		return false;
	}

	// DSV作成
	m_dsvNumber = D3D.GetDSVHeap()->CreateDSV(
		m_pBuffer.Get(),
		depthValue.Format
	);

	return true;
}

void DepthStencil::ClearBuffer()
{
	CMD->ClearDepthStencilView(
		D3D.GetDSVHeap()->GetCPUHandle(m_dsvNumber),
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr
	);
}

#pragma endregion

#pragma region CBufferAllocator

void CBufferAllocator::Create(CSUHeap* pHeap)
{
	HRESULT hr;

	m_pHeap = pHeap;

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Height           = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels        = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Width            = (UINT64)(((1 + 0xff) & ~0xff) * m_pHeap->GetUseCount().x);

	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "CBufferAllocaterの作成失敗");
		return;
	}

	m_pBuffer->Map(0, nullptr, (void**)&m_pMapBuffer);
}

void CBufferAllocator::ResetCurrentUseNumber()
{
	m_curUseNumber = 0;
}

#pragma endregion

#pragma region Texture

bool Texture::Load(const std::string& filename)
{
	HRESULT hr;

	std::wstring _path;
	stow_s(filename, _path);

	TexMetadata  metaData = {};
	ScratchImage scratchImg = {};
	const Image* pImage = nullptr;

	// WICテクスチャロード
	hr = LoadFromWICFile(_path.c_str(), WIC_FLAGS_NONE, &metaData, scratchImg);
	if (FAILED(hr)) { return false; }

	pImage = scratchImg.GetImage(0, 0, 0);

	// Texture用のHeapPropを作成
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_CUSTOM;
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	// Texture用のResourceDescを作成
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension          = (D3D12_RESOURCE_DIMENSION)metaData.dimension;
	resDesc.Format             = metaData.format;
	resDesc.Width              = (UINT)metaData.width;
	resDesc.Height             = (UINT)metaData.height;
	resDesc.DepthOrArraySize   = (UINT16)metaData.arraySize;
	resDesc.MipLevels          = (UINT16)metaData.mipLevels;
	resDesc.SampleDesc.Count   = 1;

	// Texture用ResourceBufferを作成
	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(&m_pBuffer)
	);
	if (FAILED(hr)) {
		assert(0 && "テクスチャバッファ作成失敗");
		return false;
	}

	hr = m_pBuffer->WriteToSubresource(
		0, nullptr,
		pImage->pixels,
		(UINT)pImage->rowPitch,
		(UINT)pImage->slicePitch
	);
	if (FAILED(hr)) {
		assert(0 && "バッファにテクスチャデータの書き込み失敗");
		return false;
	}

	m_srvNumber = D3D.GetCSUHeap()->CreateSRV(m_pBuffer.Get());

	return true;
}

void Texture::Set(int index)
{
	CMD->SetGraphicsRootDescriptorTable(
		index, D3D.GetCSUHeap()->GetGPUHandle(m_srvNumber)
	);
}

#pragma endregion
