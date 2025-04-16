#pragma once

#pragma region �萔�^

enum class DepthStencilFormat
{
	DepthLowQuality            = DXGI_FORMAT_R16_TYPELESS,
	DepthHighQuality           = DXGI_FORMAT_R32_TYPELESS,
	DepthHighQualityAndStencil = DXGI_FORMAT_R24G8_TYPELESS,
};

#pragma endregion

class Buffer
{
public:
	Buffer() {}
	~Buffer() {}

protected:
	com_ptr<ID3D12Resource> m_pBuffer = nullptr;
};

/// <summary>
/// �[�x�e�N�X�`��
/// </summary>
class DepthStencil: public Buffer
{
public:
	/// <summary>
	/// �[�x�e�N�X�`������
	/// </summary>
	/// <param name="_resolute">�𑜓x</param>
	/// <param name="_format">�[�x�e�N�X�`���̃t�H�[�}�b�g</param>
	bool Create(
		const Vector2 _resolute,
		DepthStencilFormat _format = DepthStencilFormat::DepthHighQuality
	);

	/// <summary>
	/// �[�x�o�b�t�@�ɏ������܂�Ă���f�[�^������������֐�
	/// </summary>
	void ClearBuffer();

	#pragma region �擾�n

	/// <summary>
	/// DSV�ԍ����擾
	/// </summary>
	/// <returns>DSV�ԍ�</returns>
	UINT GetDSVNumber() { return m_dsvNumber; }

	#pragma endregion

private:
	UINT m_dsvNumber = 0;
};

/// <summary>
/// �o�b�t�@�[�A���P�[�^
/// </summary>
class CBufferAllocator : public Buffer
{
public:
	/// <summary>
	/// �쐬
	/// </summary>
	/// <param name="pHeap">CBVSRVUAVHeap�̃|�C���^</param>
	void Create(CSUHeap* pHeap);

	/// <summary>
	/// �g�p���Ă���o�b�t�@�̔ԍ���������
	/// </summary>
	void ResetCurrentUseNumber();

	/// <summary>
	/// �萔�o�b�t�@�[�Ƀf�[�^�̃o�C���h���s��
	/// </summary>
	/// <typeparam name="T">�^��</typeparam>
	/// <param name="idx">���W�X�^�ԍ�</param>
	/// <param name="data">�o�C���h�f�[�^</param>
	template<typename T>
	void BindAndAttachData(int dscIdx, const T& data)
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

		auto heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		UINT64 incSize = DEV->GetDescriptorHandleIncrementSize(heapType);

		// D3D12_CPU_DESCRIPTOR_HANDLE
		auto cpuHandle = m_pHeap->GetHeap()->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += incSize * m_curUseNumber;

		DEV->CreateConstantBufferView(&cbDesc, cpuHandle);


		// D3D12_GPU_DESCRIPTOR_HANDLE
		auto gpuHandle = m_pHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart();
		gpuHandle.ptr += incSize * m_curUseNumber;

		CMD->SetGraphicsRootDescriptorTable(dscIdx, gpuHandle);

		m_curUseNumber += useValue;
	}


private:
	CSUHeap*                   m_pHeap      = nullptr;
	struct { char buf[256]; }* m_pMapBuffer = nullptr;

	int      m_curUseNumber = 0;
};

/// <summary>
/// �e�N�X�`��
/// </summary>
class Texture : public Buffer
{
public:
	/// <summary>
	/// �e�N�X�`���̃��[�h
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	/// <returns>���[�h������������true</returns>
	bool Load(const std::string& filename);

	/// <summary>
	/// �V�F�[�_�[���\�[�X�Ƃ��ăZ�b�g
	/// </summary>
	/// <param name="index">�C���f�b�N�X</param>
	void Set(int index);

	#pragma region �擾�n

	/// <summary>
	/// SRV�ԍ����擾
	/// </summary>
	/// <returns>SRV�ԍ�</returns>
	int GetSRVNumber() { return m_srvNumber; }

	#pragma endregion

private:
	int m_srvNumber = 0;
};
