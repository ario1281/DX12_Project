#include"main.h"


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdParam, int nCmdShow)
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// ウィンドウ作成、Direct3D初期化など
	//===================================================================
	if (APP.Init(hInst, nCmdShow, SCRW, SCRH) == false) { return 0; }

	//===================================================================
	// ループ
	//===================================================================
	APP.Loop();

	//===================================================================
	// 解放
	//===================================================================
	APP.Release();

	// COM解放
	CoUninitialize();

	return 0;
}

bool System::Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h)
{
	// インスタンスハンドル記憶
	APP.m_hInst = _hInst;

	//===================================================================
	//　フルスクリーン確認
	//===================================================================
	bool bFullScreen = false;
	if (MessageBox(nullptr,"フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		bFullScreen = true;
	}

	//===================================================================
	// ウィンドウ作成
	//===================================================================
	if (m_Window.Create(_hInst, _w, _h, _cmdShow, "Window") == false) {
		MessageBox(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	// 　Direct3D初期化
	//===================================================================
	if (DX12.Init(_hInst, m_Window.GetHWnd(), _h, _w) == false) {
		MessageBox(m_Window.GetHWnd(), "Direct3D初期化失敗", "エラー", MB_OK | MB_ICONSTOP);
		return false;
	}

	return true;
}

void System::Loop()
{
}

void System::Release()
{
}
