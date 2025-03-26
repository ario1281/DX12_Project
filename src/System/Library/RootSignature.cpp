#include "RootSignature.h"

void RootSignature::Create(const std::vector<RangeType>& rangeType, UINT& cbvCount)
{
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
