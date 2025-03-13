#pragma once

//===============================
// 軌跡ポリゴンクラス
//===============================
class TrailPolygon
{
public:

#pragma region 行列を追加
	// 先頭
	void AddMatrix_Front(const Matrix& mat)
	{
		m_matList.push_front(mat);
	}

	// 末尾
	void AddMatrix_Back(const Matrix& mat)
	{
		m_matList.push_back(mat);
	}

#pragma endregion

#pragma region 行列を削除
	// 先頭
	void DelMatrix_Front()
	{
		m_matList.pop_front();
	}

	// 末尾
	void DelMatrix_Back()
	{
		m_matList.pop_back();
	}

#pragma endregion

	// リストの数 取得
	int GetNum() const {
		return (int)m_matList.size();
	}


#pragma region 描画

	void Draw(float width);

	void DrawBillBoard(float width);

	void DrawStrip();

#pragma endregion

private:
	deque<Matrix> m_matList;
};