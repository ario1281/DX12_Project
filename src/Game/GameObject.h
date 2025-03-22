#pragma once

class GameObject
{
public:
	GameObject()  { OutputDebugString("GameObject���m��"); }
	~GameObject() { OutputDebugString("GameObject�����"); }

#pragma region �擾�n



#pragma endregion

	virtual bool Update(const std::list<shared_ptr<GameObject>>* obj = nullptr) { return false; }
	virtual void Draw2D();
	virtual void Draw3D();

	void SetModel(const std::string &name);

protected:
	// �I�u�W�F�N�g���
	shared_ptr<MeshManager>    m_spModel = nullptr;
	shared_ptr<TextureManager> m_spTex   = nullptr;

	Matrix                   m_mWorld;

	std::string                m_tag     = "";
	std::string                m_name    = "GameObject";
};
