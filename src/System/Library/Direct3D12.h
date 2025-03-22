#pragma once

class RTVHeap;
class DSVHeap;
class CSUHeap;

class CBufferAllocator;
class DepthStencil;

class Direct3D12
{
public:
	/// <summary>
	/// Direct3Dの初期化
	/// </summary>
	/// <param name="hInst">ウィンドウハンドル</param>
	/// <param name="hwnd"></param>
	/// <param name="w">横幅</param>
	/// <param name="h">縦幅</param>
	/// <param name="fullscreen">フルスクリーン</param>
	bool Init(HINSTANCE hInst, HWND hwnd, int width, int height, bool fullscreen);
	
	/// <summary>
	/// 描画準備
	/// </summary>
	void Prepare();

	/// <summary>
	/// 画面の切り替え(スワップチェイン)
	/// </summary>
	void ScreenFlip();

	/// <summary>
	/// コマンドキューの同期待ち
	/// </summary>
	void WaitForCommandQueue();

#pragma region 取得系

	ID3D12Device8*
	GetDevice() const { return m_pDevice.Get(); }

	ID3D12GraphicsCommandList6*
	GetCmdList() const { return m_pCmdList.Get(); }

	CSUHeap* GetCSUHeap() const { return m_upCSUHeap.get(); }
	DSVHeap* GetDSVHeap() const { return m_upDSVHeap.get(); }

	CBufferAllocator* GetCBufferAllocator() const {
		return m_upBufferAllocater.get();
	}

	void SetFullScreen(bool fullscreen)
	{
		if (m_pSwapChain != nullptr) {
			// フルスクリーンに切り替え
			m_pSwapChain->SetFullscreenState(fullscreen, nullptr);
		}
	}

#pragma endregion

private:
	bool CreateFactory();
	bool CreateDevice();

	bool CreateCommandList();
	bool CreateSwapChain(HWND hwnd, int width, int height);
	bool CreateDescriptorHeap();
	bool CreateSwapChainRTV();

	bool CreateFence();

	void SetResourceBarrier(
		ID3D12Resource*       pResource,
		D3D12_RESOURCE_STATES before,
		D3D12_RESOURCE_STATES after
	);

	enum class GPUTier
	{
		NVIDIA,
		Amd,
		Intel,
		Arm,
		Qualcomm,
		Kind,
	};

	com_ptr<ID3D12Device8>                 m_pDevice       = nullptr;
	com_ptr<IDXGIFactory7>                 m_pDxgiFactory  = nullptr;

	com_ptr<ID3D12CommandAllocator>        m_pCmdAllocator = nullptr;
	com_ptr<ID3D12GraphicsCommandList7>    m_pCmdList      = nullptr;
	com_ptr<ID3D12CommandQueue>            m_pCmdQueue     = nullptr;

	com_ptr<IDXGISwapChain4>               m_pSwapChain    = nullptr;

	std::array<com_ptr<ID3D12Resource>, 2> m_pSwapChainBuffers;
	unique_ptr<RTVHeap>                    m_pRTVHeap      = nullptr;

	com_ptr<ID3D12Fence>                   m_pFence        = nullptr;
	UINT64                                 m_fenceVal      = 0;

	unique_ptr<RTVHeap>          m_upRTVHeap            = nullptr;
	unique_ptr<CSUHeap>          m_upCSUHeap            = nullptr;
	unique_ptr<DSVHeap>          m_upDSVHeap            = nullptr;
	unique_ptr<DepthStencil>     m_upDepthStencil       = nullptr;
	unique_ptr<CBufferAllocator> m_upBufferAllocater    = nullptr;

#pragma region シングルトン
public:
	static Direct3D12& GetInstance() {
		static Direct3D12 instance;
		return instance;
	}

private:
	Direct3D12() {}

#pragma endregion

};

#define D3D  Direct3D12::GetInstance()
#define DEV  (&D3D)->GetDevice()
#define CMD  (&D3D)->GetCmdList()