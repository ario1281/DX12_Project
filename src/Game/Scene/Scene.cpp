#include "Scene.h"

#include "Game\Object\CameraComponent.h"

void Scene::Start()
{
	auto spCam = std::make_shared<CameraComponent>();
	m_objects.push_back(spCam);
}

bool Scene::Update()
{
	return true;
}

void Scene::Render2D()
{
	for (auto&& obj : m_objects) { obj->Draw2D(); }
}

void Scene::Render3D()
{
	for (auto&& obj : m_objects) { obj->Draw3D(); }
}
