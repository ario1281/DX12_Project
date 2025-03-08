#pragma once

#include <windows.h>
#include <stdio.h>

//===========================
// define.h
//===========================
#include"System/Library/define.h"

//===========================
// その他
//===========================
#include"Game\GameWindow.h"

class System
{
public:
	bool Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h);
	void Loop();
	void Release();
	void EndGame() { m_EndFlg = true;  }

	//=================================================
	// 公開データ
	//=================================================

	// ゲームウィンドウクラス
	GameWindow m_Window;

	//=================================================
	// グローバルデータ
	//=================================================

	HINSTANCE	m_hInst = nullptr;	// インスタンスハンドル
	DWORD		m_FrameCnt = 0;		// 現在フレーム地
	int			m_Fps = 0;			// FPS値

private:

	// ゲーム終了フラグ
	bool m_EndFlg = false;
	bool m_sceneFlg = false;


#pragma region シングルトン
public:
	static System& GetInstance()
	{
		static System instance;
		return instance;
	}

private:
	System() {};

#pragma endregion

};

#define APP System::GetInstance()