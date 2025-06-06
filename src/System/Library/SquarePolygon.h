#pragma once

class SquarePolygon
{
public:
	/// <summary>
	/// ポリゴン設定
	/// </summary>
	/// <param name="w">幅</param>
	/// <param name="h">高さ</param>
	/// <param name="">色</param>
	void Init(float w, float h, DWORD color);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// アニメーションの分割数を設定
	/// </summary>
	void SetAnimationInfo(int splitX, int splitY)
	{
		m_anmSplitX = splitX;
		m_anmSplitY = splitY;
	}

private:
	// 1つの頂点データ
	struct VERTEX
	{
		Vector3   pos;     // 座標
		DWORD     color;   // 色
		XMFLOAT2  uv;      // UV座標
	};
	// 頂点配列
	VERTEX m_vertex[4];

	com_ptr<ID3D12Resource>  m_pVBuffer = nullptr;
	com_ptr<ID3D12Resource>  m_pIBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW  m_ibView;

	// アニメーション関係
	int   m_anmSplitX = 1; // 横の分割
	int   m_anmSplitY = 1; // 縦の分割

	float m_anmPos    = 0; // 現在のコマ位置
};