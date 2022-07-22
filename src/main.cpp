#include"System\Library\define.h"
#include"main.h"

#pragma region 構造体宣言

struct SceneData
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
	XMFLOAT3 look;
};

#pragma region パッキング
#pragma pack(1)		//　1バイトパッキング
struct PMDMaterial
{
	XMVECTOR3			diffuse;		//　ディフューズ色
	float				alpha;			//　ディフューズα
	float				specularity;	//　スペキュラの強さ(乗算値)
	XMVECTOR3			specular;		//　スペキュラ色
	XMVECTOR3			ambient;		//　アンビエント色
	unsigned char		toonIdx;		//　トゥーン番号(後述)
	unsigned char		edgeFlg;		//　マテリアル毎の輪郭線フラグ

	//　2バイトのパディングが発生！！

	unsigned int		indicesNum;		//　このマテリアルが割り当たるインデックス数
	char				texFilePath[20];//　テクスチャファイル名(プラスアルファ…後述)
};
#pragma pack()
#pragma endregion

struct MaterialForHlsl {
	XMVECTOR3			diffuse;
	float				alpha;
	XMVECTOR3			specular;
	float				specularity;
	XMVECTOR3			ambient;
};

struct AdditionalMaterial {
	std::string		texPath;
	int					toonIdx;
	bool				sdgeFlg;
};

struct Material {
	unsigned int		indicesNum;
	MaterialForHlsl		material;
	AdditionalMaterial	additional;
};

#pragma endregion

#pragma region グローバル変数

using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;
std::map <std::string, LoadLambda_t> loadLambdaTable;

std::map<std::string, ID3D12Resource*> _resTable;

std::string strModelPath("model/初音ミク.pmd");

std::string vs_Path = "data/Shader/BasicVertexShader.hlsl";
std::string ps_Path = "data/Shader/BasicPixelShader.hlsl";

const int matsCnt = 5;

#pragma endregion



///モデルのパスとテクスチャのパスから合成パスを得る
///@param modelPath アプリケーションから見たpmdモデルのパス
///@param texPath PMDモデルから見たテクスチャのパス
///@return アプリケーションから見たテクスチャのパス
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
	//ファイルのフォルダ区切りは\と/の二種類が使用される可能性があり
	//ともかく末尾の\か/を得られればいいので、双方のrfindをとり比較する
	//int型に代入しているのは見つからなかった場合はrfindがepos(-1→0xffffffff)を返すため
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

///ファイル名から拡張子を取得する
///@param path 対象のパス文字列
///@return 拡張子
std::string GetExtension(const std::string& path) {
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///ファイル名から拡張子を取得する(ワイド文字版)
///@param path 対象のパス文字列
///@return 拡張子
std::wstring GetExtension(const std::wstring& path) {
	int idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///テクスチャのパスをセパレータ文字で分離する
///@param path 対象のパス文字列
///@param splitter 区切り文字
///@return 分離前後の文字列ペア
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*') {
	int idx = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}

///string(マルチバイト文字列)からwstring(ワイド文字列)を得る
///@param str マルチバイト文字列
///@return 変換されたワイド文字列
std::wstring GetWideStringFromString(const std::string& str) {
	//呼び出し1回目(文字列数を得る)
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0);

	std::wstring wstr;//stringのwchar_t版
	wstr.resize(num1);//得られた文字列数でリサイズ

	//呼び出し2回目(確保済みのwstrに変換文字列をコピー)
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);//一応チェック
	return wstr;
}

//トゥーンのためのグラデーションテクスチャ
ID3D12Resource* CreateGrayGradationTexture() {
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 256;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	ID3D12Resource* gradBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&gradBuff)
	);
	if (FAILED(hr)) {
		return nullptr;
	}

	//上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (0xff << 24) | RGB(c, c, c);//RGBAが逆並びしているためRGBマクロと0xff<<24を用いて表す。
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//これでもOK
		std::fill(it, it + 4, col);
		--c;
	}

	hr = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int)*data.size());
	return gradBuff;
}
ID3D12Resource* CreateWhiteTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* whiteBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff)
	);
	if (FAILED(hr)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	hr = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	return whiteBuff;
}
ID3D12Resource* CreateBlackTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* blackBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(hr)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);

	hr = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	return blackBuff;
}

