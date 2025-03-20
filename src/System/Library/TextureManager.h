#pragma once

class TextureManager
{
public:
	/// <summary>
	/// �e�N�X�`���[�ǂݍ���
	/// </summary>
	/// <param name="filename">�t�@�C����</param>
	/// <returns>
	/// ����:true�@���s:false
	/// </returns>
	bool Load(const std::wstring& filename);


#pragma region �擾�n

	const Image& GetImage() const { return m_img; }

	const D3D12_RECT& GetRect() const { return m_rect; }

#pragma endregion

private:
	com_ptr<ID3D12Resource> m_texBuffer; // 

	Image          m_img;
	D3D12_RECT     m_rect;
};