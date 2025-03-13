#pragma once

//===============================
// �O�Ճ|���S���N���X
//===============================
class TrailPolygon
{
public:

#pragma region �s���ǉ�
	// �擪
	void AddMatrix_Front(const Matrix& mat)
	{
		m_matList.push_front(mat);
	}

	// ����
	void AddMatrix_Back(const Matrix& mat)
	{
		m_matList.push_back(mat);
	}

#pragma endregion

#pragma region �s����폜
	// �擪
	void DelMatrix_Front()
	{
		m_matList.pop_front();
	}

	// ����
	void DelMatrix_Back()
	{
		m_matList.pop_back();
	}

#pragma endregion

	// ���X�g�̐� �擾
	int GetNum() const {
		return (int)m_matList.size();
	}


#pragma region �`��

	void Draw(float width);

	void DrawBillBoard(float width);

	void DrawStrip();

#pragma endregion

private:
	deque<Matrix> m_matList;
};