ID3D12Resource* LoadTextureFromFile(std::string& texPath)
{
	if (_resTable.find(texPath) != _resTable.end())
		return _resTable[texPath];

	//WICテクスチャのロード
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wtexpath = GetWideStringFromString(texPath);//テクスチャのファイルパス
	auto ext = GetExtension(texPath);//拡張子を取得
	auto hr = loadLambdaTable[ext](wtexpath, &metadata, scratchImg);
	if (FAILED(hr)) {
		return nullptr;
	}
	auto img = scratchImg.GetImage(0, 0, 0);//生データ抽出

	//WriteToSubresourceで転送する用のヒープ設定
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	heapProp.CreationNodeMask = 0;//単一アダプタのため0
	heapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = metadata.format;
	resDesc.Width = metadata.width;//幅
	resDesc.Height = metadata.height;//高さ
	resDesc.DepthOrArraySize = metadata.arraySize;
	resDesc.SampleDesc.Count = 1;//通常テクスチャなのでアンチェリしない
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = metadata.mipLevels;//ミップマップしないのでミップ数は１つ
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* texBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(&texBuff));

	if (FAILED(hr)) {
		return nullptr;
	}
	hr = texBuff->WriteToSubresource(0,
		nullptr,//全領域へコピー
		img->pixels,//元データアドレス
		img->rowPitch,//1ラインサイズ
		img->slicePitch//全サイズ
	);
	if (FAILED(hr)) {
		return nullptr;
	}

	_resTable[texPath] = texBuff;
	return _resTable[texPath];
}


///////////////////////////////////////////////////////////////////////////////


void DebugOutputFormatString(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}
void EnableDebugLayer()
{
	ID3D12Debug* debugLayer = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer))))
	{
		debugLayer->EnableDebugLayer();	//　デバッグレイヤーを有効化
		debugLayer->Release();			//　有効化したらインターフェイスを開放
	}
}

LRESULT APIENTRY WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(WM_QUIT);
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(WM_QUIT);
			return 0;
		}
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


HRESULT InitializeDXGIDevice()
{
#ifdef _DEBUG
	auto hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&D3D12.Factory));
