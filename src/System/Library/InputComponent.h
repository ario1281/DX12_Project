#pragma once
#include "main.h"

class InputComponent
{
public:
	enum class SystemCode
	{
		Bt_Right = 0,
		Bt_Left,
		Bt_Up,
		Bt_Down,

		Bt_1,
		Bt_2,
		Bt_Esc,


		ButtonNum
	};

	enum class Type
	{
		NONE,
		DOWN,
		KEEP,
		UP
	};

	int Button(int no) { return m_buttons[no]; }
	Vector3 Position() { return m_vMove; }
	float Vartical() { return m_Vartical; }
	float Horizontal() { return m_Horizontal; }

	void Push(unsigned int no) {
		if (no >= m_buttons.size()) { return; }
		if (m_buttons[no] == (UINT)Type::NONE) { m_buttons[no] = (UINT)Type::DOWN; }
		else if (m_buttons[no] == (UINT)Type::DOWN) { m_buttons[no] = (UINT)Type::KEEP; }
	}
	void Release(unsigned int no) {
		if (no >= m_buttons.size()) { return; }
		if (m_buttons[no] == (UINT)Type::DOWN || m_buttons[no] == (UINT)Type::KEEP) {
			m_buttons[no] = (UINT)Type::UP;
		}
		else {
			m_buttons[no] = (UINT)Type::NONE;
		}
	}

	virtual void Update() {}

protected:
	std::array<size_t, (UINT)SystemCode::ButtonNum> m_buttons;

	Vector3 m_vMove      = { 0,0,0 };
	float   m_Vartical   = 0.0f;
	float   m_Horizontal = 0.0f;
};

class IC_MouseAndKey : public InputComponent
{
public:
	void Update()override;

private:
	std::array<size_t, (UINT)SystemCode::ButtonNum> m_keyCode = {
	'D',
	'A',
	'W',
	'S',

	VK_LBUTTON,
	VK_RBUTTON,

	VK_ESCAPE,
	};
};

class Cursor
{
public:
	void Init() {
		ClientToScreen(APP.GetHWnd(), &m_pBase);
		SetCursorPos(m_pBase.x, m_pBase.y);
	}

	void SetPos() { SetCursorPos(m_pBase.x, m_pBase.y); }

	void Update();

	/*----------------------------------------
	* 四角形の当たり判定
	 x		…　左上のx座標
	 y		…　左上のy座標
	 w		…　横幅の長さ
	 h		…　縦幅の高さ
	----------------------------------------*/
	bool IsRange(float x, float y, float w, float h);

	float Horizontal() { return m_horizontal; }
	float Vertical() { return m_vertical; }


	~Cursor() {}
private:
	POINT m_pBase = { (LONG)(SCRW / 2.0f),(LONG)(SCRH / 2.0f) };

	float m_horizontal = 0.0f;
	float m_vertical = 0.0f;

	//　スピード調整
	const float m_sCtrl = 7.5f;
	const float m_rAng = 60.0f;

#pragma region シングルトン
public:
	static Cursor& GetInstance() {
		static Cursor Instance;
		return Instance;
	}
private:
	Cursor() {}
#pragma endregion

};

#define CURSOR Cursor::GetInstance()