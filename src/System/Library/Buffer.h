#pragma once

class Buffer
{
public:
	Buffer() {}
	~Buffer() {}

protected:
	com_ptr<ID3D12Resource> m_pBuffer = nullptr;
};


enum class DepthStencilFormat
{
	DepthLowQuality            = DXGI_FORMAT_R16_TYPELESS,
	DepthHighQuality           = DXGI_FORMAT_R32_TYPELESS,
	DepthHighQualityAndStencil = DXGI_FORMAT_R24G8_TYPELESS,
};

/// <summary>
/// 深度テクスチャ
/// </summary>
class DepthStencil: public Buffer
{
public:
	/// <summary>
	/// 深度テクスチャ生成
	/// </summary>
	/// <param name="_resolute">解像度</param>
	/// <param name="_format">深度テクスチャのフォーマット</param>
	bool Create(
		const Vector2 _resolute,
		DepthStencilFormat _format = DepthStencilFormat::DepthHighQuality
	);

	/// <summary>
	/// 深度バッファに書き込まれているデータを初期化する関数
	/// </summary>
	void ClearBuffer();

	#pragma region 取得系

	/// <summary>
	/// DSV番号を取得
	/// </summary>
	/// <returns>DSV番号</returns>
	UINT GetDSVNumber() { return m_dsvNumber; }

	#pragma endregion

private:
	UINT m_dsvNumber = 0;
};

/// <summary>
/// バッファーアロケータ
/// </summary>
class CBufferAllocator : public Buffer
{
public:
	/// <summary>
	/// 作成
	/// </summary>
	/// <param name="pHeap">CBVSRVUAVHeapのポインタ</param>
	void Create(CSUHeap* pHeap);

	/// <summary>
	/// 使用しているバッファの番号を初期化
	/// </summary>
	void ResetCurrentUseNumber();

	/// <summary>
	/// 定数バッファーにデータのバインドを行う
	/// </summary>
	/// <param name="idx">レジスタ番号</param>
	/// <param name="data">バインドデータ</param>
	template<typename T>
	void BindAndAttachData(int idx, const T& data);

private:
	CSUHeap*                   m_pHeap      = nullptr;
	struct { char buf[256]; }* m_pMapBuffer = nullptr;

	int      m_curUseNumber = 0;
};

/// <summary>
/// テクスチャ
/// </summary>
class Texture : public Buffer
{
public:
	/// <summary>
	/// テクスチャのロード
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	/// <returns>ロードが成功したらtrue</returns>
	bool Load(const std::string& filename);

	/// <summary>
	/// シェーダーリソースとしてセット
	/// </summary>
	/// <param name="index">インデックス</param>
	void Set(int index);

	#pragma region 取得系

	/// <summary>
	/// SRV番号を取得
	/// </summary>
	/// <returns>SRV番号</returns>
	int GetSRVNumber() { return m_srvNumber; }

	#pragma endregion

private:
	int m_srvNumber = 0;
};