#else
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(&D3D12.Factory));
#endif	//	_DEBUG
	if (FAILED(hr)) return hr;

	//　アダプタ列挙用
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; D3D12.Factory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC aDesc = {};
		adpt->GetDesc(&aDesc);
		std::wstring strDesc = aDesc.Description;
		//　探したいアダプタの名前確認
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}
	
	//　Direct3Dデバイスの初期化
	//　フィーチャレベル列挙
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL featLv;
	hr = S_FALSE;
	for (auto lv : levels) {
		if (D3D12CreateDevice(tmpAdapter, lv, IID_PPV_ARGS(&D3D12.Device)) == S_OK) {
			featLv = lv;
			hr = S_OK;
			break;
		}
	}
	return hr;
}
HRESULT InitializeCommand()
{
	auto hr = D3D12.Device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D3D12.cmdAllocator));
	if (FAILED(hr)) {
		return hr;
	}

	hr = D3D12.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12.cmdAllocator, nullptr, IID_PPV_ARGS(&D3D12.cmdList));
	if (FAILED(hr)) {
		return hr;
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//　タイムアウト無し
	cmdQueueDesc.NodeMask = 0;							//　アダプタ 単体使用なら 0
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	//　コマンドリストと合わせる
	hr = D3D12.Device->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&D3D12.cmdQueue));
	return hr;
}
HRESULT CreateSwapChain(IDXGIFactory6* &_factory, const HWND &_hWnd, int _w, int _h)
{
	DXGI_SWAP_CHAIN_DESC1 scDesc1 = {};
	scDesc1.Width = _w;
	scDesc1.Height = _h;
	scDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc1.Stereo = false;
	scDesc1.SampleDesc.Count = 1;
	scDesc1.SampleDesc.Quality = 0;
	scDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc1.BufferCount = 2;
	scDesc1.Scaling = DXGI_SCALING_STRETCH;						//　バックバッファは伸び縮み可能
	scDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			//　フリップ後は速やかに破棄
	scDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;			//　特に指定なし
	scDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		//　ウィンドウ⇔フルスクリーン切り替え可能

	auto hr = _factory->CreateSwapChainForHwnd(
		D3D12.cmdQueue, _hWnd, &scDesc1,
		nullptr, nullptr,
		(IDXGISwapChain1**)&D3D12.SwapChain);
	return hr;
}
HRESULT CreateFinalRenderTarget(ID3D12DescriptorHeap* &_rtvHeaps, std::vector<ID3D12Resource*>& _backBuffs)
{
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;						//　裏表の2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//　指定なし

	auto hr = D3D12.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeaps));
	if (FAILED(hr)) return hr;

	DXGI_SWAP_CHAIN_DESC scDesc = {};
	hr = D3D12.SwapChain->GetDesc(&scDesc);
	if (FAILED(hr)) return hr;
	_backBuffs.resize(scDesc.BufferCount);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	auto handle = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < scDesc.BufferCount; i++) {
		hr = D3D12.SwapChain->GetBuffer(i, IID_PPV_ARGS(&_backBuffs[i]));
		assert(SUCCEEDED(hr));

		rtvDesc.Format = _backBuffs[i]->GetDesc().Format;

		D3D12.Device->CreateRenderTargetView(_backBuffs[i], &rtvDesc, handle);
		handle.ptr +=
			D3D12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}
}
//　深度バッファ作成
HRESULT CreateDepthBufferView(ID3D12DescriptorHeap* &_dsvHeap, ID3D12Resource* &_depBuff, int _w, int _h)
{
	//　深度バッファの仕様
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
	resDesc.Width = _w;//幅と高さはレンダーターゲットと同じ
	resDesc.Height = _h;//上に同じ
	resDesc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
	resDesc.Format = DXGI_FORMAT_D32_FLOAT;//深度値書き込み用フォーマット
	resDesc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//このバッファは深度ステンシルとして使用します
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;

	//デプス用ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//このクリアバリューが重要な意味を持つ
	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.DepthStencil.Depth = 1.0f;//深さ１(最大値)でクリア
	clearVal.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア

	auto hr = D3D12.Device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
		&clearVal,
		IID_PPV_ARGS(&_depBuff));
	if (FAILED(hr)) {
		return hr;
	}

	//深度のためのデスクリプタヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};//深度に使うよという事がわかればいい
	heapDesc.NumDescriptors = 1;//深度ビュー1つのみ
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//デプスステンシルビューとして使う
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = D3D12.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_dsvHeap));

	//深度ビュー作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし
	D3D12.Device->CreateDepthStencilView(_depBuff,
		&dsvDesc, _dsvHeap->GetCPUDescriptorHandleForHeapStart());

	return hr;
}



