#include "TrailPolygon.h"

void TrailPolygon::Draw(float width)
{
	if (m_matList.size() < 2) { return; }

	// 
	float uvCount = (float)(m_matList.size() - 1);

	// 1�̒��_�f�[�^
	struct VERTEX
	{
		XMFLOAT3 pos;   // ���W
		DWORD    color; // �F
		XMFLOAT2 uv;    // UV���W
	};

	vector<VERTEX> vertex;
	vertex.resize(m_matList.size() * 2);

	//--------------------------
	// ���_�f�[�^�쐬
	//--------------------------
	for (UINT i = 0; i < m_matList.size(); i++)
	{
	}
}

void TrailPolygon::DrawBillBoard(float width)
{
}

void TrailPolygon::DrawStrip()
{
}
