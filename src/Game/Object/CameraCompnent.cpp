#include "CameraComponent.h"

CameraComponent::CameraComponent()
{
	m_tag = "Camera";
}

bool CameraComponent::Update(const std::list<shared_ptr<GameObject>>* obj)
{
	auto& mCamera = m_mWorld;
	auto owner    = m_wpOwner.lock();
	if (owner) { mCamera = owner->GetWorldMatrix(); }

	// �����_�̈ʒu�ύX
	mCamera.Move(0, 0.5f, 0);

	// �J�����ʒu
	m_camera.GetCamMat() = Matrix();


}
