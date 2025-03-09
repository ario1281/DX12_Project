#include "define.h"
#include "GraphicsDevice.h"

bool GraphicsDevice::Init(HINSTANCE hInst, HWND hwnd, int w, int h, bool fullscreen)
{
    //=======================================================
    // �t�@�N�g���[�̍쐬
    //=======================================================
    if (!CreateFactory())
	{
		assert(0 && "D3D12�f�o�C�X�쐬���s");
		return false;
	}

    //=======================================================
    // �f�o�C�X�̍쐬
    //=======================================================
    if (!CreateDevice())
	{
		assert(0 && "�R�}���h���X�g�̍쐬���s");
		return false;
	}

	//=======================================================
	// �R�}���h���X�g�̍쐬
	//=======================================================
	if (!CreateCommandList())
	{
		assert(0 && "�R�}���h���X�g�̍쐬���s");
		return false;
	}

	//=======================================================
	// �X���b�v�`�F�C���̍쐬
	//=======================================================
	if (!CreateSwapChain(hwnd, w, h))
	{
		assert(0 && "�X���b�v�`�F�C���̍쐬���s");
		return false;
	}
	
	//=======================================================
	// �q�[�v�̍쐬
	//=======================================================
	m_pRTVHeap = make_unique<RTVHeap>();
	if (!m_pRTVHeap->Create(m_pDevice.Get(), 100))
	{
		assert(0 && "RTV�q�[�v�̍쐬���s");
		return false;
	}

	//=======================================================
	// �X���b�v�`�F�C��RTV�̍쐬
	//=======================================================
	if (!CreateSwapChainRTV())
	{
		assert(0 && "�X���b�v�`�F�C��RTV�̍쐬���s");
		return false;

	}

	//=======================================================
	// �X���b�v�`�F�C��RTV�̍쐬
	//=======================================================
	if (!CreateFence())
	{
		assert(0 && "�t�F���X�̍쐬���s");
		return false;
	}

	return true;
}

void GraphicsDevice::ScreenFlip()
{
	// ���\�[�X�o���A�̃X�e�[�g�������_�[�^�[�Q�b�g�ɕύX
	auto bbIdx = m_pSwapChain->GetCurrentBackBufferIndex();
	SetResourceBarrier(
		m_pSwapChainBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// �����_�[�^�[�Q�b�g���Z�b�g
	auto rtvHeap = m_pRTVHeap->GetRTVCPUHandle(bbIdx);
	m_pCmdList->OMSetRenderTargets(1, &rtvHeap, false, nullptr);

	// �Z�b�g���������_�[�^�[�Q�b�g�̉�ʂ��N���A
	float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_pCmdList->ClearRenderTargetView(rtvHeap, clearColor, 0, nullptr);

	// ���\�[�X�o���A�̃X�e�[�g���v���[���g�ɖ߂�
	SetResourceBarrier(
		m_pSwapChainBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	// �R�}���h���X�g����Ď��s
	ID3D12CommandList* cmdlists[] = { m_pCmdList.Get() };
	m_pCmdQueue->ExecuteCommandLists(1, cmdlists);

	// �R�}���h���X�g�̓o�L��҂�
	WaitForCommandQueue();

	// �R�}���h�A���P�[�^�[�ƃR�}���h���X�g��������
	m_pCmdAllocator->Reset();
	m_pCmdList->Reset(m_pCmdAllocator.Get(), nullptr);

	// �X���b�v�`�F�C���Ƀv���[���g�𑗂�
	m_pSwapChain->Present(1, 0);
}

void GraphicsDevice::WaitForCommandQueue()

{
	m_pCmdQueue->Signal(m_pFence.Get(), m_fenceVal++);

	if (m_pFence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);	// �C�x���g�n���h���̎擾
		if (!event) {
			assert(0 && "�C�x���g�G���[�A�A�v���P�[�V�������I�����܂�");
		}

		m_pFence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);				// �C�x���g����������܂ő҂�������
		CloseHandle(event);									// �C�x���g�n���h�������
	}
}


bool GraphicsDevice::CreateFactory()
{
    auto hr = CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG,
		IID_PPV_ARGS(m_pDxgiFactory.GetAddressOf())
	);
	if (FAILED(hr)) { return false; }

	return true;
}

