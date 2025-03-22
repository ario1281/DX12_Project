#pragma once

class Buffer;

class TextureManager : Buffer
{
public:

	bool Load(const std::string& filename);

	void Set(int index);

#pragma region Žæ“¾Œn

	Image* GetImage() const { return m_pImage.Get(); }

	CD3DX12_RECT GetRect() const { return m_rect; }

	int GetSRVNumber() const { return m_srvNumber; }

#pragma endregion

private:
	com_ptr<Image> m_pImage = nullptr;
	CD3DX12_RECT   m_rect;

	int m_srvNumber = 0;
};