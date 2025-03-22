#pragma once

class SceneBase
{
public:
	SceneBase() {

	}
	~SceneBase() {}

	virtual void Start() { m_object.clear(); }
	virtual bool Update() { return false; }
	virtual void Render3D() {}
	virtual void Render2D() {}

	void Frame();

protected:
	bool IsActive() { return GetActiveWindow() != NULL ? true : false; }

	weak_ptr<GameObject>              m_wpCamera;
	std::list<shared_ptr<GameObject>> m_object;
};
