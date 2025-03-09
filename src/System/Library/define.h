#pragma once
//===============================================
//　
//　基本
//　
//===============================================
#define	SCRW		1280	// ウィンドウ幅（Width
#define	SCRH		720		// ウィンドウ高さ（Height

#pragma comment(lib, "winmm.lib")

#include<windows.h>
#include<stdio.h>
#include<math.h>
#include<commctrl.h>
#include<ctime>

#include<wrl/client.h>

using namespace std;


//===============================================
//　
//　DirectX12
//　
//===============================================
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <d3d12.h>
#include <dxgi1_6.h>

#pragma comment(lib, "DirectXTK12.lib")
#include <SimpleMath.h>

namespace Math = DirectX::SimpleMath;


//===============================================
//　
//　STL
//　
//===============================================
#include <map>
#include <unordered_map>
#include <unordered_set>
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
#include <fileSystem>
#include <chrono>


/*===============================================
//　
//　その他
//　
//===============================================*/
#include "Helper.h"

#include "Heap/RTVHeap.h"

#include "GraphicsDevice.h"

