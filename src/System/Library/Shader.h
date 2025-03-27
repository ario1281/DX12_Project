#pragma once

#include "Pipeline.h"
#include "RootSignature.h"

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