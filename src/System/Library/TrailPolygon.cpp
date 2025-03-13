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
	for (UINT i = 0; i < m_matList.size(); i++) {
		// 登録行列の参照(ショートカット)
		Matrix& mat = m_matList[i];

		// 2つの頂点の参照(ショートカット)
		VERTEX& v1 = vertex[i * 2];
		VERTEX& v2 = vertex[i * 2 + 1];

		// X方向
		Vector3 axisX = mat.GetXAxis();
		axisX.Normalize();

		// 座標
		v1.pos = mat.GetPos() + axisX * width * 0.5f;
		v1.pos = mat.GetPos() - axisX * width * 0.5f;

		// 色
		v1.color = 0xFFFFFFFF;
		v2.color = 0xFFFFFFFF;

		// UV
		v1.uv = { 0, i / uvCount };
		v2.uv = { 1, i / uvCount };
	}
}

void TrailPolygon::DrawBillBoard(float width)
{
}

void TrailPolygon::DrawStrip()
{
}
