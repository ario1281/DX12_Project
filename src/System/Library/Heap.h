#pragma once

enum class HeapType
{
	SAMPLER = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
	CSU = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
	RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
	DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
};

template<typename TempHeap>
class Heap
{
public:
	Heap() {}
	~Heap() {}

	bool Create(GraphicsDevice* pDevice, HeapType heapType, TempHeap useCount)
	{
		auto type = static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(heapType);

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = type;
		heapDesc.NodeMask = 0;
		heapDesc.NumDescriptors = ComputeUseCount(useCount);
		heapDesc.Flags = heapType == HeapType::CSU ?
			D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		auto hr = pDevice->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pHeap));
		if (FAILED(hr)) { return false; }

		m_useCount = useCount;
		m_incrementSize = pDevice->GetDevice()->GetDescriptorHandleIncrementSize(type);
		m_pDevice = pDevice;

		return true;
	}

	virtual const D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int number)
	{
		auto handle = m_pHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += (UINT64)m_incrementSize * number;

		return handle;
	}

	virtual const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int number)
	{
		auto handle = m_pHeap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += (UINT64)m_incrementSize * number;

		return handle;
	}

protected:
	GraphicsDevice*               m_pDevice = nullptr;
	com_ptr<ID3D12DescriptorHeap> m_pHeap   = nullptr;

	TempHeap m_useCount;
	int      m_incrementSize    = 0;
	int      m_nextRegistNumber = 0;

private:
	UINT ComputeUseCount(UINT useCount)
	{
		return useCount;
	}

	UINT ComputeUseCount(XMVECTOR3 useCount)
	{
		return (UINT)(useCount.x + useCount.y + useCount.z);
	}
};

class RTVHeap : public Heap<int>
{
public:
	RTVHeap() {}
	~RTVHeap() {}

	int CreateRTV(ID3D12Resource* pBuffer);
};

class DSVHeap : public Heap<int>
{
public:
	DSVHeap() {}
	~DSVHeap() {}

	int CreateDSV(ID3D12Resource* pBuffer, DXGI_FORMAT format);
};

class CSUHeap : public Heap<XMVECTOR3>
{
	CSUHeap() {}
	~CSUHeap() {}

	int CreateSRV(ID3D12Resource* pBuffer);

	const D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int number)override;

	void SetHeap();


	ID3D12DescriptorHeap* GetHeap() { return m_pHeap.Get(); }

	const XMVECTOR3& GetUseCount() { return m_useCount; }
};