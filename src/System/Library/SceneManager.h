#pragma once
#include "Game/SceneBase.h"

class SceneManager
{
public:
	void NowScene()
	{
		if (m_pScene != nullptr) {
			m_pScene->Frame();
		}
	}
	void ChangeScene(SceneBase* newScene)
	{
		if (m_pScene != nullptr) { delete m_pScene; }
		m_pScene = newScene;
	}

	~SceneManager()
	{
		if (m_pScene != nullptr) { delete m_pScene; }
	}

private:
	SceneBase* m_pScene = nullptr;

#pragma region ƒVƒ“ƒOƒ‹ƒgƒ“
public:
	static SceneManager& GetInstance()
	{
		static SceneManager instance;
		return instance;
	}

private:
	SceneManager() {}

#pragma endregion

};

#define SCENE SceneManager::GetInstance()