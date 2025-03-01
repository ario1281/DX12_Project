#include"main.h"


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdParam, int nCmdShow)
{
	// ���������[�N��m�点��
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM������
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// �E�B���h�E�쐬�ADirect3D�������Ȃ�
	//===================================================================
	if (!APP.Init(hInst, nCmdShow, SCRW, SCRH)) { return 0; }

	//===================================================================
	// ���[�v
	//===================================================================
	//APP.Loop();

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
	bool fullScreen = false;
	if (MessageBox(nullptr,"�t���X�N���[���ɂ��܂����H", "�m�F", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES) {
		fullScreen = true;
	}

	//===================================================================
	// �E�B���h�E�쐬
	//===================================================================
	if (!m_Window.Create(_hInst, _w, _h, _cmdShow, "Window")) {
		MessageBox(nullptr, "�E�B���h�E�쐬�Ɏ��s", "�G���[", MB_OK);
		return false;
	}

	//===================================================================
	// �@Direct3D������
	//===================================================================
	if (!DX12.Init(_hInst, m_Window.GetHWnd(), _w, _h, fullScreen)) {
		MessageBox(m_Window.GetHWnd(), "Direct3D���������s", "�G���[", MB_OK | MB_ICONSTOP);
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
		// �Q�[���I���w�肪����Ƃ�
		if (m_EndFlg) { break; }

		//=========================================
		//
		// �E�B���h�E�֌W�̏���
		//
		//=========================================

		// �E�B���h�E���j������Ă�Ȃ�A���[�v�I��
		if (!m_Window.IsCreated()) { break; }

		// �E�B���h�E�̃��b�Z�[�W����������
		m_Window.ProcessMessage();

		//=========================================
		//
		// �Q�[�������̊֌W
		//
		//=========================================

		// �����J�n����
		DWORD st = timeGetTime();


		//=========================================
		// �V�[������
		//=========================================



		//=========================================
		// fps����
		//=========================================

		// �����I������Get
		DWORD et = timeGetTime();
		// Fps����
		if (et - st < 16) {
			Sleep(16 - (et - st));	// ����������҂�
		}

		// FPS�v��
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
