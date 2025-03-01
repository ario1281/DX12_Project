#include"main.h"


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdParam, int nCmdShow)
{
	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// ウィンドウ作成、Direct3D初期化など
	//===================================================================
	if (!APP.Init(hInst, nCmdShow, SCRW, SCRH)) { return 0; }

	//===================================================================
	// ループ
	//===================================================================
	//APP.Loop();

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
	bool fullScreen = false;
	if (MessageBox(nullptr,"フルスクリーンにしますか？", "確認", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		fullScreen = true;
	}

	//===================================================================
	// ウィンドウ作成
	//===================================================================
	if (!m_Window.Create(_hInst, _w, _h, _cmdShow, "Window")) {
		MessageBox(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	// 　Direct3D初期化
	//===================================================================
	if (!DX12.Init(_hInst, m_Window.GetHWnd(), _w, _h, fullScreen)) {
		MessageBox(m_Window.GetHWnd(), "Direct3D初期化失敗", "エラー", MB_OK | MB_ICONSTOP);
		return false;
	}

	return true;
}

void System::Loop()
{
	DWORD baseTime = timeGetTime();
	int count = 0;

	while (1)
	{
		// ゲーム終了指定があるとき
		if (m_EndFlg) { break; }

		//=========================================
		//
		// ウィンドウ関係の処理
		//
		//=========================================

		// ウィンドウが破棄されてるなら、ループ終了
		if (!m_Window.IsCreated()) { break; }

		// ウィンドウのメッセージを処理する
		m_Window.ProcessMessage();

		//=========================================
		//
		// ゲーム処理の関係
		//
		//=========================================

		// 処理開始時間
		DWORD st = timeGetTime();


		//=========================================
		// シーン処理
		//=========================================



		//=========================================
		// fps制御
		//=========================================

		// 処理終了時間Get
		DWORD et = timeGetTime();
		// Fps制御
		if (et - st < 16) {
			Sleep(16 - (et - st));	// 速すぎたら待つ
		}

		// FPS計測
		count++;
		if (st - baseTime >= 1000) {
			m_Fps = (count * 1000) / (st - baseTime);
			baseTime = st;
			count = 0;
		}

		m_FrameCnt++;
	}
}

void System::Release()
{
}
