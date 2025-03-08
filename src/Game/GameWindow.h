#pragma once

class GameWindow {
public:
	HWND GetHWnd() const { return m_hWnd; }

	 bool IsCreated() const { return m_hWnd ? true : false; }

	int GetMouseWheelVal()const { return m_MouseWheelVal; }
	bool GetAnyKeyFlag()const { return m_AnyKeyFlg; }
	bool GetMouseFlag()const { return m_MouseFlg; }

	bool Create(HINSTANCE _hInst, int _showCmd, int clientWidth, int clientHeight, const std::string& _wndName);

	void Release();

	bool ProcessMessage();

	void SetClientSize(int w, int h);

	~GameWindow() { Release(); }

private:
	HWND	m_hWnd = nullptr;
	bool	m_AnyKeyFlg = false;
	bool	m_MouseFlg = false;
	int		m_MouseWheelVal = 0;

	static LRESULT CALLBACK callWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

};