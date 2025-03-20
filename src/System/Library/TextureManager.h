#pragma once

class TextureManager
{
public:
	/// <summary>
	/// テクスチャー読み込み
	/// </summary>
	/// <param name="filename">ファイル名</param>
	/// <returns>
	/// 成功:true　失敗:false
	/// </returns>
	bool Load(const std::wstring& filename);


#pragma region 取得系

	const Image& GetImage() const { return m_img; }

	const D3D12_RECT& GetRect() const { return m_rect; }

#pragma endregion

private:
	com_ptr<ID3D12Resource> m_texBuffer; // 

	Image          m_img;
	D3D12_RECT     m_rect;
};