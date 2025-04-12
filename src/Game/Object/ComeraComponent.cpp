#include "CameraComponent.h"

CameraComponent::CameraComponent()
{
	m_tag = "Camera";
}

bool CameraComponent::Update(const std::list<shared_ptr<GameObject>>* obj)
{
	auto& mCamera = m_mWorld;
	auto owner = m_wpOwner.lock();
	if (owner) { mCamera = owner->GetWorldMatrix(); }

	// 注視点の位置変更
	mCamera.Move(0, 0.5f, 0);

	// カメラ位置
	m_camera.GetCamMat() = Matrix();

	// 回転
	m_camera.GetCamMat().RotateX(XMConvertToRadians(CURSOR.Horizontal()));
	m_camera.GetCamMat().RotateX(XMConvertToRadians(CURSOR.Vertical()));

	// 移動
	m_camera.GetCamMat().MoveLocal(m_vOffset);
	m_camera.GetCamMat().Move(mCamera.GetPos());

	m_camera.CameraToView();
	m_camera.CreateParspectiveFovLH(45.0f, 16.0f / 9.0f, 1.0f, 2000.0f);

	D3D.SetCamera(m_camera);

	return true;
}
