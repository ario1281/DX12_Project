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

	#pragma region 取得系

	// カメラ行列の設定
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

	//　射影行列の作成
	void CreateParspectiveFovLH(float fov, float aspect, float zNear, float zFar) {
		m_stCam.mProj = XMMatrixPerspectiveFovLH(fov, aspect, zNear, zFar);
	}

private:
	Matrix   m_mCam;
	StCamera m_stCam;
};
