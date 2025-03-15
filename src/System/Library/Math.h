#pragma once

class DXMATRIX;

class DXVECTOR3 : public XMFLOAT3
{
public:
	// コンストラクタ
	DXVECTOR3()
	{
		x = y = z = 0;
	}
	DXVECTOR3(float _x, float _y, float _z)
	{
		x = _x; y = _y; z = _z;
	}

	// コピーコンストラクタ
	DXVECTOR3(const XMFLOAT3& v)
	{
		*this = (DXVECTOR3)v;
	}

#pragma region 代入演算子

	// オペレータオーバーロード
	DXVECTOR3& operator = (const XMFLOAT3& v)
	{
		*this = v;
		return *this;
	}

	bool operator == (const XMFLOAT3& v) const
	{
		return x == v.x || y == v.y || z == v.z;
	}
	bool operator != (const XMFLOAT3& v) const
	{
		return x != v.x || y != v.y || z != v.z;
	}

	DXVECTOR3& operator += (const XMFLOAT3& v)
	{
		x += v.x; y += v.y; z += v.z;
		return *this;
	}
	DXVECTOR3& operator -= (const XMFLOAT3& v)
	{
		x -= v.x; y -= v.y; z -= v.z;
		return *this;
	}
	DXVECTOR3& operator *= (float f)
	{
		x *= f; y *= f; z *= f;
		return *this;
	}
	DXVECTOR3& operator /= (float f)
	{
		float fInv = 1.0f / f;
		x *= f; y *= f; z *= f;
		return *this;
	}

	DXVECTOR3 operator + (const XMFLOAT3& v) const
	{
		return DXVECTOR3(x + v.x, y + v.y, z + v.z);
	}
	DXVECTOR3 operator - (const XMFLOAT3& v) const
	{
		return DXVECTOR3(x - v.x, y - v.y, z - v.z);
	}
	DXVECTOR3 operator * (float f) const
	{
		return DXVECTOR3(x * f, y * f, z * f);
	}
	DXVECTOR3 operator / (float f) const
	{
		float fInv = 1.0f / f;
		return DXVECTOR3(x * fInv, y * fInv, z * fInv);
	}

	DXVECTOR3 operator + () const
	{
		return *this;
	}
	DXVECTOR3 operator - () const
	{
		return DXVECTOR3(-x, -y, -z);
	}

#pragma endregion

	void Set(float _x, float _y, float _z)
	{
		x = _x; y = _y; z = _z;
	}

	void Normalize()
	{
		auto v = XMVector3Normalize(XMLoadFloat3(this));
		XMStoreFloat3(this, v);
	}

	void SetLength(float len)
	{
		auto v = XMVector3Normalize(XMLoadFloat3(this)) * len;
		XMStoreFloat3(this, v);
	}
	float Length() const
	{
		auto v = XMVector3Length(XMLoadFloat3(this));
		return XMVectorGetX(v);
	}

	// ベクトル返還
	void TransNormal(const DXMATRIX& m);
	void TransCoord(const DXMATRIX& m);

};

class DXMATRIX : public XMMATRIX
{
public:
	// コンストラクタ
	DXMATRIX()
	{
		*this = XMMatrixIdentity();
	}

	// コピーコンストラクタ
	DXMATRIX(const XMMATRIX& m)
	{
		*this = m;
	}

#pragma region 代入演算子

	// オペレータオーバーロード
	DXMATRIX& operator=(const XMMATRIX& m)
	{
		*this = m;
		return *this;
	}

#pragma endregion

	//===========================
	// 「作成」系
	//===========================

	// 移動行列作成
	// ・m128_f32
	// m00, m01, m02, m03
	// m10, m11, m12, m13
	// m20, m21, m22, m23
	// m30, m31, m32, m33
	void CreateTranslation(float x, float y, float z)
	{
		*this = XMMatrixTranslation(x, y, z);
	}

	void SetPosition(float x, float y, float z)
	{
		r[3].m128_f32[0] = x; // m30
		r[3].m128_f32[1] = y; // m31
		r[3].m128_f32[2] = z; // m32
	}

