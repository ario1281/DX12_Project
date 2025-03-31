#include "System/Library/define.h"
#include "SceneBase.h"
#include "GameObject.h"

void SceneBase::Frame()
{
	if (!Update()) { return; }

	D3D.Prepare();

	D3D.GetCSUHeap()->SetHeap();
	D3D.GetCBufferAllocator()->ResetCurrentUseNumber();

	D3D.GetCBufferAllocator()->ResetCurrentUseNumber();

	Render3D();

	D3D.ScreenFlip();
}

shared_ptr<GameObject> SceneBase::FindObjectWithTag(const std::string& tag)
{
	for (auto&& obj : m_objects)
	{
		if (obj == nullptr) { continue; }
		if (obj->GetTag() == tag) { return obj; }
	}
	return nullptr;
}
