#include "SceneBase.h"

void SceneBase::Frame()
{
	D3D.Prepare();

	D3D.GetCSUHeap()->SetHeap();
	D3D.GetCBufferAllocator()->ResetCurrentUseNumber();

	D3D.ScreenFlip();
}
