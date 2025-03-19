#include "GameObject.h"

void GameObject::Draw2D()
{
	if (!m_spTex) { return; }
}

void GameObject::Draw3D()
{
	if (!m_spModel) { return; }

	m_spModel->Draw();
}

void GameObject::SetModel(const std::string& name)
{
}
