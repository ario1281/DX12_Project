#include "define.h"
#include "Direct3D12.h"

void Direct3D12::DebugLayer()
{
#ifndef NDEBUG
    ID3D12Debug* debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug))))
    {
        debug->EnableDebugLayer();
        debug->Release();
    }
#endif // !NDEBUG
}

bool Direct3D12::Init(HINSTANCE hInst, HWND hwnd, int h, int w)
{
    HRESULT hr;
    hr = InitializeDXGIDevice();
    if (FAILED(hr)) { return false; }

    hr = InitializeD3D12Command();
    if (FAILED(hr)) { return false; }

    hr = CreateSwapChain(hwnd, h, w);


    return true;
}

HRESULT Direct3D12::InitializeDXGIDevice()
{
    HRESULT hr;

    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory));
    if (FAILED(hr)) { return hr; }

    // DirectX12まわりの初期化
    vector<IDXGIAdapter*> adapters;
    for (int i = 0; m_Factory->EnumAdapters(i, &m_Adapter) != DXGI_ERROR_NOT_FOUND ; i++)
    {
        adapters.push_back(m_Adapter);
    }
    for (auto adapter : adapters)
    {
        DXGI_ADAPTER_DESC adapter_desc = {};
        adapter->GetDesc(&adapter_desc);
        wstring str = adapter_desc.Description;
        if (str.find(L"NVIDIA") != string::npos)
        {
            m_Adapter = adapter;
            break;
        }
    }

    // Direct3Dデバイスの初期化
    D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    for (auto lv: levels)
    {
        hr = D3D12CreateDevice(m_Adapter, lv, IID_PPV_ARGS(&m_Device));
        if (SUCCEEDED(hr)) {
            m_Level = lv;
            break;
        }
    }

    return hr;
}

HRESULT Direct3D12::InitializeD3D12Command()
{
    HRESULT hr;

    hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CmdAllocator));
    if (FAILED(hr)) { return hr; }

    hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CmdAllocator, nullptr, IID_PPV_ARGS(&m_CmdList));
    if (FAILED(hr)) { return hr; }

    D3D12_COMMAND_QUEUE_DESC cqDesc = {};
    cqDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cqDesc.NodeMask = 0;
    cqDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    cqDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;

    hr = m_Device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&m_CmdQueue));
    if (FAILED(hr)) { return hr; }

    return hr;
}

HRESULT Direct3D12::CreateSwapChain(const HWND& hwnd, int h, int w)
{
    DXGI_SWAP_CHAIN_DESC1 scDesc1 = {};
    scDesc1.Width  = w;
    scDesc1.Height = h;
    scDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc1.Stereo = false;
    scDesc1.SampleDesc.Count = 1;
    scDesc1.SampleDesc.Quality = 0;
    scDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc1.BufferCount = 2;
    scDesc1.Scaling = DXGI_SCALING_STRETCH;
    scDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    return m_Factory->CreateSwapChainForHwnd(m_CmdQueue, hwnd, &scDesc1, nullptr, nullptr, (IDXGISwapChain1**)&m_SwapChain);
}

HRESULT Direct3D12::CreateFinalRenderTarget()
{
    HRESULT hr;

    vector<ID3D12Resource*> backBuffers;
    ID3D12DescriptorHeap* rtvHeaps = nullptr;

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.NodeMask       = 0;
    heapDesc.NumDescriptors = 2;
    heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
    if (FAILED(hr)) { return hr; }

    DXGI_SWAP_CHAIN_DESC swcDesc = {};
    hr = m_SwapChain->GetDesc(&swcDesc);
    backBuffers.resize(swcDesc.BufferCount);

    D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();

    //SRGBレンダーターゲットビュー設定
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (size_t i = 0; i < swcDesc.BufferCount; ++i) {
        hr = m_SwapChain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&backBuffers[i]));
        assert(SUCCEEDED(hr));

        rtvDesc.Format = backBuffers[i]->GetDesc().Format;
        m_Device->CreateRenderTargetView(backBuffers[i], &rtvDesc, handle);
        handle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }
    return hr;
}
