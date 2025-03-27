#include "RootSignature.h"

void RootSignature::Create(const std::vector<RangeType>& rangeTypes, UINT& cbvCount)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignDesc = {};
	int rangeCount = (int)rangeTypes.size();

	// レンジ数分だけルートパラメータ、レンジを生成
	std::vector<D3D12_ROOT_PARAMETER>   rootParams(rangeCount);
	std::vector<D3D12_DESCRIPTOR_RANGE> ranges(rangeCount);

	// SRVの数だけSamplerDescを生成
	int sampCount = 0;

	for (int i = 0; i < (int)rangeTypes.size(); ++i)
	{
		if (rangeTypes[i] == RangeType::SRV) { ++sampCount; }
	}

	// 指定された順に設定
	sampCount = 0;

	bool bSampler = false;
	int uavCount = 0;

	for (int i = 0; i < rangeCount; ++i)
	{
		switch (rangeTypes[i])
		{
		case RangeType::CBV:
			ranges[i] = CreateRange(RangeType::CBV, cbvCount);
			++cbvCount;
			break;
		case RangeType::SRV:
			ranges[i] = CreateRange(RangeType::SRV, sampCount);
			++sampCount;
			bSampler = true;
			break;
		case RangeType::UAV:
			ranges[i] = CreateRange(RangeType::UAV, uavCount);
			++uavCount;
			break;
		default:
			break;
		}
		rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[i].DescriptorTable.pDescriptorRanges = &ranges[i];
		rootParams[i].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	std::array<D3D12_STATIC_SAMPLER_DESC, 4> pStaticSamplerDescs = {};

	if (bSampler)
	{
		pStaticSamplerDescs[0] = CreateStaticSampler(TextureAddressMode::Wrap, D3D12Filter::Point, 0);
		pStaticSamplerDescs[1] = CreateStaticSampler(TextureAddressMode::Clamp, D3D12Filter::Point, 1);
		pStaticSamplerDescs[2] = CreateStaticSampler(TextureAddressMode::Wrap, D3D12Filter::Linear, 2);
		pStaticSamplerDescs[3] = CreateStaticSampler(TextureAddressMode::Clamp, D3D12Filter::Linear, 3);
	}

	rootSignDesc.pStaticSamplers   = bSampler ? pStaticSamplerDescs.data() : nullptr;
	rootSignDesc.NumStaticSamplers = bSampler ? 4 : 0;
	rootSignDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignDesc.pParameters       = rootParams.data();
	rootSignDesc.NumParameters     = (int)rangeTypes.size();

	ID3DBlob* pErrBlob = nullptr;
	auto hr = D3D12SerializeRootSignature(
		&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&m_pRootBlob, &pErrBlob
	);
	if (FAILED(hr)) {
		assert(0 && "ルートシグネチャ初期化失敗");
	}

	hr = DEV->CreateRootSignature(
		0,
		m_pRootBlob->GetBufferPointer(),
		m_pRootBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_pRootSign)
	);
	if (FAILED(hr)) {
		assert(0 && "ルートシグネチャ作成失敗");
	}
}

const D3D12_DESCRIPTOR_RANGE&
RootSignature::CreateRange(RangeType type, int count)
{
	D3D12_DESCRIPTOR_RANGE descRange = {};
	descRange.NumDescriptors                    = 1;
	descRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRange.BaseShaderRegister                = count;
	descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	return descRange;
}

const D3D12_STATIC_SAMPLER_DESC&
RootSignature::CreateStaticSampler(TextureAddressMode mode, D3D12Filter filter, int count)
{
	D3D12_TEXTURE_ADDRESS_MODE _mode = {};
	_mode = mode == TextureAddressMode::Wrap ?
		D3D12_TEXTURE_ADDRESS_MODE_WRAP :
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	D3D12_FILTER _filter = {};
	_filter = filter == D3D12Filter::Point ?
		D3D12_FILTER_MIN_MAG_MIP_POINT :
		D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	D3D12_STATIC_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU         = _mode;
	sampDesc.AddressV         = _mode;
	sampDesc.AddressW         = _mode;
	sampDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampDesc.Filter           = _filter;
	sampDesc.MaxLOD           = D3D12_FLOAT32_MAX;
	sampDesc.MinLOD           = 0.0f;
	sampDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	sampDesc.MaxAnisotropy    = 16;
	sampDesc.ShaderRegister   = count;

	return sampDesc;
}
