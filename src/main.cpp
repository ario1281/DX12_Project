#include"main.h"


int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_  HINSTANCE hPrev, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	// メモリリークを知らせる
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM初期化
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// ウィンドウ作成、Direct3D初期化など
	//===================================================================
	if (!APP.Init(hInst, nShowCmd, SCRW, SCRH)) { return -1; }

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


bool System::Init(HINSTANCE _hInst, int _showCmd, int _w, int _h)
{
	// インスタンスハンドル記憶
	APP.m_hInst = _hInst;

	//===================================================================
	//　フルスクリーン確認
	//===================================================================
	bool fullScreen = false;
	long caption = MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2;
	if (MessageBox(nullptr, "フルスクリーンにしますか？", "確認", caption) == IDYES)
	{
		fullScreen = true;
	}

	//===================================================================
	// ウィンドウ作成
	//===================================================================
	if (!m_Window.Create(_hInst, _showCmd, _w, _h, "Window"))
	{
		MessageBox(nullptr, "ウィンドウ作成に失敗", "エラー", MB_OK);
		return false;
	}

	//===================================================================
	// 　Direct3D初期化
	//===================================================================
	if (!D3D.Init(_hInst, m_Window.GetHWnd(), _w, _h, fullScreen))
	{
		MessageBox(m_Window.GetHWnd(), "", "Direct3D初期化失敗", MB_OK | MB_ICONSTOP);
		return false;
	}

	return true;
}

void System::Loop()
{
	DWORD baseTime = timeGetTime();
	int count = 0;

	srand(baseTime);

	m_Scene = new SceneBase();
	SCENE.ChangeScene(m_Scene);

	while (1)
	{
		// ゲーム終了指定があるとき
		if (m_EndFlg) { break; }

		//=========================================
		// ウィンドウ関係の処理
		//=========================================

		// ウィンドウが破棄されてるなら、ループ終了
		if (!m_Window.IsCreated()) { break; }

		// ウィンドウのメッセージを処理する
		m_Window.ProcessMessage();

		//=========================================
		// ゲーム処理の関係
		//=========================================
		// 処理開始時間
		DWORD st = timeGetTime();

		//=========================================
		// ゲーム処理の関係
		//=========================================
		SCENE.NowScene();

		if (m_sceneFlg) {
			SCENE.ChangeScene(m_Scene);
			m_sceneFlg = false;
		}

		//=========================================
		// fps制御
		//=========================================

		// 処理終了時間Get
		DWORD et = timeGetTime();
		// Fps制御
		if (et - st < 16)
		{
			Sleep(16 - (et - st));	// 速すぎたら待つ
		}

		// FPS計測
		count++;
		if (st - baseTime >= 1000)
		{
			m_Fps = (count * 1000) / (st - baseTime);
			baseTime = st;
			count = 0;
		}

		m_FrameCnt++;
	}
}

void System::Release()
{
	m_Window.Release();
}
