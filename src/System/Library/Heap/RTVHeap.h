#pragma once

class RTVHeap
{
public:
	bool Create(ID3D12Device* pDevice, int useCount);

	int CreateRTV(ID3D12Resource* pBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUHandle(int number);

private:
	ID3D12Device*				  m_pDevice = nullptr;
	com_ptr<ID3D12DescriptorHeap> m_pHeap	= nullptr;

	int m_useCount			= 0;
	int m_incrementSize		= 0;
	int m_nextRegistNumber	= 0;

};