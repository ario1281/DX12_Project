#pragma once

struct RenderSet
{
};

class Shader
{
public:

private:
	ID3D10Blob* m_pVSBlob = nullptr; // 頂点シェーダー
	ID3D10Blob* m_pHSBlob = nullptr; // ハルシェーダー
	ID3D10Blob* m_pDSBlob = nullptr; // ドメインシェーダー
	ID3D10Blob* m_pGSBlob = nullptr; // ジオメトリシェーダー
	ID3D10Blob* m_pPSBlob = nullptr; // ピクセルシェーダー

	UINT m_cbvCount = 0;
};