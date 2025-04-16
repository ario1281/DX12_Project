#pragma once

#pragma region 定数型

enum class DepthStencilFormat
{
	DepthLowQuality            = DXGI_FORMAT_R16_TYPELESS,
	DepthHighQuality           = DXGI_FORMAT_R32_TYPELESS,
	DepthHighQualityAndStencil = DXGI_FORMAT_R24G8_TYPELESS,
};

#pragma endregion

class Buffer
{
public:
	Buffer() {}
	~Buffer() {}

protected:
	com_ptr<ID3D12Resource> m_pBuffer = nullptr;
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
	/// <typeparam name="T">型名</typeparam>
	/// <param name="idx">レジスタ番号</param>
	/// <param name="data">バインドデータ</param>
	template<typename T>
	void BindAndAttachData(int dscIdx, const T& data)
	{
		if (!m_pHeap) { return; }

		// dataサイズを256アライメントして計算
		size_t align = (sizeof(T) + 0xff) & ~0xff;

		// 256byteをいくつ使用するかアライメントした結果を256で割って計算
		int useValue = align / 0x100;

		// 現在使い終わっている番号と今から使う容量がヒープの容量を超えている場合はリターン
		if (m_curUseNumber + useValue > (int)m_pHeap->GetUseCount().x)
		{
			assert(0 && "使用できるヒープ容量を超えました");
			return;
		}

		int top = m_curUseNumber;

		// 先頭アドレスに使う分のポインタを足してmemcpy
		memcpy(m_pMapBuffer + top, &data, sizeof(T));

		// ビューを作って値をシェーダーにアタッチ
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc = {};
		cbDesc.BufferLocation = m_pBuffer->GetGPUVirtualAddress() + (UINT64)top * 0x100;
		cbDesc.SizeInBytes = align;

		auto heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		UINT64 incSize = DEV->GetDescriptorHandleIncrementSize(heapType);

		// D3D12_CPU_DESCRIPTOR_HANDLE
		auto cpuHandle = m_pHeap->GetHeap()->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += incSize * m_curUseNumber;

		DEV->CreateConstantBufferView(&cbDesc, cpuHandle);


		// D3D12_GPU_DESCRIPTOR_HANDLE
		auto gpuHandle = m_pHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += incSize * m_curUseNumber;

		CMD->SetGraphicsRootDescriptorTable(dscIdx, gpuHandle);

		m_curUseNumber += useValue;
	}


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
