#include "SceneBase.h"

void SceneBase::Frame()
{
	D3D.Prepare();

	D3D.GetCSUHeap()->GetHeap();


	D3D.ScreenFlip();
}
