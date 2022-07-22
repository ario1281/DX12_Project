#pragma once
#include"define.h"

class Direct3D12
{
public:
	HRESULT Direct3D12::CreateDepthStencilView();
	smart_ptr<ID3D12Resource> GetTextureByPath(const char* texpath);
	HRESULT InitializeDXGIDevice();

private:
	IDXGIFactory6* Factory = nullptr;
	ID3D12Device* Device = nullptr;
	ID3D12CommandAllocator* cmdAllocator = nullptr;
	ID3D12GraphicsCommandList* cmdList = nullptr;
	ID3D12CommandQueue* cmdQueue = nullptr;
	IDXGISwapChain4* SwapChain = nullptr;

public:
	static Direct3D12& GetInstance()
	{
		static Direct3D12 instance;
		return instance;
	}
private:
	Direct3D12() { OutputDebugString("DirectX12—pHeader‚ªŠm•Û"); }
	~Direct3D12() { OutputDebugString("GDirectX12—pHeader‚ª‰ð•ú"); }
};

#define D3D12 Direct3D12::GetInstance()