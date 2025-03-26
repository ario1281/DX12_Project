#include "Mesh.h"

void Mesh::Create(const std::vector<MeshVertex>& verteces, const std::vector<MeshFace>& faces, const Material material)
{
	HRESULT hr;

	m_material = material;

	if (static_cast<int>(verteces.size()) == 0) {
		assert(0 && "���_��1������܂���");
		return;
	}

	m_instCount = static_cast<UINT>(faces.size() * 3);

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type                 = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resDesc = {};

	#pragma region �o�[�e�b�N�X�o�b�t�@�쐬

	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width            = sizeof(MeshVertex) * verteces.size();
	resDesc.Height           = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels        = 1;
	resDesc.Format           = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags            = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pVBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "���_�o�b�t�@�[�쐬���s");
		return;
	}

	m_vbView.BufferLocation = m_pVBuffer->GetGPUVirtualAddress();
	m_vbView.SizeInBytes    = (UINT)resDesc.Width;
	m_vbView.StrideInBytes  = sizeof(MeshVertex);

	// ���_�o�b�t�@�ɏ�����������
	MeshVertex* vbMap = nullptr; {
		m_pVBuffer->Map(0, nullptr, (void**)&vbMap);
		std::copy(std::begin(verteces), std::end(verteces), vbMap);
		m_pVBuffer->Unmap(0, nullptr);
	}

	#pragma endregion

	#pragma region �C���f�N�X�o�b�t�@�쐬

	resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
	resDesc.Width            = sizeof(MeshFace) * faces.size();
	resDesc.Height           = 1;
	resDesc.DepthOrArraySize = 1;
	resDesc.MipLevels        = 1;
	resDesc.Format           = DXGI_FORMAT_UNKNOWN;
	resDesc.SampleDesc.Count = 1;
	resDesc.Flags            = D3D12_RESOURCE_FLAG_NONE;
	resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// �C���f�b�N�X�o�b�t�@�쐬
	hr = DEV->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&m_pIBuffer)
	);

	if (FAILED(hr)) {
		assert(0 && "�C���f�b�N�X�o�b�t�@�[�쐬���s");
		return;
	}

	// �C���f�b�N�X�o�b�t�@�̃f�[�^���r���[�ɏ�������
	m_ibView.BufferLocation = m_pIBuffer->GetGPUVirtualAddress();
	m_ibView.SizeInBytes    = (UINT)resDesc.Width;
	m_ibView.Format         = DXGI_FORMAT_R32_UINT;

	// �C���f�b�N�X�o�b�t�@�ɏ�����������
	MeshFace* ibMap = nullptr; {
		m_pIBuffer->Map(0, nullptr, (void**)&ibMap);
		std::copy(std::begin(faces), std::end(faces), ibMap);
		m_pIBuffer->Unmap(0, nullptr);
	}

	#pragma endregion

}

void Mesh::DrawInstanced(UINT verCount) const
{
	CMD->IASetVertexBuffers(0, 1, &m_vbView);
	CMD->IASetIndexBuffer(&m_ibView);

	CMD->DrawIndexedInstanced(verCount, 1, 0, 0, 0);
}