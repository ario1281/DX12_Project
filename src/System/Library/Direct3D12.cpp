#include "define.h"
#include "Direct3D12.h"

bool Direct3D12::Init(HINSTANCE hInst, HWND hwnd, int w, int h, bool fullscreen)
{
	#ifdef _DEBUG

	// �f�o�b�O���C���[��L���ɂ���
	com_ptr<ID3D12Debug> _debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&_debug));
	_debug->EnableDebugLayer();

	#endif // _DEBUG
	
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

	// �t���X�N���[��
	SetFullScreen(fullscreen);

	//=======================================================
	// �q�[�v�̍쐬
	//=======================================================
	if (!CreateDescriptorHeap())
	{
		assert(0 && "�q�[�v�̍쐬���s");
		return false;
	}

	m_upBufferAllocater = std::make_unique<CBufferAllocator>();
	m_upBufferAllocater->Create(m_upCSUHeap.get());

	//=======================================================
	// �[�x�e�N�X�`���̍쐬
	//=======================================================
	m_upDepthStencil = std::make_unique<DepthStencil>();
	if (!m_upDepthStencil->Create(Vector2((float)w, (float)h)))
	{
		assert(0 && "DepthStencil�̍쐬���s");
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

void Direct3D12::Prepare()
{
	// ���\�[�X�o���A�̃X�e�[�g�������_�[�^�[�Q�b�g�ɕύX
	auto bbIdx = m_pSwapChain->GetCurrentBackBufferIndex();
	SetResourceBarrier(
		m_pSwapChainBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);

	// �����_�[�^�[�Q�b�g���Z�b�g
	auto rtvH = m_upRTVHeap->GetCPUHandle(bbIdx);
	auto dstH = m_upDSVHeap->GetCPUHandle(m_upDepthStencil->GetDSVNumber());
	m_pCmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	// �Z�b�g���������_�[�^�[�Q�b�g�̉�ʂ��N���A
	float clearColor[] = { 0.0f, 0.0f, 1.0f, 1.0f }; // �F
	m_pCmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	// �f�v�X�o�b�t�@�̃N���A
	m_upDepthStencil->ClearBuffer();
}

void Direct3D12::ScreenFlip()
{
	// ���\�[�X�o���A�̃X�e�[�g���v���[���g�ɖ߂�
	auto bbIdx = m_pSwapChain->GetCurrentBackBufferIndex();
	SetResourceBarrier(
		m_pSwapChainBuffers[bbIdx].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT
	);

	// ���߂̃N���[�Y
	m_pCmdList->Close();

	// �R�}���h���X�g�̎��s
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

void Direct3D12::WaitForCommandQueue()
{
	m_pCmdQueue->Signal(m_pFence.Get(), ++m_fenceVal);

	if (m_pFence->GetCompletedValue() != m_fenceVal)
	{
		auto event = CreateEvent(nullptr, false, false, nullptr);	// �C�x���g�n���h���̎擾
		if (!event)
		{
			assert(0 && "�C�x���g�G���[�A�A�v���P�[�V�������I�����܂�");
		}

		m_pFence->SetEventOnCompletion(m_fenceVal, event);
		WaitForSingleObject(event, INFINITE);				// �C�x���g����������܂ő҂�������
		CloseHandle(event);									// �C�x���g�n���h�������
	}
}



bool Direct3D12::CreateDevice()
{
	HRESULT hr;
	UINT flag = 0;

	com_ptr<IDXGIAdapter>              pAdapter;
	std::vector<com_ptr<IDXGIAdapter>> pAdapters;
	std::vector<DXGI_ADAPTER_DESC>     descs;

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	#ifdef _DEBUG

	flag |= DXGI_CREATE_FACTORY_DEBUG;

	// �f�o�b�O���C���[��L���ɂ���
	com_ptr<ID3D12Debug> _debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&_debug));
	_debug->EnableDebugLayer();

	#endif // _DEBUG

	hr = CreateDXGIFactory2(flag, IID_PPV_ARGS(&m_pDxgiFactory));
	if (FAILED(hr)) { return false; }

	for (UINT i = 0; true; ++i)
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
		if (std::wstring(descs[i].Description).find(L"NVIDIA") != std::wstring::npos)
		{
			pAdapter = pAdapters[i];
			break;
		}
		else if (std::wstring(descs[i].Description).find(L"Amd") != std::wstring::npos)
		{
			if (gpuTier > GPUTier::Amd)
			{
				pAdapter = pAdapters[i];
				gpuTier  = GPUTier::Amd;
			}
		}
		else if (std::wstring(descs[i].Description).find(L"Intel") != std::wstring::npos)
		{
			if (gpuTier > GPUTier::Intel)
			{
				pAdapter = pAdapters[i];
				gpuTier  = GPUTier::Intel;
			}
		}
		else if (std::wstring(descs[i].Description).find(L"Arm") != std::wstring::npos)
		{
			if (gpuTier > GPUTier::Arm)
			{
				pAdapter = pAdapters[i];
				gpuTier  = GPUTier::Arm;
			}
		}
		else if (std::wstring(descs[i].Description).find(L"Qualcomm") != std::wstring::npos)
		{
			if (gpuTier > GPUTier::Qualcomm)
			{
				pAdapter = pAdapters[i];
				gpuTier  = GPUTier::Qualcomm;
			}
		}
	}

	D3D_FEATURE_LEVEL featLevel;
	for (auto lv : levels)
	{
		if (D3D12CreateDevice(pAdapter.Get(), lv, IID_PPV_ARGS(&m_pDevice)) == S_OK)
		{
			featLevel = lv;
			break;          // �����\�ȃo�[�W���������������烋�[�v�ł��؂�
		}
	}

	return true;
}

