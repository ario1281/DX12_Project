#pragma once

class Direct3D12
{
public:
	bool Init(HINSTANCE hInst, HWND hwnd, int w, int h, bool fullscreen);

private:
	IDXGIFactory6* m_Factory	= nullptr;
	IDXGIAdapter* m_Adapter		= nullptr;
	ID3D12Device* m_Device		= nullptr;

	ID3D12CommandAllocator*	m_CmdAllocator	= nullptr;
	ID3D12GraphicsCommandList* m_CmdList	= nullptr;
	ID3D12CommandQueue* m_CmdQueue			= nullptr;
	IDXGISwapChain4* m_SwapChain			= nullptr;

	D3D_FEATURE_LEVEL m_Level;

	HRESULT InitializeDXGIDevice();
	HRESULT InitializeD3D12Command();

	HRESULT CreateSwapChain(const HWND &hwnd, int h, int w);
	HRESULT CreateFinalRenderTarget();


#pragma region ƒVƒ“ƒOƒ‹ƒgƒ“
public:
	static Direct3D12& GetInstance() {
		static Direct3D12 instance;
		return instance;
	}

private:
	Direct3D12() {}

#pragma endregion

};

#define DX12 Direct3D12::GetInstance()