#ifdef _DEBUG
int main()
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	int nCmdShow = SW_SHOW;
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdParam, int nCmdShow)
{
#endif
	auto hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	/*===================================================================
		ウィンドウ作成、Direct3D初期化など
	===================================================================*/
	if (!APP.Init(hInst, nCmdShow, SCRW, SCRH)) return 0;

#ifdef _DEBUG
	//　デバッグレイヤーをオン
	EnableDebugLayer();
#endif // _DEBUG

	if (FAILED(InitializeDXGIDevice())) {
		assert(0);
		return 0;
	}

	if (FAILED(InitializeCommand())) {
		assert(0);
		return 0;
	}
	if (FAILED(CreateSwapChain(D3D12.Factory, hWnd, SCRW, SCRH))) {
		assert(0);
		return 0;
	}

	std::vector<ID3D12Resource*> _backBuffs;
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	if (FAILED(CreateFinalRenderTarget(rtvHeaps, _backBuffs))) {
		assert(0);
		return 0;
	}

#pragma region 拡張子別 読み取り

	loadLambdaTable["sph"]
		= loadLambdaTable["spa"]
		= loadLambdaTable["bmp"]
		= loadLambdaTable["png"]
		= loadLambdaTable["jpg"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromWICFile(path.c_str(), (WIC_FLAGS)0, meta, img);
	};

	loadLambdaTable["tga"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromTGAFile(path.c_str(), meta, img);
	};

	loadLambdaTable["dds"]
		= [](const std::wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromDDSFile(path.c_str(), (DDS_FLAGS)0, meta, img);
	};

#pragma endregion

	ID3D12Resource* depBuff = nullptr;
	ID3D12DescriptorHeap* dsvHeap = nullptr;
	if (FAILED(CreateDepthBufferView(dsvHeap, depBuff, SCRW, SCRH))) return 1;

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	if (FAILED(D3D12.Device->CreateFence(_fenceVal,
		D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence)))) {
		assert(0);
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////

	auto whiteTex = CreateWhiteTexture();
	auto blackTex = CreateBlackTexture();
	auto gradTex = CreateGrayGradationTexture();

#pragma region PMDファイルを読み込み
	
	//　BMPヘッダー構造体
	struct PMDHeader {
		float version;			//　例：00 00 80 3F == 1.00
		char model_name[20];	//　モデル名
		char comment[256];		//　モデルコメント
	};
	char signature[3];
	PMDHeader pmdheader = {};
	FILE* fp;

	strModelPath = "data/" + strModelPath;
	if (fopen_s(&fp, strModelPath.c_str(), "rb") == EOF) return 0;
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	//　VERTEX
	unsigned int vertNum;		//　頂点数
	fread(&vertNum, sizeof(vertNum), 1, fp);
	constexpr unsigned int pmdvert_size = 38;						//　頂点1つ辺りのサイズ
	std::vector<unsigned char> vertices(vertNum * pmdvert_size);	//　バッファ確保
	fread(vertices.data(), vertices.size(), 1, fp);					//　一気に読み込み

	//　INDEX
	unsigned int idxNum;		//　インデックス数
	fread(&idxNum, sizeof(idxNum), 1, fp);
	std::vector<unsigned short> indices(idxNum);
	auto idx_size = indices.size() * sizeof(indices[0]);
	fread(indices.data(), idx_size, 1, fp);

	//　MATERIAL
	unsigned int matNum;		//　マテリアル数
	fread(&matNum, sizeof(matNum), 1, fp);
	std::vector<PMDMaterial> pmdMats(matNum);
	auto pmdMat_size = pmdMats.size() * sizeof(PMDMaterial);
	fread(pmdMats.data(), pmdMat_size, 1, fp);

	fclose(fp);

#pragma endregion

#pragma region バーテックス

	//UPLOAD(確保は可能)
	ID3D12Resource* vertBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertices.size()),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&vertBuff));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	
	unsigned char* vertMap = nullptr;
	hr = vertBuff->Map(0, nullptr, (void**)&vertMap);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	std::copy(vertices.begin(), vertices.end(), vertMap);
	vertBuff->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = {};
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();		//　バッファの仮想アドレス
	vbView.SizeInBytes = vertices.size();							//　総バイト数
	vbView.StrideInBytes = pmdvert_size;							//　1頂点当たりのバイト数

#pragma endregion

#pragma region インデックス

	ID3D12Resource* idxBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(idx_size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&idxBuff));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	//作ったバッファにインデックスデータをコピー
	unsigned short* idxMap = nullptr;
	hr = idxBuff->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	std::copy(indices.begin(), indices.end(), idxMap);
	idxBuff->Unmap(0, nullptr);

	//インデックスバッファビューを作成
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = (UINT)idx_size;

#pragma endregion

