#pragma once

class CameraManager
{
public:
	CameraManager() {}

	#pragma region 取得系

	// カメラ行列の設定
	Matrix& GetCamMat()           { return m_mCam; }

	const Matrix& GetView() const { return m_mView; }
	const Matrix& GetProj() const { return m_mProj; }

	#pragma endregion

	void CameraToView()
	{
		m_mView = m_mCam;
		m_mView.Inverse();
	}

	//　射影行列の作成
	void CreateParspectiveFovLH(float fov, float aspect, float zNear, float zFar) {
		m_mProj = XMMatrixPerspectiveFovLH(fov, aspect, zNear, zFar);
	}

private:
	Matrix m_mCam;
	Matrix m_mView;
	Matrix m_mProj;
};
