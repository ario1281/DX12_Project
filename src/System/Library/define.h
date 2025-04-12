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
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <d3dcompiler.h>

// DirectX Tex
#pragma comment(lib,"DirectXTex.lib")
#include <DirectXTex.h>

using namespace DirectX;

// DirectX Tool Kit
#pragma comment(lib, "DirectXTK12.lib")
#include <SimpleMath.h>

using Vector2 = SimpleMath::Vector2;
using Vector4 = SimpleMath::Vector4;

//===============================================
//  
//  assimp
//  
//===============================================
//#ifdef _DEBUG
//#pragma comment(lib, "assimp-vc143-mtd.lib")
//#else
//#pragma comment(lib, "assimp-vc143-mt.lib")
//#endif // _DEBUG
//
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>

//===============================================
//  
//  Other
//  
//===============================================

#include "Helper.h"

#include "Math.h"

#include "CameraManager.h"

#include "Direct3D12.h"
#include "Heap.h"
#include "Buffer.h"

#include "SceneManager.h"

#include "Mesh.h"
#include "Model.h"

#include "InputComponent.h"

#include "Shader.h"
