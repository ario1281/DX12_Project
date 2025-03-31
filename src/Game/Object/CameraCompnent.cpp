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

	// 注視点の位置変更
	mCamera.Move(0, 0.5f, 0);

	// カメラ位置
	m_camera.GetCamMat() = Matrix();


}
