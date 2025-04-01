#pragma once
#include "Game/GameObject.h"

class CameraComponent : public GameObject
{
public:
	CameraComponent();

	virtual bool Update(const std::list<shared_ptr<GameObject>>* obj) override;

	void SetOwner(const shared_ptr<GameObject>& spOwner) { m_wpOwner = spOwner; }

private:
	weak_ptr<GameObject> m_wpOwner;
	CameraManager        m_camera;

	Vector3              m_vOffset = { 0,1.25f,-5.0f };
};