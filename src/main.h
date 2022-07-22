#pragma once
#include <windows.h>
#include <stdio.h>

#include"Game\GameWindow.h"

class System
{
public:
	bool System::Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h);
	void Loop();
	void Release();
	void EndGame() { m_EndFlg = true;  }

	/*=================================================
		���J�f�[�^
	=================================================*/
	//�@�Q�[���E�B���h�E�N���X
	GameWindow m_Window;

	/*=================================================
		�O���[�o���f�[�^
	=================================================*/
	HINSTANCE	m_hInst = nullptr;
	DWORD		m_FrameCnt = 0;
	int			m_Fps = 0;

private:
	bool m_EndFlg = false;
	bool m_sceneFlg = false;

public:
	static System& GetInstance() {
		static System instance;
		return instance;
	}

private:
	System() {};
	~System() {};
};

#define APP System::GetInstance()