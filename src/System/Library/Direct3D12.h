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
	/// Direct3D�̏�����
	/// </summary>
	/// <param name="hInst">�E�B���h�E�n���h��</param>
	/// <param name="hwnd"></param>
	/// <param name="w">����</param>
	/// <param name="h">�c��</param>
	/// <param name="fullscreen">�t���X�N���[��</param>
	bool Init(HINSTANCE hInst, HWND hwnd, int width, int height, bool fullscreen);
	
	/// <summary>
	/// �`�揀��
	/// </summary>
	void Prepare();

	/// <summary>
	/// ��ʂ̐؂�ւ�(�X���b�v�`�F�C��)
	/// </summary>
	void ScreenFlip();

	/// <summary>
	/// �R�}���h�L���[�̓����҂�
	/// </summary>
	void WaitForCommandQueue();

#pragma region �擾�n

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
			// �t���X�N���[���ɐ؂�ւ�
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

#pragma region �V���O���g��
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