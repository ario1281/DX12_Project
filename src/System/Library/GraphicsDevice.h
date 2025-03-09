#pragma once

class GraphicsDevice
{
public:
	bool Init(HINSTANCE hInst, HWND hwnd, int w, int h, bool fullscreen);

	void ScreenFlip();

	void WaitForCommandQueue();


	ID3D12Device8* GetDevice()const { return m_pDevice.Get(); }

	ID3D12GraphicsCommandList6* GetCmdList()const { return m_pCmdList.Get(); }

private:
	bool CreateFactory();
	bool CreateDevice();
	bool CreateCommandList();
	bool CreateSwapChain(HWND hwnd, int width, int height);
	bool CreateSwapChainRTV();
	bool CreateFence();
	void SetResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	enum class GPUTier
	{
		NVIDIA,
		Amd,
		Intel,
		Arm,
		Qualcomm,
		Kind,
	};

	com_ptr<ID3D12Device8>				m_pDevice			= nullptr;
	com_ptr<IDXGIFactory6>				m_pDxgiFactory		= nullptr;

	com_ptr<ID3D12CommandAllocator>		m_pCmdAllocator		= nullptr;
	com_ptr<ID3D12GraphicsCommandList6>	m_pCmdList			= nullptr;
	com_ptr<ID3D12CommandQueue>			m_pCmdQueue			= nullptr;

	com_ptr<IDXGISwapChain4>			m_pSwapChain		= nullptr;

	array<com_ptr<ID3D12Resource>, 2>	m_pSwapChainBuffers;
	unique_ptr<RTVHeap>					m_pRTVHeap			= nullptr;

	com_ptr<ID3D12Fence>				m_pFence			= nullptr;
	UINT64								m_fenceVal			= 0;

#pragma region ƒVƒ“ƒOƒ‹ƒgƒ“
public:
	static GraphicsDevice& GetInstance() {
		static GraphicsDevice instance;
		return instance;
	}

private:
	GraphicsDevice() {}

#pragma endregion

};

#define DX12 GraphicsDevice::GetInstance()