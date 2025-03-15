#include "Math.h"

void DXVECTOR3::TransNormal(const DXMATRIX& m)
{
	auto v = XMVector3TransformNormal(XMLoadFloat3(this), m);
	XMStoreFloat3(this, v);
}

void DXVECTOR3::TransCoord(const DXMATRIX& m)
{
	auto v = XMVector3TransformCoord(XMLoadFloat3(this), m);
	XMStoreFloat3(this, v);
}
