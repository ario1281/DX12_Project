#pragma once

#include <windows.h>
#include <stdio.h>

//===========================
// define.h
//===========================
#include"System/Library/define.h"

//===========================
// ���̑�
//===========================
#include"Game/GameWindow.h"


class System
{
public:
	bool Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h);
	void Loop();
	void Release();

	void EndGame() { m_EndFlg = true; }

	#pragma region �擾�n

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
	// ���J�f�[�^
	//=================================================

	// �Q�[���E�B���h�E�N���X
	GameWindow m_Window;
	// �V�[���N���X
	SceneBase* m_Scene = nullptr;

	//=================================================
	// �O���[�o���f�[�^
	//=================================================

	HINSTANCE  m_hInst    = nullptr; // �C���X�^���X�n���h��
	DWORD      m_FrameCnt = 0;       // ���݃t���[���n
	int        m_Fps      = 0;       // FPS�l

private:

	// �Q�[���I���t���O
	bool m_EndFlg   = false;
	bool m_sceneFlg = false;


#pragma region �V���O���g��
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