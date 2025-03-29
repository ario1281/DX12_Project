#pragma once

class ModelLoader;

class Model
{
public:

	struct Node
	{
		shared_ptr<Mesh> spMesh; // ���b�V��
		Matrix           mLocal; // ���W
	};

	/// <summary>
	/// ���f���̃��[�h
	/// </summary>
	/// <param name="filepath">�t�@�C���p�X</param>
	/// <returns>����������true</returns>
	bool Load(const std::string& filename);

	#pragma region �擾�n

	const std::vector<Node>& GetNodes() const { return m_nodes; }

	#pragma endregion

private:
	std::vector<Node> m_nodes;
};

class ModelLoader
{
public:

private:

};