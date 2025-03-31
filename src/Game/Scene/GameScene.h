#pragma once

class GameScene : public SceneBase
{
public:
	GameScene() { Start(); }
	~GameScene() {}

	void Start()	override;
	bool Update()	override;
	void Render2D()	override;
	void Render3D()	override;

private:
	Matrix      m_mWorld;
	const float m_Rotate = 0.0625f;

	bool m_pauseFlg = false;
	bool m_clearFlg = false;
};