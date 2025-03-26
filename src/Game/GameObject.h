#pragma once

class GameObject
{
public:
	GameObject()  { OutputDebugString("GameObject���m��"); }
	~GameObject() { OutputDebugString("GameObject�����"); }

	#pragma region �擾�n

	inline const std::string& GetTag() const { return m_tag; }
	inline const std::string& GetName() const { return m_name; }

	#pragma endregion

	virtual bool Update(const std::list<shared_ptr<GameObject>>* obj = nullptr) { return false; }
	virtual void Draw2D();
	virtual void Draw3D();

	void SetModel(const std::string &name);

protected:
	// �I�u�W�F�N�g���
	shared_ptr<Mesh>    m_spModel = nullptr;
	shared_ptr<Texture> m_spTex   = nullptr;

	Matrix      m_mWorld;
	std::string m_tag     = "";
	std::string m_name    = "GameObject";
};
