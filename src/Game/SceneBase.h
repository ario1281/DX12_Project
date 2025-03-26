#pragma once

class GameObject;

class SceneBase
{
public:
	SceneBase() { Start(); }
	~SceneBase() {}

	virtual void Start() { SafeClear(m_objects); }
	virtual bool Update() { return false; }
	virtual void Render3D() {}
	virtual void Render2D() {}

	void Frame();

	shared_ptr<GameObject> FindObjectWithTag(const std::string& tag);

protected:
	bool IsActive() { return GetActiveWindow() != NULL ? true : false; }

	weak_ptr<GameObject>              m_wpCamera;
	std::list<shared_ptr<GameObject>> m_objects;
};
