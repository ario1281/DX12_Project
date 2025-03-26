#pragma once

enum class RangeType
{
	SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
	UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
	CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
};

enum TextureAddressMode
{
	Wrap,
	Clamp,
};

enum class D3D12Filter
{
	Point,
	Linear,
};

class RootSignature
{
public:
	/// <summary>
	/// 作成
	/// </summary>
	/// <param name="rangeType">レンジタイプリスト</param>
	/// <param name="cbvCount">CBV数</param>
	void Create(const std::vector<RangeType>& rangeType, UINT& cbvCount);

	#pragma region 取得系

	ID3D12RootSignature* GetRootSignature() { return m_pRootSign.Get(); }

	#pragma endregion

private:
	/// <summary>
	/// レンジの作成
	/// </summary>
	/// <param name="pRange">レンジのポインタ</param>
	/// <param name="type">レンジタイプ</param>
	/// <param name="count">登録数</param>
	const D3D12_DESCRIPTOR_RANGE&
	CreateRange(RangeType type, int count);

	/// <summary>
	/// サンプラーの作成
	/// </summary>
	/// <param name="pSampDesc">サンプラーデスクのポインタ</param>
	/// <param name="mode">アドレスモード</param>
	/// <param name="filter">フィルター</param>
	/// <param name="count">使用サンプラー数</param>
	const D3D12_STATIC_SAMPLER_DESC&
	CreateStaticSampler(
		TextureAddressMode         mode,
		D3D12Filter                filter,
		int                        count
	);

	com_ptr<ID3D10Blob>          m_pRootBlob = nullptr;
	com_ptr<ID3D12RootSignature> m_pRootSign = nullptr;
};
