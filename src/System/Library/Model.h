#pragma once

class ModelLoader;

class Model
{
public:

	struct Node
	{
		shared_ptr<Mesh> spMesh; // メッシュ
		Matrix           mLocal; // 座標
	};

	/// <summary>
	/// モデルのロード
	/// </summary>
	/// <param name="filepath">ファイルパス</param>
	/// <returns>成功したらtrue</returns>
	bool Load(const std::string& filename);

	#pragma region 取得系

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