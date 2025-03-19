#pragma once

class MeshManager
{
public:
	bool Load(const std::string &filename);

	void Draw();

private:
	com_ptr<ID3D12Resource> m_VBuffer;
	com_ptr<ID3D12Resource> m_IBuffer;

	std::string m_fileName;
};