	void CreateRotationX(float radian)
	{
		*this = XMMatrixRotationX(radian);
	}
	void CreateRotationY(float radian)
	{
		*this = XMMatrixRotationY(radian);
	}
	void CreateRotationZ(float radian)
	{
		*this = XMMatrixRotationZ(radian);
	}

	void CreateScaling(float x, float y, float z)
	{
		*this = XMMatrixScaling(x, y, z);
	}


	//===========================
	// 「操作」系
	//===========================

	void Move(float x, float y, float z) {
		DXMATRIX m;
		m.CreateTranslation(x, y, z);
		*this = (*this) * m;
	}
	void Move(const DXVECTOR3& v) {
		DXMATRIX m;
		m.CreateTranslation(v.x, v.y, v.z);
		*this = (*this) * m;
	}

	void MoveLocal(float x, float y, float z) {
		DXMATRIX m;
		m.CreateTranslation(x, y, z);
		*this = m * (*this);
	}
	void MoveLocal(const DXVECTOR3& v) {
		DXMATRIX m;
		m.CreateTranslation(v.x, v.y, v.z);
		*this = m * (*this);
	}

	// 
	void RotateX(float radian) {
		DXMATRIX m;
		m.CreateRotationX(radian);
		*this = (*this) * m;
	}
	void RotateXLocal(float radian) {
		DXMATRIX m;
		m.CreateRotationX(radian);
		*this = m * (*this);
	}
	// 
	void RotateY(float radian) {
		DXMATRIX m;
		m.CreateRotationY(radian);
		*this = (*this) * m;
	}
	void RotateYLocal(float radian) {
		DXMATRIX m;
		m.CreateRotationY(radian);
		*this = m * (*this);
	}
	// 
	void RotateZ(float radian) {
		DXMATRIX m;
		m.CreateRotationZ(radian);
		*this = (*this) * m;
	}
	void RotateZLocal(float radian) {
		DXMATRIX m;
		m.CreateRotationZ(radian);
		*this = m * (*this);
	}

	// 
	void Scale(float x, float y, float z)
	{
		DXMATRIX m;
		m.CreateScaling(x, y, z);
		*this = (*this) * m;
	}
	void ScaleLocal(float x, float y, float z)
	{
		DXMATRIX m;
		m.CreateScaling(x, y, z);
		*this = m * (*this);
	}

	void Inverse()
	{
		*this = XMMatrixInverse(nullptr, *this);
	}


	// ビルポート処理
	void SetBillBoard(const DXMATRIX& mCam)
	{
		// 拡大率取得
		float sX = GetXScale();
		float sY = GetYScale();
		float sZ = GetZScale();
		// カメラの回転をコピー
		r[0].m128_f32[0] = mCam.r[0].m128_f32[0] * sX; // m00
		r[0].m128_f32[1] = mCam.r[0].m128_f32[1] * sX; // m01
		r[0].m128_f32[2] = mCam.r[0].m128_f32[2] * sX; // m02

		r[1].m128_f32[0] = mCam.r[1].m128_f32[0] * sY; // m10
		r[1].m128_f32[1] = mCam.r[1].m128_f32[1] * sY; // m11
		r[1].m128_f32[2] = mCam.r[1].m128_f32[2] * sY; // m12

		r[2].m128_f32[0] = mCam.r[2].m128_f32[0] * sZ; // m20
		r[2].m128_f32[1] = mCam.r[2].m128_f32[1] * sZ; // m21
		r[2].m128_f32[2] = mCam.r[2].m128_f32[2] * sZ; // m22
	}


	//===========================
	// 「取得」系
	//===========================
	const DXVECTOR3& GetPos() const
	{
		return *(DXVECTOR3*)&r[3];
	}

	const DXVECTOR3& GetXAxis() const
	{
		return *(DXVECTOR3*)&r[0];
	}
	const DXVECTOR3& GetYAxis() const
	{
		return *(DXVECTOR3*)&r[1];
	}
	const DXVECTOR3& GetZAxis() const
	{
		return *(DXVECTOR3*)&r[2];
	}

	float GetXScale() const
	{
		return GetXAxis().Length();
	}
	float GetYScale() const
	{
		return GetYAxis().Length();
	}
	float GetZScale() const
	{
		return GetZAxis().Length();
	}

};
