#pragma once

struct RenderSet
{
};

class Shader
{
public:

private:
	ID3D10Blob* m_pVSBlob = nullptr; // ���_�V�F�[�_�[
	ID3D10Blob* m_pHSBlob = nullptr; // �n���V�F�[�_�[
	ID3D10Blob* m_pDSBlob = nullptr; // �h���C���V�F�[�_�[
	ID3D10Blob* m_pGSBlob = nullptr; // �W�I���g���V�F�[�_�[
	ID3D10Blob* m_pPSBlob = nullptr; // �s�N�Z���V�F�[�_�[

	UINT m_cbvCount = 0;
};