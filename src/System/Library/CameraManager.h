#pragma once

struct StCamera
{
	Matrix mView;
	Matrix mProj;
};

class CameraManager
{
public:
	CameraManager() {}

	#pragma region �擾�n

	// �J�����s��̐ݒ�
	Matrix& GetCamMat()           { return m_mCam; }

	const Matrix& GetView() const { return m_stCam.mView; }
	const Matrix& GetProj() const { return m_stCam.mProj; }

	StCamera& GetStCam() const { return m_stCam; }

	#pragma endregion

	void CameraToView()
	{
		m_stCam.mView = m_mCam;
		m_stCam.mView.Inverse();
	}

	//�@�ˉe�s��̍쐬
	void CreateParspectiveFovLH(float fov, float aspect, float zNear, float zFar) {
		m_stCam.mProj = XMMatrixPerspectiveFovLH(fov, aspect, zNear, zFar);
	}

private:
	Matrix   m_mCam;
	StCamera m_stCam;
};
