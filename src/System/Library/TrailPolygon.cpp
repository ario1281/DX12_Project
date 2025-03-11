#include "TrailPolygon.h"

void TrailPolygon::Draw(float width)
{
	if (m_matList.size() < 2) { return; }

	// 
	float uvCount = (float)(m_matList.size() - 1);

	// 1つの頂点データ
	struct VERTEX
	{
		XMFLOAT3 pos;   // 座標
		DWORD    color; // 色
		XMFLOAT2 uv;    // UV座標
	};

	vector<VERTEX> vertex;
	vertex.resize(m_matList.size() * 2);

	//--------------------------
	// 頂点データ作成
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
