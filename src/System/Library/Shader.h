#pragma once

#pragma region 定数型

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

#pragma region 構造型

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
	/// 作成
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <param name="renderingSetting">描画設定</param>
	/// <param name="rangeTypes">レンジタイプ</param>
	void Create(
		const std::string&            filename,
		const RenderSetting&          rendrSetting,
		const std::vector<RangeType>& rangeType
	);

	/// <summary>
	/// 描画開始
	/// </summary>
	/// <param name="w">横幅</param>
	/// <param name="h">縦幅</param>
	void Begin(int width, int height);

	/// <summary>
	/// メッシュの描画
	/// </summary>
	/// <param name="mesh">メッシュ</param>
	void DrawMesh(const Mesh& mesh);

	/// <summary>
	/// モデルの描画
	/// </summary>
	/// <param name="modelData">モデルデータ</param>
	void DrawModel(const Model& model);

	/// <summary>
	/// CBVカウント取得
	/// </summary>
	/// <returns>CBVカウント</returns>
	UINT GetCBVCount() const { return m_cbvCount; }

private:
	/// <summary>
	/// シェーダーファイルのロード
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void LoadShaderFile(const std::string& filePath);

	/// <summary>
	/// マテリアルをセット
	/// </summary>
	/// <param name="material">マテリアル情報</param>
	void SetMaterial(const Material& material);

	std::unique_ptr<Pipeline>		m_upPipeline      = nullptr;
	std::unique_ptr<RootSignature>	m_upRootSignature = nullptr;

	ID3D10Blob* m_pVSBlob = nullptr; // 頂点シェーダー
	ID3D10Blob* m_pHSBlob = nullptr; // ハルシェーダー
	ID3D10Blob* m_pDSBlob = nullptr; // ドメインシェーダー
	ID3D10Blob* m_pGSBlob = nullptr; // ジオメトリシェーダー
	ID3D10Blob* m_pPSBlob = nullptr; // ピクセルシェーダー

	UINT m_cbvCount = 0;
};

class Pipeline
{
public:
	/// <summary>
	/// 描画設定のセット
	/// </summary>
	/// <param name="pRootSignature">ルートシグネチャのポインタ</param>
	/// <param name="inputLayouts">頂点レイアウト情報</param>
	/// <param name="cullMode">カリングモード</param>
	/// <param name="blendMode">ブレンドモード</param>
	/// <param name="topologyType">プリミティブトポロジー</param>
	void SetRenderSettings(
		RootSignature*                  pRootSignature,
		const std::vector<InputLayout>& inputLayouts,
		CullMode                        cullMode,
		BlendMode                       blendMode,
		PrimitiveTopologyType           topologyType
	);

	/// <summary>
	/// 作成
	/// </summary>
	/// <param name="pBlobs">シェーダーデータリスト</param>
	/// <param name="formats">フォーマットリスト</param>
	/// <param name="isDepth">深度テスト</param>
	/// <param name="isDepthMask">深度書き込み</param>
	/// <param name="rtvCount">RTV数</param>
	/// <param name="isWireFrame">ワイヤーフレームの有無</param>
	void Create(
		std::vector<ID3DBlob*>         pBlobs,
		const std::vector<DXGI_FORMAT> formats,
		bool                           isDepth,
		bool                           isDepthMask,
		int                            rtvCount,
		bool                           isWireFrame
	);

	#pragma region 取得系

	/// <summary>
	/// パイプラインの取得
	/// </summary>
	/// <returns>パイプライン</returns>
	ID3D12PipelineState*
	GetPipeline() { return m_pPipelineState.Get(); }

	/// <summary>
	/// トポロジータイプの取得
	/// </summary>
	/// <returns>トポロジータイプ</returns>
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
		TextureAddressMode mode,
		D3D12Filter        filter,
		int                count
	);

	com_ptr<ID3D10Blob>          m_pRootBlob = nullptr;
	com_ptr<ID3D12RootSignature> m_pRootSign = nullptr;
};
