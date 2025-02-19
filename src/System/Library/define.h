#pragma once
/*===============================================

	基本

===============================================*/
#define	SCRW		1280	// ウィンドウ幅（Width
#define	SCRH		720		// ウィンドウ高さ（Height

#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <commctrl.h>

using namespace std;
using namespace Microsoft::WRL;

/*################################################

	DirectX

################################################*/
#include<d3d12.h>
#include<d3dx12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>

#include<d3dcompiler.h>
#include<DirectXTex.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

#pragma comment(lib, "DirectXTex.lib")

using namespace DirectX;

using XMVECTOR3 = XMFLOAT3;
using XMVECTOR2 = XMFLOAT2;

/*===============================================

	STL

===============================================*/
#include <ctime>
#include <map>
#include <unordered_map>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include <list>
#include <iterator>
#include <queue>
#include <algorithm>
#include <memory>
#include <random>
#include <fstream>
#include <iostream>
#include <sstream>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

/*===============================================

	その他ライブラリ

===============================================*/

#include"Helper.h"

#include"Direct3D12.h"

/*===============================================

	static関数

===============================================*/
