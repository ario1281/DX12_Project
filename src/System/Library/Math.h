#pragma once

class Matrix;

class Vector3 : public SimpleMath::Vector3
{
public:
	// コンストラクタ
	Vector3() { x = y = z = 0; }
	Vector3(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	// コピーコンストラクタ
	Vector3(const SimpleMath::Vector3& _v) {
		*(SimpleMath::Vector3*)this = _v;
	}
	
	Vector3& operator = (const SimpleMath::Vector3& _v)
	{
		*(SimpleMath::Vector3*)this = _v;
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

class Matrix :public SimpleMath::Matrix
{
public:
	// コンストラクタ
	Matrix() {
		*(SimpleMath::Matrix*)this = XMMatrixIdentity();
	}

	//　コピーコンストラクタ
	Matrix(const SimpleMath::Matrix& m) {
		*(SimpleMath::Matrix*)this = m;
	}

	// 代入演算子(オペレータオーバーロード)
	Matrix& operator = (const SimpleMath::Matrix& m) {
		*(SimpleMath::Matrix*)this = m;
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
	void CreateTranslation(const Vector3 v) {
		*this = XMMatrixTranslation(v.x, v.y, v.z);
	}

	void SetPos(float x, float y, float z) {
		_41 = x;
		_42 = y;
		_43 = z;
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

	//　ビルボード処理
	void SetBillBoard(const Matrix& mCam) {
		// 自分の拡大率を取得
		float sX = GetXScale();
		float sY = GetYScale();
		float sZ = GetZScale();
		// カメラの回転をコピー(自分の拡大率も考慮)
		_11 = mCam._11 * sX;
		_12 = mCam._12 * sX;
		_13 = mCam._13 * sX;
		_21 = mCam._21 * sY;
		_22 = mCam._22 * sY;
		_23 = mCam._23 * sY;
		_31 = mCam._31 * sZ;
		_32 = mCam._32 * sZ;
		_33 = mCam._33 * sZ;
	}

	//===========================
	// 「取得」系
	//===========================
	const Vector3& GetPos() const {
		return *(Vector3*)&_41;
	}

	const Vector3& GetXAxis() const {
		return *(Vector3*)&_11;
	}
	const Vector3& GetYAxis() const {
		return *(Vector3*)&_21;
	}
	const Vector3& GetZAxis() const {
		return *(Vector3*)&_31;
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
