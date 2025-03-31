#include "GameScene.h"

#include "Game\Object\CameraComponent.h"

void GameScene::Start()
{
	auto spCam = std::make_shared<CameraComponent>();
	m_objects.push_back(spCam);
}

bool GameScene::Update()
{
	return false;
}

void GameScene::Render2D()
{
}

void GameScene::Render3D()
{
}
