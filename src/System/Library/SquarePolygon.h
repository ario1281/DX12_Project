#pragma once

class SquarePolygon
{
public:
	/// <summary>
	/// �|���S���ݒ�
	/// </summary>
	/// <param name="w">��</param>
	/// <param name="h">����</param>
	/// <param name="">�F</param>
	void Init(float w, float h, DWORD color);

	/// <summary>
	/// �`��
	/// </summary>
	void Draw();

	/// <summary>
	/// �A�j���[�V�����̕�������ݒ�
	/// </summary>
	void SetAnimationInfo(int splitX, int splitY)
	{
		m_anmSplitX = splitX;
		m_anmSplitY = splitY;
	}

private:
	// 1�̒��_�f�[�^
	struct VERTEX
	{
		DXVECTOR3 pos;   // ���W
		DWORD     color; // �F
		XMFLOAT2  uv;    // UV���W
	};
	// ���_�z��
	VERTEX m_vertex[4];

	// �A�j���[�V�����֌W
	int   m_anmSplitX = 1; // ���̕���
	int   m_anmSplitY = 1; // �c�̕���

	float m_anmPos    = 0; // ���݂̃R�}�ʒu
};