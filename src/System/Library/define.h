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

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#pragma comment(lib,"DirectXTex.lib")

using namespace DirectX;


//===============================================
//  
//  Helper
//  
//===============================================
template<class T>
using com_ptr    = Microsoft::WRL::ComPtr<T>;

template<class T>
using shared_ptr = std::shared_ptr<T>;

template<class T>
using unique_ptr = std::unique_ptr<T>;


//===============================================
//  
//  Other
//  
//===============================================

#include "Math.h"
#include "Direct3D12.h"

#include "TextureManager.h"
#include "MeshManager.h"

#include "Heap.h"