bool Direct3D12::CreateCommandList()
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
	desc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;       // �^�C���A�E�g�Ȃ�
	desc.NodeMask = 0;                                   // �A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł���
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL; // �v���C�I���e�B�͓��Ɏw��Ȃ�
	desc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;      // �R�}���h���X�g�ƍ��킹��
	hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCmdQueue));

	if (FAILED(hr)) { return false; }

	return true;
}

bool Direct3D12::CreateSwapChain(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.Width              = width;
	desc.Height             = height;
	desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage        = DXGI_USAGE_BACK_BUFFER;
	desc.BufferCount        = 2;
	desc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;          // �t���b�v��͑��₩�ɔj��
	desc.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // �E�B���h�E�ƃt���X�N���[���؂�ւ��\


	// �X���b�v�`�F�C���̍쐬
	IDXGISwapChain1* pSwapChain;
	auto hr = m_pDxgiFactory->CreateSwapChainForHwnd(
		m_pCmdQueue.Get(),
		hwnd, &desc,
		nullptr, nullptr,
		&pSwapChain
	);
	if (FAILED(hr)) { return false; }

	pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));

	return true;
}

bool Direct3D12::CreateDescriptorHeap()
{
	m_upRTVHeap = std::make_unique<RTVHeap>();
	m_upCSUHeap = std::make_unique<CSUHeap>();
	m_upDSVHeap = std::make_unique<DSVHeap>();

	//=======================================================
	// �q�[�v�̍쐬
	//=======================================================
	// RTV�q�[�v
	if (!m_upRTVHeap->Create(HeapType::RTV, 100)) { return false; }

	// CBV, SRV, UAV�q�[�v
	if (!m_upCSUHeap->Create(HeapType::CSU, Vector3(100, 100, 100))) { return false; }

	// DSV�q�[�v
	if (!m_upDSVHeap->Create(HeapType::DSV, 100)) { return false; }

	return true;
}

bool Direct3D12::CreateSwapChainRTV()
{
	for (UINT i = 0; i < (int)m_pSwapChainBuffers.size(); ++i)
	{
		auto hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pSwapChainBuffers[i]));
		if (FAILED(hr)) { return false; }

		m_upRTVHeap->CreateRTV(m_pSwapChainBuffers[i].Get());
	}

	return true;
}

bool Direct3D12::CreateFence()
{
	auto hr = m_pDevice->CreateFence(m_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
	if (FAILED(hr)) { return false; }

	return true;
}

void Direct3D12::SetResourceBarrier(ID3D12Resource* pResource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Transition.pResource   = pResource;
	barrier.Transition.StateAfter  = after;
	barrier.Transition.StateBefore = before;
	m_pCmdList->ResourceBarrier(1, &barrier);
}

