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

	// �����_�̈ʒu�ύX
	mCamera.Move(0, 0.5f, 0);

	// �J�����ʒu
	m_camera.GetCamMat() = Matrix();

	// ��]
	m_camera.GetCamMat().RotateX(XMConvertToRadians(CURSOR.Horizontal()));
	m_camera.GetCamMat().RotateX(XMConvertToRadians(CURSOR.Vertical()));

	// �ړ�
	m_camera.GetCamMat().MoveLocal(m_vOffset);
	m_camera.GetCamMat().Move(mCamera.GetPos());

	m_camera.CameraToView();
	m_camera.CreateParspectiveFovLH(45.0f, 16.0f / 9.0f, 1.0f, 2000.0f);

	D3D.SetCamera(m_camera);

	return true;
}