#pragma region マテリアル

	std::vector<Material> materials(pmdMats.size());
	//　コピー
	for (int i = 0; i < pmdMats.size(); i++)
	{
		materials[i].indicesNum = pmdMats[i].indicesNum;
		materials[i].material.diffuse = pmdMats[i].diffuse;
		materials[i].material.alpha = pmdMats[i].alpha;
		materials[i].material.specular = pmdMats[i].specular;
		materials[i].material.specularity = pmdMats[i].specularity;
		materials[i].material.ambient = pmdMats[i].ambient;
	}

	std::vector<ID3D12Resource*> texReses(pmdMats.size());
	std::vector<ID3D12Resource*> sphReses(pmdMats.size());
	std::vector<ID3D12Resource*> spaReses(pmdMats.size());
	std::vector<ID3D12Resource*> toonReses(pmdMats.size());
	//　テクスチャの読み込み
	for (int i = 0; i < pmdMats.size(); i++) {
		//トゥーンリソースの読み込み
		std::string toonFilePath = "data/toon/";
		char toonFileName[16];
		sprintf_s(toonFileName, "toon%02d.bmp", pmdMats[i].toonIdx + 1);
		toonFilePath += toonFileName;
		toonReses[i] = LoadTextureFromFile(toonFilePath);

		if (strlen(pmdMats[i].texFilePath) == 0) {
			texReses[i] = nullptr;
			continue;
		}

		std::string texFileName = pmdMats[i].texFilePath;
		std::string sphFileName = "";
		std::string spaFileName = "";
		if (count(texFileName.begin(), texFileName.end(), '*') > 0)	//スプリッタがある
		{
			auto namepair = SplitFileName(texFileName);
			if		(GetExtension(namepair.first) == "sph") {
				texFileName = namepair.second;
				sphFileName = namepair.first;
			}
			else if (GetExtension(namepair.first) == "spa") {
				texFileName = namepair.second;
				spaFileName = namepair.first;
			}
			else {
				texFileName = namepair.first;
				if (GetExtension(namepair.second) == "sph") {
					sphFileName = namepair.second;
				}
				else if (GetExtension(namepair.second) == "spa") {
					spaFileName = namepair.second;
				}
			}
		}
		else {
			if		(GetExtension(pmdMats[i].texFilePath) == "sph") {
				sphFileName = pmdMats[i].texFilePath;
				texFileName = "";
			}
			else if (GetExtension(pmdMats[i].texFilePath) == "spa") {
				spaFileName = pmdMats[i].texFilePath;
				texFileName = "";
			}
			else {
				texFileName = pmdMats[i].texFilePath;
			}
		}
		//モデルとテクスチャパスからアプリケーションからのテクスチャパスを得る
		if (texFileName != "") {
			auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
			texReses[i] = LoadTextureFromFile(texFilePath);
		}
		if (sphFileName != "") {
			auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
			sphReses[i] = LoadTextureFromFile(sphFilePath);
		}
		if (spaFileName != "") {
			auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
			spaReses[i] = LoadTextureFromFile(spaFilePath);
		}
	}

	auto matBuffSize = sizeof(MaterialForHlsl);
	matBuffSize = (matBuffSize + 0xff) &~0xff;

	//マテリアルバッファを作成
	ID3D12Resource* matBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(matBuffSize * matNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&matBuff)
	);

	//マップマテリアルにコピー
	char* mapMaterial = nullptr;
	hr = matBuff->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	for (auto& m : materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;	//　データコピー
		mapMaterial += matBuffSize;						//　次のアライメント位置まで進める
	}
	matBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
	matDescHeapDesc.NumDescriptors = matNum * matsCnt;					//マテリアル数ぶん(定数1つ、テクスチャ3つ)
	matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDescHeapDesc.NodeMask = 0;
	matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;		//デスクリプタヒープ種別

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = matBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = (UINT)matBuffSize;

	//通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ID3D12DescriptorHeap* matDescHeap = nullptr;
	hr = D3D12.Device->CreateDescriptorHeap(&matDescHeapDesc, IID_PPV_ARGS(&matDescHeap));//生成

	auto matDescHeapH = matDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = D3D12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (int i = 0; i < matNum; i++)
	{
		//マテリアル固定バッファビュー
		D3D12.Device->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += matBuffSize;

		if (texReses[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = texReses[i]->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(texReses[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (sphReses[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = sphReses[i]->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(sphReses[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (spaReses[i] == nullptr) {
			srvDesc.Format = blackTex->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(blackTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = spaReses[i]->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(spaReses[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (toonReses[i] == nullptr) {
			srvDesc.Format = gradTex->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = toonReses[i]->GetDesc().Format;
			D3D12.Device->CreateShaderResourceView(toonReses[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
	}

#pragma endregion

	ID3D10Blob* _vsBlob = nullptr;
	ID3D10Blob* _psBlob = nullptr;
	ID3D10Blob* errBlob = nullptr;
	hr = D3DCompileFromFile(GetWideStringFromString(vs_Path).c_str(),
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_vsBlob, &errBlob);
	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			OutputDebugStringA("ファイルが見つかりません");
		else if(hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
			OutputDebugStringA("パスが見つかりません");
		else {
			std::string errstr;
			errstr.resize(errBlob->GetBufferSize());
			std::copy_n((char*)errBlob->GetBufferPointer(), errBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			OutputDebugStringA(errstr.c_str());
		}
		assert(0);
		return 1;
	}

	hr = D3DCompileFromFile(GetWideStringFromString(ps_Path).c_str(),
		nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &_psBlob, &errBlob);
	if (FAILED(hr))
	{
		if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			OutputDebugStringA("ファイルが見つかりません");
		else if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
			OutputDebugStringA("パスが見つかりません");
		else {
			std::string errstr;
			errstr.resize(errBlob->GetBufferSize());
			std::copy_n((char*)errBlob->GetBufferPointer(), errBlob->GetBufferSize(), errstr.begin());
			errstr += "\n";

			OutputDebugStringA(errstr.c_str());
		}
		assert(0);
		return 1;
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
		{
			"EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
		},
	};

	const UINT tbl_Cnt = 3;
	D3D12_DESCRIPTOR_RANGE tblDescRange[tbl_Cnt] = {};
	//　座標変換
	tblDescRange[0].NumDescriptors = 1;									//テクスチャ1つ
	tblDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;		//種別は定数
	tblDescRange[0].BaseShaderRegister = 0;								//0番スロットから
	tblDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//　マテリアル
	tblDescRange[1].NumDescriptors = 1;									//テクスチャ1つ
	tblDescRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;		//種別は定数
	tblDescRange[1].BaseShaderRegister = 1;								//1番スロットから
	tblDescRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	tblDescRange[2].NumDescriptors = 4;									//テクスチャ4つ
	tblDescRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;		//種別はテクスチャ
	tblDescRange[2].BaseShaderRegister = 0;								//0番スロットから
	tblDescRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	const UINT prm_Cnt = 2;
	D3D12_ROOT_PARAMETER rParams[prm_Cnt] = {};
	rParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rParams[0].DescriptorTable.pDescriptorRanges = &tblDescRange[0];
	rParams[0].DescriptorTable.NumDescriptorRanges = 1;
	rParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	
	rParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rParams[1].DescriptorTable.pDescriptorRanges = &tblDescRange[1];
	rParams[1].DescriptorTable.NumDescriptorRanges = 2;
	rParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	const UINT smpr_Cnt = 2;
	D3D12_STATIC_SAMPLER_DESC smprDescs[smpr_Cnt] = {};
	smprDescs[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smprDescs[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smprDescs[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	smprDescs[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	smprDescs[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	smprDescs[0].MaxLOD = D3D12_FLOAT32_MAX;
	smprDescs[0].MinLOD = 0.0f;
	smprDescs[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	smprDescs[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	smprDescs[0].ShaderRegister = 0;

	smprDescs[1] = smprDescs[0];
	smprDescs[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smprDescs[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smprDescs[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	smprDescs[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rSigDesc = {};
	rSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rSigDesc.pParameters		= rParams;		//　ルートパラメータの先頭アドレス
	rSigDesc.NumParameters		= prm_Cnt;			//　ルートパラメータ数
	rSigDesc.pStaticSamplers	= smprDescs;		//　サンプラの先頭アドレス
	rSigDesc.NumStaticSamplers	= smpr_Cnt;			//　サンプラ数

	ID3DBlob* _rSigBlob = nullptr;
	hr = D3D12SerializeRootSignature(&rSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0, &_rSigBlob, &errBlob);
	if (FAILED(hr)) {
		assert(0);
		return hr;
	}

	ID3D12RootSignature* _pRootSig = nullptr;
	hr = D3D12.Device->CreateRootSignature(0, _rSigBlob->GetBufferPointer(),
		_rSigBlob->GetBufferSize(), IID_PPV_ARGS(&_pRootSig));
	if (FAILED(hr)) {
		assert(0);
		return hr;
	}
	SafeRelease(_rSigBlob);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	gpsDesc.VS.pShaderBytecode	= _vsBlob->GetBufferPointer();
	gpsDesc.VS.BytecodeLength	= _vsBlob->GetBufferSize();
	gpsDesc.PS.pShaderBytecode	= _psBlob->GetBufferPointer();
	gpsDesc.PS.BytecodeLength	= _psBlob->GetBufferSize();

	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;				//中身は0xffffffff

	//　OutputManager部分
	//　レンダーターゲット
	gpsDesc.NumRenderTargets	= 1;//今は１つのみ
	gpsDesc.RTVFormats[0]		= DXGI_FORMAT_R8G8B8A8_UNORM;//0～1に正規化されたRGBA

	//　ブレンド設定
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.BlendState.RenderTarget->BlendEnable	= true;
	gpsDesc.BlendState.RenderTarget->SrcBlend		= D3D12_BLEND_SRC_ALPHA;
	gpsDesc.BlendState.RenderTarget->DestBlend		= D3D12_BLEND_INV_SRC_ALPHA;
	gpsDesc.BlendState.RenderTarget->BlendOp		= D3D12_BLEND_OP_ADD;

	//　ラスタライザ(RS)
	gpsDesc.RasterizerState.MultisampleEnable = false;										//　まだアンチェリは使わない
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;							//　カリングしない
	gpsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;						//　中身を塗りつぶす
	gpsDesc.RasterizerState.DepthClipEnable = true;											//　深度方向のクリッピングは有効に
	gpsDesc.RasterizerState.FrontCounterClockwise = false;
	gpsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpsDesc.RasterizerState.AntialiasedLineEnable = false;
	gpsDesc.RasterizerState.ForcedSampleCount = 0;
	gpsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//　深度ステンシル
	gpsDesc.DepthStencilState.DepthEnable = true;		//　深度バッファの使用
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsDesc.DepthStencilState.StencilEnable = false;

	gpsDesc.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
	gpsDesc.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列数

	gpsDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;		//　ストリップ時のカットなし
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		//　三角形で構成

	gpsDesc.SampleDesc.Count = 1;			//　サンプリングは1ピクセルにつき１
	gpsDesc.SampleDesc.Quality = 0;			//　クオリティは最低

	gpsDesc.pRootSignature = _pRootSig;

	ID3D12PipelineState* _pipeState = nullptr;
	hr = D3D12.Device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_pipeState));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	D3D12_VIEWPORT viewport = {};
	viewport.Width = SCRW;					//　出力先の幅(ピクセル数)
	viewport.Height = SCRH;					//　出力先の高さ(ピクセル数)
	viewport.TopLeftX = 0;					//　出力先の左上座標X
	viewport.TopLeftY = 0;					//　出力先の左上座標Y
	viewport.MaxDepth = 1.0f;				//　深度最大値
	viewport.MinDepth = 0.0f;				//　深度最小値

	D3D12_RECT rcScissor = {};
	rcScissor.top = 0;						//　切り抜き上座標
	rcScissor.left = 0;						//　切り抜き左座標
	rcScissor.right = rcScissor.left + SCRW;	//　切り抜き右座標
	rcScissor.bottom = rcScissor.top + SCRH;	//　切り抜き下座標

	/*##########################################################
		XMMATRIX の構造体宣言
	##########################################################*/
	auto worldMat = XMMatrixIdentity();
	XMVECTOR3 camLook(0, 15, -15);
	XMVECTOR3 camPos(0, 15, 0);
	XMVECTOR3 camHead(0, 1, 0);

	auto viewMat = XMMatrixLookAtLH(
		XMLoadFloat3(&camLook),
		XMLoadFloat3(&camPos),
		XMLoadFloat3(&camHead)
	);
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4,
		static_cast<float>(SCRW) / static_cast<float>(SCRH),
		1.0f,
		100.0f
	);
	ID3D12Resource* constBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff)&~0xff),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&constBuff));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	SceneData* mapScene = nullptr;
	hr = constBuff->Map(0, nullptr, (void**)&mapScene);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	//　行列内容をコピー
	mapScene->world = worldMat;
	mapScene->view = viewMat;
	mapScene->proj = projMat;
	mapScene->look = camLook;
	constBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ID3D12DescriptorHeap* basicDescHeap = nullptr;
	hr = D3D12.Device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&basicDescHeap));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	auto basicHeapHandle = basicDescHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = constBuff->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (UINT)constBuff->GetDesc().Width;

	//　定数バッファビューの作成
	D3D12.Device->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	DebugOutputFormatString("Initialize Succesed\n");

	//　ウィンドウ表示
	ShowWindow(hWnd, nCmdShow);

	MSG msg = {};
	unsigned int frame = 0;
	float angle = 0.0f;
	auto dsvH = dsvHeap->GetCPUDescriptorHandleForHeapStart();
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			worldMat = XMMatrixRotationY(angle);
			mapScene->world = worldMat;
			mapScene->view = viewMat;
			mapScene->proj = projMat;
			angle += 0.01f;


			//DirectX処理
			//バックバッファのインデックスを取得
			auto bbIdx = D3D12.SwapChain->GetCurrentBackBufferIndex();

			D3D12.cmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(_backBuffs[bbIdx],
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

			D3D12.cmdList->SetPipelineState(_pipeState);

			//レンダーターゲットを指定
			auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
			rtvH.ptr += bbIdx * D3D12.Device->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			D3D12.cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
			D3D12.cmdList->ClearDepthStencilView(dsvH,
				D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			//画面クリア
			float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
			D3D12.cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

			D3D12.cmdList->RSSetViewports(1, &viewport);
			D3D12.cmdList->RSSetScissorRects(1, &rcScissor);

			D3D12.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			D3D12.cmdList->IASetVertexBuffers(0, 1, &vbView);
			D3D12.cmdList->IASetIndexBuffer(&ibView);

			D3D12.cmdList->SetGraphicsRootSignature(_pRootSig);

			//　WVP変換行列
			D3D12.cmdList->SetDescriptorHeaps(1, &basicDescHeap);
			D3D12.cmdList->SetGraphicsRootDescriptorTable(0, 
				basicDescHeap->GetGPUDescriptorHandleForHeapStart());

			//　マテリアル
			D3D12.cmdList->SetDescriptorHeaps(1, &matDescHeap);

			auto materialH = matDescHeap->GetGPUDescriptorHandleForHeapStart();
			unsigned int idxOffset = 0;

			auto cbvsrvIncSize
				= D3D12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
				* matsCnt;
			for (auto& m : materials) {
				D3D12.cmdList->SetGraphicsRootDescriptorTable(1, materialH);
				D3D12.cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
				materialH.ptr += cbvsrvIncSize;
				idxOffset += m.indicesNum;
			}

			D3D12.cmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(_backBuffs[bbIdx],
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			//命令のクローズ
			D3D12.cmdList->Close();



			//コマンドリストの実行
			ID3D12CommandList* cmdlists[] = { D3D12.cmdList };
			D3D12.cmdQueue->ExecuteCommandLists(1, cmdlists);
			////待ち
			_fenceVal++;
			D3D12.cmdQueue->Signal(_fence, _fenceVal);

			if (_fence->GetCompletedValue() != _fenceVal) {
				auto eve = CreateEvent(nullptr, false, false, nullptr);
				_fence->SetEventOnCompletion(_fenceVal, eve);
				WaitForSingleObjectEx(eve, INFINITE, false);
				CloseHandle(eve);
			}

			//フリップ
			D3D12.SwapChain->Present(0, 0);
			D3D12.cmdAllocator->Reset();
			D3D12.cmdList->Reset(D3D12.cmdAllocator, _pipeState);
		}
	}
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
 }

 bool System::Init(HINSTANCE _hInit,int _cmdShow, int _w, int _h) {

	 APP.m_hInst = _hInit;

	 /*===================================================================
		 フルスクリーン確認
	 ===================================================================*/
	 bool isFullScreen = false;
	 if (MessageBox(nullptr, "フルスクリーンにしますか？", "確認",
		 MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		 isFullScreen = true;

	 /*===================================================================
		 ウィンドウ作成
	 ===================================================================*/
	 if(!m_Window.Create(_hInit, _cmdShow, _w, _h, "window"))
 }

 void System::Loop()
 {
 }

 void System::Release()
 {
 }
