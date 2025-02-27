#include"main.h"


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdParam, int nCmdShow)
{
	// ���������[�N��m�点��
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM������
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// �E�B���h�E�쐬�ADirect3D�������Ȃ�
	//===================================================================
	if (APP.Init(hInst, nCmdShow, SCRW, SCRH) == false) { return 0; }

	//===================================================================
	// ���[�v
	//===================================================================
	APP.Loop();

	//===================================================================
	// ���
	//===================================================================
	APP.Release();

	// COM���
	CoUninitialize();

	return 0;
}

bool System::Init(HINSTANCE _hInst, int _cmdShow, int _w, int _h)
{
	// �C���X�^���X�n���h���L��
	APP.m_hInst = _hInst;

	//===================================================================
	//�@�t���X�N���[���m�F
	//===================================================================
	bool bFullScreen = false;
	if (MessageBox(nullptr,"�t���X�N���[���ɂ��܂����H", "�m�F", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		bFullScreen = true;
	}

	//===================================================================
	// �E�B���h�E�쐬
	//===================================================================
	if (m_Window.Create(_hInst, _w, _h, _cmdShow, "Window") == false) {
		MessageBox(nullptr, "�E�B���h�E�쐬�Ɏ��s", "�G���[", MB_OK);
		return false;
	}

	//===================================================================
	// �@Direct3D������
	//===================================================================
	if (DX12.Init(_hInst, m_Window.GetHWnd(), _h, _w) == false) {
		MessageBox(m_Window.GetHWnd(), "Direct3D���������s", "�G���[", MB_OK | MB_ICONSTOP);
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
