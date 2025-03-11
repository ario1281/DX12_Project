#pragma once

class Matrix;

class Vector3 : public XMFLOAT3
{
public:
	// コンストラクタ
	Vector3() {
		x = y = z = 0;
	}
	Vector3(float _x, float _y, float _z) {
		x = _x; y = _y; z = _z;
	}

	// コピーコンストラクタ
	Vector3(const XMFLOAT3& v) {
		*(XMFLOAT3*)this = v;
	}
	// 代入演算子(オペレータオーバーロード)
	Vector3& operator=(const XMFLOAT3& v) {
		*(XMFLOAT3*)this = v;
		return *this;
	}

	void Set(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}

	void Normalize() {
		XMStoreFloat3(this, XMVector3Normalize(XMLoadFloat3(this)));
	}

	void SetLength(float len) {
		auto v = XMVector3Normalize(XMLoadFloat3(this)) * len;
		XMStoreFloat3(this, v);
	}

	float Length() const {
		Vector3 v;
		XMStoreFloat3(&v, XMVector3Length(XMLoadFloat3(this)));
		return v.x;
	}

	// ベクトル返還
	void TransNormal(const Matrix& m);
	void TransCoord(const Matrix& m);

};

class Matrix : public XMMATRIX
{
public:
	// コンストラクタ
	Matrix() {
		*(XMMATRIX*)this = XMMatrixIdentity();
	}

	// コピーコンストラクタ
	Matrix(const XMMATRIX& m) {
		*(XMMATRIX*)this = m;
	}

	// 代入演算子(オペレータオーバーロード)
	Matrix& operator=(const XMMATRIX& m) {
		*(XMMATRIX*)this = m;
		return *this;
	}

	//===========================
	// 「作成」系
	//===========================

	// 移動行列作成
	// ・x	… X座標
	// ・y	… Y座標
	// ・z	… Z座標
	void CreateTranslation(float x, float y, float z) {
		*this = XMMatrixTranslation(x, y, z);
	}

	void SetPosition(float x, float y, float z) {
		r[3].m128_f32[0] = x;
		r[3].m128_f32[1] = y;
		r[3].m128_f32[2] = z;
	}

	void CreateRotationX(float radian) {
		*this = XMMatrixRotationX(radian);
	}
	void CreateRotationY(float radian) {
		*this = XMMatrixRotationY(radian);
	}
	void CreateRotationZ(float radian) {
		*this = XMMatrixRotationZ(radian);
	}

	void CreateScaling(float x, float y, float z) {
		*this = XMMatrixScaling(x, y, z);
	}

	//===========================
	// 「操作」系
	//===========================

	void Move(float x, float y, float z) {
		Matrix m;
		m.CreateTranslation(x, y, z);
		*this = (*this) * m;
	}
	void Move(const Vector3& v) {
		Matrix m;
		m.CreateTranslation(v.x, v.y, v.z);
		*this = (*this) * m;
	}

	void MoveLocal(float x, float y, float z) {
		Matrix m;
		m.CreateTranslation(x, y, z);
		*this = m * (*this);
	}
	void MoveLocal(const Vector3& v) {
		Matrix m;
		m.CreateTranslation(v.x, v.y, v.z);
		*this = m * (*this);
	}

	// 
	void RotateX(float radian) {
		Matrix m;
		m.CreateRotationX(radian);
		*this = (*this) * m;
	}
	void RotateXLocal(float radian) {
		Matrix m;
		m.CreateRotationX(radian);
		*this = m * (*this);
	}
	// 
	void RotateY(float radian) {
		Matrix m;
		m.CreateRotationY(radian);
		*this = (*this) * m;
	}
	void RotateYLocal(float radian) {
		Matrix m;
		m.CreateRotationY(radian);
		*this = m * (*this);
	}
	// 
	void RotateZ(float radian) {
		Matrix m;
		m.CreateRotationZ(radian);
		*this = (*this) * m;
	}
	void RotateZLocal(float radian) {
		Matrix m;
		m.CreateRotationZ(radian);
		*this = m * (*this);
	}

	// 
	void Scale(float x, float y, float z) {
		Matrix m;
		m.CreateScaling(x, y, z);
		*this = (*this) * m;
	}
	void ScaleLocal(float x, float y, float z) {
		Matrix m;
		m.CreateScaling(x, y, z);
		*this = m * (*this);
	}

	void Inverse() {
		*this = XMMatrixInverse(nullptr, *this);
	}


	// ビルポート処理
	void SetBillBoard(const Matrix& mCam) {
		// 拡大率取得
		float sX = GetXScale();
		float sY = GetYScale();
		float sZ = GetZScale();
		// カメラの回転をコピー
		r[0].m128_f32[0] = mCam.r[0].m128_f32[0] * sX;
		r[0].m128_f32[1] = mCam.r[0].m128_f32[1] * sX;
		r[0].m128_f32[2] = mCam.r[0].m128_f32[2] * sX;
		r[1].m128_f32[0] = mCam.r[1].m128_f32[0] * sY;
		r[1].m128_f32[1] = mCam.r[1].m128_f32[1] * sY;
		r[1].m128_f32[2] = mCam.r[1].m128_f32[2] * sY;
		r[2].m128_f32[0] = mCam.r[2].m128_f32[0] * sZ;
		r[2].m128_f32[1] = mCam.r[2].m128_f32[1] * sZ;
		r[2].m128_f32[2] = mCam.r[2].m128_f32[2] * sZ;
	}

	//===========================
	// 「取得」系
	//===========================
	const Vector3& GetPos() const {
		return *(Vector3*)&r[3];
	}

	const Vector3& GetXAxis() const {
		return *(Vector3*)&r[0];
	}
	const Vector3& GetYAxis() const {
		return *(Vector3*)&r[1];
	}
	const Vector3& GetZAxis() const {
		return *(Vector3*)&r[2];
	}

	float GetXScale() const {
		return GetXAxis().Length();
	}
	float GetYScale() const {
		return GetYAxis().Length();
	}
	float GetZScale() const {
		return GetZAxis().Length();
	}




};
