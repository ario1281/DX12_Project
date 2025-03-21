#include "SceneBase.h"

void SceneBase::Frame()
{
	D3D.Prepare();

	D3D.ScreenFlip();
}
