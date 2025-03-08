#include "main.h"
#include"GameWindow.h"

bool GameWindow::Create(HINSTANCE _hInst, int _showCmd, int _w, int _h, const string& _wndName)
{
	/*===================================================================
		メインウィンドウ作成
	===================================================================*/

#pragma region ウィンドウクラスの定義

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = &GameWindow::callWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = _hInst;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = _wndName.c_str();

#pragma endregion

	if (!RegisterClassEx(&wc)) { return false; }

	m_hWnd = CreateWindow(
		wc.lpszClassName,
		"3Dゲームプログラミング",
		WS_OVERLAPPEDWINDOW - WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT, _w, _h,
		nullptr,
		nullptr,
		_hInst,
		this
	);

	if (m_hWnd == nullptr) { return false; }

	SetClientSize(_w, _h);

	ShowWindow(m_hWnd, _showCmd);

	UpdateWindow(m_hWnd);

	timeBeginPeriod(1);

	return true;
}

void GameWindow::Release()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = nullptr;
	}
}

bool GameWindow::ProcessMessage()
{
	m_MouseWheelVal = 0;

	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		// 終了メッセージがきた
		if (msg.message == WM_QUIT) { return false; }

		//メッセージ処理
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

void GameWindow::SetClientSize(int _w, int _h)
{
	RECT rcWnd, rcCli;

	GetWindowRect(m_hWnd, &rcWnd); // ウィンドウのRECT取得
	GetClientRect(m_hWnd, &rcCli); // クライアント領域のRECT取得

	// ウィンドウの余白を考えて、クライアントのサイズを指定サイズにする。
	MoveWindow(m_hWnd,
		rcWnd.left,// X座標
		rcWnd.top,// Y座標
		_w + (rcWnd.right - rcWnd.left) - (rcCli.right - rcCli.left),
		_h + (rcWnd.bottom - rcWnd.top) - (rcCli.bottom - rcCli.top),
		TRUE
	);
}

LRESULT GameWindow::callWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// ウィンドウプロパティから、GameWindowクラスのインスタンスを取得
	GameWindow* pThis = (GameWindow*)GetProp(hWnd, "GameWindowInstance");

	// nullptrの場合は、デフォルト処理を実行
	if (pThis == nullptr) {
		switch (msg) {
		case WM_CREATE:
		{
			// CreateWindow()で渡したパラメータを取得
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			GameWindow* window = (GameWindow*)createStruct->lpCreateParams;

			// ウィンドウプロパティにこのクラスのインスタンスアドレスを埋め込んでおく
			// 次回から、pThis->WindowProcの方へ処理が流れていく
			SetProp(hWnd, "GameWindowInstance", window);
			return 0;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	// インスタンス側のWindow関数を実行する
	return pThis->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT GameWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	//===================================
	//メッセージによって処理を選択
	//===================================
	m_AnyKeyFlg = false;
	m_MouseFlg = false;
	switch (msg)
	{
#pragma region Alt＋キーが押された
	case WM_SYSKEYDOWN:
		m_AnyKeyFlg = true;
		switch (wParam)
		{
		case VK_RETURN:		// ALT+Enterでフルスクリーン切り替え

			break;

#if _DEBUG
		case VK_ESCAPE:	//　ESCキーで終了
			Release();
			return 0;
#endif // _DEBUG


		default:
			// メッセージのデフォルト処理
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
#pragma endregion

#pragma region キーが押された
	case WM_KEYDOWN:
		m_AnyKeyFlg = true;
		switch (wParam)
		{
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
#pragma endregion

		// ホイールスクロール時
	case WM_MOUSEWHEEL:
		m_MouseWheelVal = (short)HIWORD(wParam);
		break;

		// Xボタンが押された
	case WM_CLOSE:
		// 破棄
		Release();
		break;

		// ウィンドウ破棄直前
	case WM_DESTROY:
		RemoveProp(hWnd, "GameWindowInstance");
		PostQuitMessage(0);
		break;

	default:
		// メッセージのデフォルト処理
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
