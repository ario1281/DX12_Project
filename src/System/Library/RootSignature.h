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
	/// �쐬
	/// </summary>
	/// <param name="rangeType">�����W�^�C�v���X�g</param>
	/// <param name="cbvCount">CBV��</param>
	void Create(const std::vector<RangeType>& rangeType, UINT& cbvCount);

	#pragma region �擾�n

	ID3D12RootSignature* GetRootSignature() { return m_pRootSign.Get(); }

	#pragma endregion

private:
	/// <summary>
	/// �����W�̍쐬
	/// </summary>
	/// <param name="pRange">�����W�̃|�C���^</param>
	/// <param name="type">�����W�^�C�v</param>
	/// <param name="count">�o�^��</param>
	const D3D12_DESCRIPTOR_RANGE&
	CreateRange(RangeType type, int count);

	/// <summary>
	/// �T���v���[�̍쐬
	/// </summary>
	/// <param name="pSampDesc">�T���v���[�f�X�N�̃|�C���^</param>
	/// <param name="mode">�A�h���X���[�h</param>
	/// <param name="filter">�t�B���^�[</param>
	/// <param name="count">�g�p�T���v���[��</param>
	const D3D12_STATIC_SAMPLER_DESC&
	CreateStaticSampler(
		TextureAddressMode         mode,
		D3D12Filter                filter,
		int                        count
	);

	com_ptr<ID3D10Blob>          m_pRootBlob = nullptr;
	com_ptr<ID3D12RootSignature> m_pRootSign = nullptr;
};
