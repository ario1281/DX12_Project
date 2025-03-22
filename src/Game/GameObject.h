#pragma once

class GameObject
{
public:
	GameObject()  { OutputDebugString("GameObjectが確保"); }
	~GameObject() { OutputDebugString("GameObjectが解放"); }

#pragma region 取得系



#pragma endregion

	virtual bool Update(const std::list<shared_ptr<GameObject>>* obj = nullptr) { return false; }
	virtual void Draw2D();
	virtual void Draw3D();

	void SetModel(const std::string &name);

protected:
	// オブジェクト情報
	shared_ptr<MeshManager>    m_spModel = nullptr;
	shared_ptr<TextureManager> m_spTex   = nullptr;

	Matrix                   m_mWorld;

	std::string                m_tag     = "";
	std::string                m_name    = "GameObject";
};
