#pragma once
//===============================================
//  
//  Šî–{
//  
//===============================================
#define	SCRW		1280    // Screen Width
#define	SCRH		720     // Screen Height

#pragma comment(lib, "winmm.lib")

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <commctrl.h>
#include <ctime>

#include <wrl/client.h>


//===============================================
//  
//  STL
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


//===============================================
//  
//  DirectX12
//  
//===============================================
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <DirectXMath.h>

#include <d3dcompiler.h>
#include <DirectXTex.h>

#include <SimpleMath.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib, "DirectXTK12.lib")
#pragma comment(lib,"DirectXTex.lib")

using namespace DirectX;
using namespace SimpleMath;


//===============================================
//  
//  Other
//  
//===============================================

#include "Helper.h"


#include "Direct3D12.h"
#include "Heap.h"
#include "Buffer.h"

#include "SceneManager.h"

#include "Mesh.h"
#include "Model.h"

#include "Pipeline.h"
#include "RootSignature.h"
#include "Shader.h"
