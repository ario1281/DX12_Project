#pragma once

#pragma region �萔�^

enum class CullMode
{
	None  = D3D12_CULL_MODE_NONE,
	Front = D3D12_CULL_MODE_FRONT,
	Back  = D3D12_CULL_MODE_BACK,
};
enum class BlendMode
{
	Add,
	Alpha,
};
enum class InputLayout
{
	POSITION,
	TEXCOORD,
	NORMAL,
	TANGENT,
	COLOR,
	SKININDEX,
	SKINWEIGHT
};
enum class PrimitiveTopologyType
{
	Undefined = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
	Point     = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
	Line      = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
	Triangle  = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	Patch     = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,
};

enum class RangeType
{
	SRV = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
	UAV = D3D12_DESCRIPTOR_RANGE_TYPE_UAV,
	CBV = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
};
enum class TextureAddressMode
{
	Wrap,
	Clamp,
};
enum class D3D12Filter
{
	Point,
	Linear,
};

#pragma endregion

#pragma region �\���^

struct RenderSetting
{
	std::vector<InputLayout> InputLayouts;
	std::vector<DXGI_FORMAT> Formats;

	CullMode  CullMode  = CullMode::Back;
	BlendMode BlendMode = BlendMode::Alpha;
	PrimitiveTopologyType PrimitiveTopologyType
		= PrimitiveTopologyType::Triangle;
	bool IsDepth     = true;
	bool IsDepthMask = true;
	int  RTVCount    = 1;
	bool IsWireFrame = false;
};

#pragma endregion

class Pipeline;
class RootSignature;

class Shader
{
public:
	/// <summary>
	/// �쐬
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <param name="renderingSetting">�`��ݒ�</param>
	/// <param name="rangeTypes">�����W�^�C�v</param>
	void Create(
		const std::string&            filename,
		const RenderSetting&          rendrSetting,
		const std::vector<RangeType>& rangeType
	);

	/// <summary>
	/// �`��J�n
	/// </summary>
	/// <param name="w">����</param>
	/// <param name="h">�c��</param>
	void Begin(int width, int height);

	/// <summary>
	/// ���b�V���̕`��
	/// </summary>
	/// <param name="mesh">���b�V��</param>
	void DrawMesh(const Mesh& mesh);

	/// <summary>
	/// ���f���̕`��
	/// </summary>
	/// <param name="modelData">���f���f�[�^</param>
	void DrawModel(const Model& model);

	/// <summary>
	/// CBV�J�E���g�擾
	/// </summary>
	/// <returns>CBV�J�E���g</returns>
	UINT GetCBVCount() const { return m_cbvCount; }

private:
	/// <summary>
	/// �V�F�[�_�[�t�@�C���̃��[�h
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	void LoadShaderFile(const std::string& filePath);

	/// <summary>
	/// �}�e���A�����Z�b�g
	/// </summary>
	/// <param name="material">�}�e���A�����</param>
	void SetMaterial(const Material& material);

	std::unique_ptr<Pipeline>		m_upPipeline      = nullptr;
	std::unique_ptr<RootSignature>	m_upRootSignature = nullptr;

	ID3D10Blob* m_pVSBlob = nullptr; // ���_�V�F�[�_�[
	ID3D10Blob* m_pHSBlob = nullptr; // �n���V�F�[�_�[
	ID3D10Blob* m_pDSBlob = nullptr; // �h���C���V�F�[�_�[
	ID3D10Blob* m_pGSBlob = nullptr; // �W�I���g���V�F�[�_�[
	ID3D10Blob* m_pPSBlob = nullptr; // �s�N�Z���V�F�[�_�[

	UINT m_cbvCount = 0;
};

class Pipeline
{
public:
	/// <summary>
	/// �`��ݒ�̃Z�b�g
	/// </summary>
	/// <param name="pRootSignature">���[�g�V�O�l�`���̃|�C���^</param>
	/// <param name="inputLayouts">���_���C�A�E�g���</param>
	/// <param name="cullMode">�J�����O���[�h</param>
	/// <param name="blendMode">�u�����h���[�h</param>
	/// <param name="topologyType">�v���~�e�B�u�g�|���W�[</param>
	void SetRenderSettings(
		RootSignature*                  pRootSignature,
		const std::vector<InputLayout>& inputLayouts,
		CullMode                        cullMode,
		BlendMode                       blendMode,
		PrimitiveTopologyType           topologyType
	);

	/// <summary>
	/// �쐬
	/// </summary>
	/// <param name="pBlobs">�V�F�[�_�[�f�[�^���X�g</param>
	/// <param name="formats">�t�H�[�}�b�g���X�g</param>
	/// <param name="isDepth">�[�x�e�X�g</param>
	/// <param name="isDepthMask">�[�x��������</param>
	/// <param name="rtvCount">RTV��</param>
	/// <param name="isWireFrame">���C���[�t���[���̗L��</param>
	void Create(
		std::vector<ID3DBlob*>         pBlobs,
		const std::vector<DXGI_FORMAT> formats,
		bool                           isDepth,
		bool                           isDepthMask,
		int                            rtvCount,
		bool                           isWireFrame
	);

	#pragma region �擾�n

	/// <summary>
	/// �p�C�v���C���̎擾
	/// </summary>
	/// <returns>�p�C�v���C��</returns>
	ID3D12PipelineState*
	GetPipeline() { return m_pPipelineState.Get(); }

	/// <summary>
	/// �g�|���W�[�^�C�v�̎擾
	/// </summary>
	/// <returns>�g�|���W�[�^�C�v</returns>
	PrimitiveTopologyType
	GetTopologyType() { return m_topologyType; }

	#pragma endregion

private:
	void SetInputLayout(
		std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElements,
		const std::vector<InputLayout>&        inputLayouts
	);

	void SetBlendMode(
		D3D12_RENDER_TARGET_BLEND_DESC& blendDesc,
		BlendMode                       blendMode
	);

	RootSignature*               m_pRootSignature = nullptr;

	std::vector<InputLayout>     m_inputLayouts;
	CullMode                     m_cullMode;
	BlendMode                    m_blendMode;
	PrimitiveTopologyType        m_topologyType;

	com_ptr<ID3D12PipelineState> m_pPipelineState = nullptr;
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
		TextureAddressMode mode,
		D3D12Filter        filter,
		int                count
	);

	com_ptr<ID3D10Blob>          m_pRootBlob = nullptr;
	com_ptr<ID3D12RootSignature> m_pRootSign = nullptr;
};
