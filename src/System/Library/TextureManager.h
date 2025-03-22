#pragma once

class TextureManager : Buffer
{
public:

	bool Load(const std::string& filename);

	void Set(int index);

#pragma region Žæ“¾Œn

	Image GetImage() const { return m_image; }

	CD3DX12_RECT GetRect() const { return m_rect; }

	int GetSRVNumber() const { return m_srvNumber; }

#pragma endregion

private:
	Image        m_image;
	CD3DX12_RECT m_rect;

	int m_srvNumber = 0;
};