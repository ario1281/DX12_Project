#pragma once

class Model
{
public:

	struct Node
	{
		shared_ptr<Mesh> spMesh; // ƒƒbƒVƒ…
		Matrix           mLocal; // À•W
	};

	bool Load(const std::string& filename);

#pragma region æ“¾Œn

	const std::vector<Node>& GetNodes() const { return m_nodes; }

#pragma endregion

private:
	std::vector<Node> m_nodes;
};