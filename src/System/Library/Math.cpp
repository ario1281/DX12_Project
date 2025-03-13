#include "Math.h"

void Vector3::TransNormal(const Matrix& m)
{
	auto v = XMVector3TransformNormal(XMLoadFloat3(this), m);
	XMStoreFloat3(this, v);
}

void Vector3::TransCoord(const Matrix& m)
{
	auto v = XMVector3TransformCoord(XMLoadFloat3(this), m);
	XMStoreFloat3(this, v);
}
