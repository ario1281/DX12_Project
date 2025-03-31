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
#include"Game/GameWindow.h"


class System
{
public:
	bool Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h);
	void Loop();
	void Release();

	void EndGame() { m_EndFlg = true; }

	#pragma region 取得系

	const HWND& GetHWnd() const { return m_Window.GetHWnd(); }
	const bool GetAnyKeyFlag() const { return m_Window.GetAnyKeyFlag(); }
	const bool GetMouseFlag() const { return m_Window.GetMouseFlag(); }
	bool GetSceneFlag() { return m_sceneFlg; }
	void SetSceneFlag() { m_sceneFlg = false; }

	void SetChangeScene(SceneBase* s_scene) {
		m_sceneFlg = true;
		m_Scene = s_scene;
	}

	#pragma endregion

	//=================================================
	// 公開データ
	//=================================================

	// ゲームウィンドウクラス
	GameWindow m_Window;
	// シーンクラス
	SceneBase* m_Scene = nullptr;

	//=================================================
	// グローバルデータ
	//=================================================

	HINSTANCE  m_hInst    = nullptr; // インスタンスハンドル
	DWORD      m_FrameCnt = 0;       // 現在フレーム地
	int        m_Fps      = 0;       // FPS値

private:

	// ゲーム終了フラグ
	bool m_EndFlg   = false;
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