bool GraphicsDevice::CreateDevice()
{
    com_ptr<IDXGIAdapter> pAdapter = nullptr;
    vector<com_ptr<IDXGIAdapter>> pAdapters;
    vector<DXGI_ADAPTER_DESC> descs;

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

    for (UINT i = 0; true; i++)
    {
        pAdapters.push_back(nullptr);
        auto hr = m_pDxgiFactory->EnumAdapters(i, &pAdapters[i]);

        if (hr == DXGI_ERROR_NOT_FOUND) { break; }

        descs.push_back({});
        pAdapters[i]->GetDesc(&descs[i]);
    }

	GPUTier gpuTier = GPUTier::Kind;
	for (int i = 0; i < descs.size(); ++i)
	{
		if (wstring(descs[i].Description).find(L"NVIDIA") != wstring::npos)
		{
			pAdapter = pAdapters[i];
			break;
		}
		else if (wstring(descs[i].Description).find(L"Amd") != wstring::npos)
		{
			if (gpuTier > GPUTier::Amd)
			{
				pAdapter = pAdapters[i];
				gpuTier = GPUTier::Amd;
			}
		}
		else if (wstring(descs[i].Description).find(L"Intel") != wstring::npos)
		{
			if (gpuTier > GPUTier::Intel)
			{
				pAdapter = pAdapters[i];
				gpuTier = GPUTier::Intel;
			}
		}
		else if (wstring(descs[i].Description).find(L"Arm") != wstring::npos)
		{
			if (gpuTier > GPUTier::Arm)
			{
				pAdapter = pAdapters[i];
				gpuTier = GPUTier::Arm;
			}
		}
		else if (wstring(descs[i].Description).find(L"Qualcomm") != wstring::npos)
		{
			if (gpuTier > GPUTier::Qualcomm)
			{
				pAdapter = pAdapters[i];
				gpuTier = GPUTier::Qualcomm;
			}
		}
	}

	D3D_FEATURE_LEVEL featLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(pAdapter.Get(), lv, IID_PPV_ARGS(&m_pDevice)) == S_OK)
		{
			featLevel = lv;
			break;		// �����\�ȃo�[�W���������������烋�[�v�ł��؂�
		}
	}

	return true;
}

bool GraphicsDevice::CreateCommandList()
{
	HRESULT hr;

	hr = m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pCmdAllocator)
	);
	if (FAILED(hr)) { return false; }

	hr = m_pDevice->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pCmdAllocator.Get(),
		nullptr, IID_PPV_ARGS(&m_pCmdList)
	);
	if (FAILED(hr)) { return false; }

	// �L���[����
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;		// �^�C���A�E�g�Ȃ�
	desc.NodeMask	= 0;									// �A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł���
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;	// �v���C�I���e�B�͓��Ɏw��Ȃ�
	desc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;		// �R�}���h���X�g�ƍ��킹��
	hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCmdQueue));

	if (FAILED(hr)) { return false; }

	return true;
}

bool GraphicsDevice::CreateSwapChain(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width				= width;
	desc.Height				= height;
	desc.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count	= 1;
	desc.BufferUsage		= DXGI_USAGE_BACK_BUFFER;
	desc.BufferCount		= 2;
	desc.SwapEffect			= DXGI_SWAP_EFFECT_FLIP_DISCARD;			// �t���b�v��͑��₩�ɔj��
	desc.Flags				= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;	// �E�B���h�E�ƃt���X�N���[���؂�ւ��\

	auto hr = m_pDxgiFactory->CreateSwapChainForHwnd(
		m_pCmdQueue.Get(),
		hwnd, &desc, nullptr, nullptr,
		(IDXGISwapChain1**)m_pSwapChain.GetAddressOf()
	);
	if (FAILED(hr)) { return false; }

	return true;
}

bool GraphicsDevice::CreateSwapChainRTV()
{
	for (int i = 0; i < (int)m_pSwapChainBuffers.size(); i++)
	{
		auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainBuffers[i]));
		if (FAILED(hr)) { return false; }

		m_pRTVHeap->CreateRTV(m_pSwapChainBuffers[i].Get());
	}

	return true;
}

bool GraphicsDevice::CreateFence()
{
	auto hr = m_pDevice->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hr)) { return false; }

	return true;
}

void GraphicsDevice::SetResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Transition.pResource	= pResource;
	barrier.Transition.StateAfter	= after;
	barrier.Transition.StateBefore	= before;
	m_pCmdList->ResourceBarrier(1, &barrier);
}

