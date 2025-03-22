#include "Buffer.h"

bool DepthStencil::Create(const Vector2 _resolute, DepthStencilFormat _format)
{
	HRESULT hr;
	// �f�v�X�o�b�t�@�ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Width            = static_cast<UINT>(_resolute.x);
	resDesc.Height           = static_cast<UINT>(_resolute.y);
	resDesc.DepthOrArraySize = 1;
	resDesc.Format           = static_cast<DXGI_FORMAT>(_format);
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags            = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// �f�v�X�o�b�t�@�̃t�H�[�}�b�g�ƍő�[�x�l��ݒ�
	D3D12_CLEAR_VALUE depthValue = {};
	depthValue.DepthStencil.Depth = 1.0f;

	switch (_format)
	{
	case DepthStencilFormat::DepthLowQuality:
		depthValue.Format = DXGI_FORMAT_D16_UNORM;
		break;
	case DepthStencilFormat::DepthHighQuality:
		depthValue.Format = DXGI_FORMAT_D32_FLOAT;
		break;
	case DepthStencilFormat::DepthHighQualityAndStencil:
		depthValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		break;
	default:
		break;
	}

	// �ݒ�����Ƀf�v�X�o�b�t�@�𐶐�
	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthValue, IID_PPV_ARGS(&m_pBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "�f�v�X�X�e���V���o�b�t�@�̍쐬���s");
		return false;
	}

	// DSV�쐬
	m_dsvNumber = D3D.GetDSVHeap()->CreateDSV(
		m_pBuffer.Get(),
		depthValue.Format
	);

	return true;
}

void DepthStencil::ClearBuffer()
{
	CMD->ClearDepthStencilView(
		D3D.GetDSVHeap()->GetCPUHandle(m_dsvNumber),
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f, 0, 0,
		nullptr
	);
}



void CBufferAllocator::Create(CSUHeap* pHeap)
{
	HRESULT hr;
	m_pHeap = pHeap;

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Height           = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels        = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resDesc.Width            = (UINT64)((1 + 0xff) & ~0xff) * (int)m_pHeap->GetUseCount().x;

	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_pBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "CBufferAllocater�̍쐬���s");
		return;
	}

	m_pBuffer->Map(0, nullptr, (void**)&m_pMapBuffer);
}

void CBufferAllocator::ResetCurrentUseNumber()
{
	m_curUseNumber = 0;
}

template<typename T>
inline void CBufferAllocator::BindAndAttachData(int dscIdx, const T& data)
{
	if (!m_pHeap) { return; }

	// data�T�C�Y��256�A���C�����g���Čv�Z
	size_t align = (sizeof(T) + 0xff) & ~0xff;

	// 256byte�������g�p���邩�A���C�����g�������ʂ�256�Ŋ����Čv�Z
	int useValue = align / 0x100;

	// ���ݎg���I����Ă���ԍ��ƍ�����g���e�ʂ��q�[�v�̗e�ʂ𒴂��Ă���ꍇ�̓��^�[��
	if (m_curUseNumber + useValue > (int)m_pHeap->GetUseCount().x)
	{
		assert(0 && "�g�p�ł���q�[�v�e�ʂ𒴂��܂���");
		return;
	}

	int top = m_curUseNumber;

	// �擪�A�h���X�Ɏg�����̃|�C���^�𑫂���memcpy
	memcpy(m_pMapBuffer + top, &data, sizeof(T));

	// �r���[������Ēl���V�F�[�_�[�ɃA�^�b�`
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbDesc = {};
	cbDesc.BufferLocation = m_pBuffer->GetGPUVirtualAddress() + (UINT64)top * 0x100;
	cbDesc.SizeInBytes = align;

	//
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_pHeap->GetHeap()->GetCPUDescriptorHandleForHeapStart();
	cpuHandle.ptr += (UINT64)DEV->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) *m_curUseNumber;

	DEV->CreateConstantBufferView(&cbDesc, cpuHandle);

	//
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_pHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart();
	gpuHandle.ptr += (UINT64)DEV->GetDescriptorHandleIncrementSize
	(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) *m_curUseNumber;

	CMD->SetGraphicsRootDescriptorTable(dscIdx, gpuHandle);

	m_curUseNumber += useValue;
}
