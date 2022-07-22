#pragma once
#include"define.h"

class Direct3D12
{
public:
	HRESULT CreateDepthStencilView();
	shared_ptr<ID3D12Resource> GetTextureByPath(const char* texpath);
	HRESULT InitializeDXGIDevice();
	HRESULT CreateSwapChain(const HWND& hwnd);
	HRESULT InitializeCommand();
	HRESULT CreateSceneView();
	HRESULT CreateFinalRenderTargets();

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
	Direct3D12() { }
	~Direct3D12() { }
};

#define D3D12 Direct3D12::GetInstance()