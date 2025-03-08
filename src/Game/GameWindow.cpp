#include "main.h"
#include"GameWindow.h"

bool GameWindow::Create(HINSTANCE _hInst, int _showCmd, int _w, int _h, const string& _wndName)
{
	/*===================================================================
		���C���E�B���h�E�쐬
	===================================================================*/

#pragma region �E�B���h�E�N���X�̒�`

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
		"3D�Q�[���v���O���~���O",
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
		// �I�����b�Z�[�W������
		if (msg.message == WM_QUIT) { return false; }

		//���b�Z�[�W����
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

void GameWindow::SetClientSize(int _w, int _h)
{
	RECT rcWnd, rcCli;

	GetWindowRect(m_hWnd, &rcWnd); // �E�B���h�E��RECT�擾
	GetClientRect(m_hWnd, &rcCli); // �N���C�A���g�̈��RECT�擾

	// �E�B���h�E�̗]�����l���āA�N���C�A���g�̃T�C�Y���w��T�C�Y�ɂ���B
	MoveWindow(m_hWnd,
		rcWnd.left,// X���W
		rcWnd.top,// Y���W
		_w + (rcWnd.right - rcWnd.left) - (rcCli.right - rcCli.left),
		_h + (rcWnd.bottom - rcWnd.top) - (rcCli.bottom - rcCli.top),
		TRUE
	);
}

LRESULT GameWindow::callWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// �E�B���h�E�v���p�e�B����AGameWindow�N���X�̃C���X�^���X���擾
	GameWindow* pThis = (GameWindow*)GetProp(hWnd, "GameWindowInstance");

	// nullptr�̏ꍇ�́A�f�t�H���g���������s
	if (pThis == nullptr) {
		switch (msg) {
		case WM_CREATE:
		{
			// CreateWindow()�œn�����p�����[�^���擾
			CREATESTRUCT* createStruct = (CREATESTRUCT*)lParam;
			GameWindow* window = (GameWindow*)createStruct->lpCreateParams;

			// �E�B���h�E�v���p�e�B�ɂ��̃N���X�̃C���X�^���X�A�h���X�𖄂ߍ���ł���
			// ���񂩂�ApThis->WindowProc�̕��֏���������Ă���
			SetProp(hWnd, "GameWindowInstance", window);
			return 0;
		}
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	}

	// �C���X�^���X����Window�֐������s����
	return pThis->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT GameWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	//===================================
	//���b�Z�[�W�ɂ���ď�����I��
	//===================================
	m_AnyKeyFlg = false;
	m_MouseFlg = false;
	switch (msg)
	{
#pragma region Alt�{�L�[�������ꂽ
	case WM_SYSKEYDOWN:
		m_AnyKeyFlg = true;
		switch (wParam)
		{
		case VK_RETURN:		// ALT+Enter�Ńt���X�N���[���؂�ւ�

			break;

#if _DEBUG
		case VK_ESCAPE:	//�@ESC�L�[�ŏI��
			Release();
			return 0;
#endif // _DEBUG


		default:
			// ���b�Z�[�W�̃f�t�H���g����
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
#pragma endregion

#pragma region �L�[�������ꂽ
	case WM_KEYDOWN:
		m_AnyKeyFlg = true;
		switch (wParam)
		{
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		break;
#pragma endregion

		// �z�C�[���X�N���[����
	case WM_MOUSEWHEEL:
		m_MouseWheelVal = (short)HIWORD(wParam);
		break;

		// X�{�^���������ꂽ
	case WM_CLOSE:
		// �j��
		Release();
		break;

		// �E�B���h�E�j�����O
	case WM_DESTROY:
		RemoveProp(hWnd, "GameWindowInstance");
		PostQuitMessage(0);
		break;

	default:
		// ���b�Z�[�W�̃f�t�H���g����
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
