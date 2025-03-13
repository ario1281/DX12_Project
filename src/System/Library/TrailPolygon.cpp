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
	for (UINT i = 0; i < m_matList.size(); i++) {
		// �o�^�s��̎Q��(�V���[�g�J�b�g)
		Matrix& mat = m_matList[i];

		// 2�̒��_�̎Q��(�V���[�g�J�b�g)
		VERTEX& v1 = vertex[i * 2];
		VERTEX& v2 = vertex[i * 2 + 1];

		// X����
		Vector3 axisX = mat.GetXAxis();
		axisX.Normalize();

		// ���W
		v1.pos = mat.GetPos() + axisX * width * 0.5f;
		v1.pos = mat.GetPos() - axisX * width * 0.5f;

		// �F
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
