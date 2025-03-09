#pragma once

class SceneBase
{
public:
	SceneBase() {

	}
	~SceneBase() {}

	virtual void Start() {}
	virtual bool Update() { return false; }
	virtual void Render3D() {}
	virtual void Render2D() {}

	void Frame();

private:
	bool IsActive() { return GetActiveWindow() != NULL ? true : false; }

};
