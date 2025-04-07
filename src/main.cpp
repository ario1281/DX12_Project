#include"main.h"


int WINAPI WinMain(_In_ HINSTANCE hInst, _In_opt_  HINSTANCE hPrev, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{

	// ���������[�N��m�点��
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// COM������
	auto hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//===================================================================
	// �E�B���h�E�쐬�ADirect3D�������Ȃ�
	//===================================================================
	if (!APP.Init(hInst, nShowCmd, SCRW, SCRH)) { return -1; }

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


bool System::Init(HINSTANCE _hInst, int _showCmd, int _w, int _h)
{
	// �C���X�^���X�n���h���L��
	APP.m_hInst = _hInst;

	//===================================================================
	//�@�t���X�N���[���m�F
	//===================================================================
	bool fullScreen = false;
	long caption = MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2;
	if (MessageBox(nullptr, "�t���X�N���[���ɂ��܂����H", "�m�F", caption) == IDYES)
	{
		fullScreen = true;
	}

	//===================================================================
	// �E�B���h�E�쐬
	//===================================================================
	if (!m_Window.Create(_hInst, _showCmd, _w, _h, "Window"))
	{
		MessageBox(nullptr, "�E�B���h�E�쐬�Ɏ��s", "�G���[", MB_OK);
		return false;
	}

	//===================================================================
	// �@Direct3D������
	//===================================================================
	if (!D3D.Init(_hInst, m_Window.GetHWnd(), _w, _h, fullScreen))
	{
		MessageBox(m_Window.GetHWnd(), "", "Direct3D���������s", MB_OK | MB_ICONSTOP);
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
		// �Q�[���I���w�肪����Ƃ�
		if (m_EndFlg) { break; }

		//=========================================
		// �E�B���h�E�֌W�̏���
		//=========================================

		// �E�B���h�E���j������Ă�Ȃ�A���[�v�I��
		if (!m_Window.IsCreated()) { break; }

		// �E�B���h�E�̃��b�Z�[�W����������
		m_Window.ProcessMessage();

		//=========================================
		// �Q�[�������̊֌W
		//=========================================
		// �����J�n����
		DWORD st = timeGetTime();

		//=========================================
		// �Q�[�������̊֌W
		//=========================================
		SCENE.NowScene();

		if (m_sceneFlg) {
			SCENE.ChangeScene(m_Scene);
			m_sceneFlg = false;
		}

		//=========================================
		// fps����
		//=========================================

		// �����I������Get
		DWORD et = timeGetTime();
		// Fps����
		if (et - st < 16)
		{
			Sleep(16 - (et - st));	// ����������҂�
		}

		// FPS�v��
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
