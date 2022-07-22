#include"System\Library\define.h"
#include"main.h"

#pragma region �\���̐錾

struct SceneData
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
	XMFLOAT3 look;
};

#pragma region �p�b�L���O
#pragma pack(1)		//�@1�o�C�g�p�b�L���O
struct PMDMaterial
{
	XMVECTOR3			diffuse;		//�@�f�B�t���[�Y�F
	float				alpha;			//�@�f�B�t���[�Y��
	float				specularity;	//�@�X�y�L�����̋���(��Z�l)
	XMVECTOR3			specular;		//�@�X�y�L�����F
	XMVECTOR3			ambient;		//�@�A���r�G���g�F
	unsigned char		toonIdx;		//�@�g�D�[���ԍ�(��q)
	unsigned char		edgeFlg;		//�@�}�e���A�����̗֊s���t���O

	//�@2�o�C�g�̃p�f�B���O�������I�I

	unsigned int		indicesNum;		//�@���̃}�e���A�������蓖����C���f�b�N�X��
	char				texFilePath[20];//�@�e�N�X�`���t�@�C����(�v���X�A���t�@�c��q)
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

#pragma region �O���[�o���ϐ�

using LoadLambda_t = std::function<HRESULT(const std::wstring& path, TexMetadata*, ScratchImage&)>;
std::map <std::string, LoadLambda_t> loadLambdaTable;

std::map<std::string, ID3D12Resource*> _resTable;

std::string strModelPath("model/�����~�N.pmd");

std::string vs_Path = "data/Shader/BasicVertexShader.hlsl";
std::string ps_Path = "data/Shader/BasicPixelShader.hlsl";

const int matsCnt = 5;

#pragma endregion



///���f���̃p�X�ƃe�N�X�`���̃p�X���獇���p�X�𓾂�
///@param modelPath �A�v���P�[�V�������猩��pmd���f���̃p�X
///@param texPath PMD���f�����猩���e�N�X�`���̃p�X
///@return �A�v���P�[�V�������猩���e�N�X�`���̃p�X
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
	//�t�@�C���̃t�H���_��؂��\��/�̓��ނ��g�p�����\��������
	//�Ƃ�����������\��/�𓾂���΂����̂ŁA�o����rfind���Ƃ��r����
	//int�^�ɑ�����Ă���̂͌�����Ȃ������ꍇ��rfind��epos(-1��0xffffffff)��Ԃ�����
	int pathIndex1 = modelPath.rfind('/');
	int pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}

///�t�@�C��������g���q���擾����
///@param path �Ώۂ̃p�X������
///@return �g���q
std::string GetExtension(const std::string& path) {
	int idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///�t�@�C��������g���q���擾����(���C�h������)
///@param path �Ώۂ̃p�X������
///@return �g���q
std::wstring GetExtension(const std::wstring& path) {
	int idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///�e�N�X�`���̃p�X���Z�p���[�^�����ŕ�������
///@param path �Ώۂ̃p�X������
///@param splitter ��؂蕶��
///@return �����O��̕�����y�A
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*') {
	int idx = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}

///string(�}���`�o�C�g������)����wstring(���C�h������)�𓾂�
///@param str �}���`�o�C�g������
///@return �ϊ����ꂽ���C�h������
std::wstring GetWideStringFromString(const std::string& str) {
	//�Ăяo��1���(�����񐔂𓾂�)
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0);

	std::wstring wstr;//string��wchar_t��
	wstr.resize(num1);//����ꂽ�����񐔂Ń��T�C�Y

	//�Ăяo��2���(�m�ۍς݂�wstr�ɕϊ���������R�s�[)
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);//�ꉞ�`�F�b�N
	return wstr;
}

//�g�D�[���̂��߂̃O���f�[�V�����e�N�X�`��
ID3D12Resource* CreateGrayGradationTexture() {
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 256;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	ID3D12Resource* gradBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&gradBuff)
	);
	if (FAILED(hr)) {
		return nullptr;
	}

	//�オ�����ĉ��������e�N�X�`���f�[�^���쐬
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (0xff << 24) | RGB(c, c, c);//RGBA���t���т��Ă��邽��RGB�}�N����0xff<<24��p���ĕ\���B
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//����ł�OK
		std::fill(it, it + 4, col);
		--c;
	}

	hr = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int)*data.size());
	return gradBuff;
}
ID3D12Resource* CreateWhiteTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 4;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	ID3D12Resource* whiteBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
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
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 4;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	ID3D12Resource* blackBuff = nullptr;
	auto hr = D3D12.Device->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
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

	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wtexpath = GetWideStringFromString(texPath);//�e�N�X�`���̃t�@�C���p�X
	auto ext = GetExtension(texPath);//�g���q���擾
	auto hr = loadLambdaTable[ext](wtexpath, &metadata, scratchImg);
	if (FAILED(hr)) {
		return nullptr;
	}
	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	//WriteToSubresource�œ]������p�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	heapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	heapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = metadata.format;
	resDesc.Width = metadata.width;//��
	resDesc.Height = metadata.height;//����
	resDesc.DepthOrArraySize = metadata.arraySize;
	resDesc.SampleDesc.Count = 1;//�ʏ�e�N�X�`���Ȃ̂ŃA���`�F�����Ȃ�
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = metadata.mipLevels;//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	ID3D12Resource* texBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(&texBuff));

	if (FAILED(hr)) {
		return nullptr;
	}
	hr = texBuff->WriteToSubresource(0,
		nullptr,//�S�̈�փR�s�[
		img->pixels,//���f�[�^�A�h���X
		img->rowPitch,//1���C���T�C�Y
		img->slicePitch//�S�T�C�Y
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
		debugLayer->EnableDebugLayer();	//�@�f�o�b�O���C���[��L����
		debugLayer->Release();			//�@�L����������C���^�[�t�F�C�X���J��
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

	//�@�A�_�v�^�񋓗p
	std::vector <IDXGIAdapter*> adapters;
	IDXGIAdapter* tmpAdapter = nullptr;
	for (int i = 0; D3D12.Factory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC aDesc = {};
		adpt->GetDesc(&aDesc);
		std::wstring strDesc = aDesc.Description;
		//�@�T�������A�_�v�^�̖��O�m�F
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}
	
	//�@Direct3D�f�o�C�X�̏�����
	//�@�t�B�[�`�����x����
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
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;	//�@�^�C���A�E�g����
	cmdQueueDesc.NodeMask = 0;							//�@�A�_�v�^ �P�̎g�p�Ȃ� 0
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	//�@�R�}���h���X�g�ƍ��킹��
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
	scDesc1.Scaling = DXGI_SCALING_STRETCH;						//�@�o�b�N�o�b�t�@�͐L�яk�݉\
	scDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;			//�@�t���b�v��͑��₩�ɔj��
	scDesc1.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;			//�@���Ɏw��Ȃ�
	scDesc1.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;		//�@�E�B���h�E�̃t���X�N���[���؂�ւ��\

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
	heapDesc.NumDescriptors = 2;						//�@���\��2��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//�@�w��Ȃ�

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
//�@�[�x�o�b�t�@�쐬
HRESULT CreateDepthBufferView(ID3D12DescriptorHeap* &_dsvHeap, ID3D12Resource* &_depBuff, int _w, int _h)
{
	//�@�[�x�o�b�t�@�̎d�l
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2�����̃e�N�X�`���f�[�^�Ƃ���
	resDesc.Width = _w;//���ƍ����̓����_�[�^�[�Q�b�g�Ɠ���
	resDesc.Height = _h;//��ɓ���
	resDesc.DepthOrArraySize = 1;//�e�N�X�`���z��ł��Ȃ���3D�e�N�X�`���ł��Ȃ�
	resDesc.Format = DXGI_FORMAT_D32_FLOAT;//�[�x�l�������ݗp�t�H�[�}�b�g
	resDesc.SampleDesc.Count = 1;//�T���v����1�s�N�Z��������1��
	resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//���̃o�b�t�@�͐[�x�X�e���V���Ƃ��Ďg�p���܂�
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.MipLevels = 1;

	//�f�v�X�p�q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULT��������UNKNOWN�ł悵
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	//���̃N���A�o�����[���d�v�ȈӖ�������
	D3D12_CLEAR_VALUE clearVal = {};
	clearVal.DepthStencil.Depth = 1.0f;//�[���P(�ő�l)�ŃN���A
	clearVal.Format = DXGI_FORMAT_D32_FLOAT;//32bit�[�x�l�Ƃ��ăN���A

	auto hr = D3D12.Device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE,
		&resDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
		&clearVal,
		IID_PPV_ARGS(&_depBuff));
	if (FAILED(hr)) {
		return hr;
	}

	//�[�x�̂��߂̃f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};//�[�x�Ɏg����Ƃ��������킩��΂���
	heapDesc.NumDescriptors = 1;//�[�x�r���[1�̂�
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�f�v�X�X�e���V���r���[�Ƃ��Ďg��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = D3D12.Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_dsvHeap));

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�
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
		�E�B���h�E�쐬�ADirect3D�������Ȃ�
	===================================================================*/
	if (!APP.Init(hInst, nCmdShow, SCRW, SCRH)) return 0;

#ifdef _DEBUG
	//�@�f�o�b�O���C���[���I��
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

#pragma region �g���q�� �ǂݎ��

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

#pragma region PMD�t�@�C����ǂݍ���
	
	//�@BMP�w�b�_�[�\����
	struct PMDHeader {
		float version;			//�@��F00 00 80 3F == 1.00
		char model_name[20];	//�@���f����
		char comment[256];		//�@���f���R�����g
	};
	char signature[3];
	PMDHeader pmdheader = {};
	FILE* fp;

	strModelPath = "data/" + strModelPath;
	if (fopen_s(&fp, strModelPath.c_str(), "rb") == EOF) return 0;
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	//�@VERTEX
	unsigned int vertNum;		//�@���_��
	fread(&vertNum, sizeof(vertNum), 1, fp);
	constexpr unsigned int pmdvert_size = 38;						//�@���_1�ӂ�̃T�C�Y
	std::vector<unsigned char> vertices(vertNum * pmdvert_size);	//�@�o�b�t�@�m��
	fread(vertices.data(), vertices.size(), 1, fp);					//�@��C�ɓǂݍ���

	//�@INDEX
	unsigned int idxNum;		//�@�C���f�b�N�X��
	fread(&idxNum, sizeof(idxNum), 1, fp);
	std::vector<unsigned short> indices(idxNum);
	auto idx_size = indices.size() * sizeof(indices[0]);
	fread(indices.data(), idx_size, 1, fp);

	//�@MATERIAL
	unsigned int matNum;		//�@�}�e���A����
	fread(&matNum, sizeof(matNum), 1, fp);
	std::vector<PMDMaterial> pmdMats(matNum);
	auto pmdMat_size = pmdMats.size() * sizeof(PMDMaterial);
	fread(pmdMats.data(), pmdMat_size, 1, fp);

	fclose(fp);

#pragma endregion

#pragma region �o�[�e�b�N�X

	//UPLOAD(�m�ۂ͉\)
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
	vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();		//�@�o�b�t�@�̉��z�A�h���X
	vbView.SizeInBytes = vertices.size();							//�@���o�C�g��
	vbView.StrideInBytes = pmdvert_size;							//�@1���_������̃o�C�g��

#pragma endregion

#pragma region �C���f�b�N�X

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

	//������o�b�t�@�ɃC���f�b�N�X�f�[�^���R�s�[
	unsigned short* idxMap = nullptr;
	hr = idxBuff->Map(0, nullptr, (void**)&idxMap);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	std::copy(indices.begin(), indices.end(), idxMap);
	idxBuff->Unmap(0, nullptr);

	//�C���f�b�N�X�o�b�t�@�r���[���쐬
	D3D12_INDEX_BUFFER_VIEW ibView = {};
	ibView.BufferLocation = idxBuff->GetGPUVirtualAddress();
	ibView.Format = DXGI_FORMAT_R16_UINT;
	ibView.SizeInBytes = (UINT)idx_size;

#pragma endregion

#pragma region �}�e���A��

	std::vector<Material> materials(pmdMats.size());
	//�@�R�s�[
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
	//�@�e�N�X�`���̓ǂݍ���
	for (int i = 0; i < pmdMats.size(); i++) {
		//�g�D�[�����\�[�X�̓ǂݍ���
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
		if (count(texFileName.begin(), texFileName.end(), '*') > 0)	//�X�v���b�^������
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
		//���f���ƃe�N�X�`���p�X����A�v���P�[�V��������̃e�N�X�`���p�X�𓾂�
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

	//�}�e���A���o�b�t�@���쐬
	ID3D12Resource* matBuff = nullptr;
	hr = D3D12.Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(matBuffSize * matNum),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr, IID_PPV_ARGS(&matBuff)
	);

	//�}�b�v�}�e���A���ɃR�s�[
	char* mapMaterial = nullptr;
	hr = matBuff->Map(0, nullptr, (void**)&mapMaterial);
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}
	for (auto& m : materials)
	{
		*((MaterialForHlsl*)mapMaterial) = m.material;	//�@�f�[�^�R�s�[
		mapMaterial += matBuffSize;						//�@���̃A���C�����g�ʒu�܂Ői�߂�
	}
	matBuff->Unmap(0, nullptr);

	D3D12_DESCRIPTOR_HEAP_DESC matDescHeapDesc = {};
	matDescHeapDesc.NumDescriptors = matNum * matsCnt;					//�}�e���A�����Ԃ�(�萔1�A�e�N�X�`��3��)
	matDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	matDescHeapDesc.NodeMask = 0;
	matDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;		//�f�X�N���v�^�q�[�v���

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = matBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = (UINT)matBuffSize;

	//�ʏ�e�N�X�`���r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	ID3D12DescriptorHeap* matDescHeap = nullptr;
	hr = D3D12.Device->CreateDescriptorHeap(&matDescHeapDesc, IID_PPV_ARGS(&matDescHeap));//����

	auto matDescHeapH = matDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = D3D12.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (int i = 0; i < matNum; i++)
	{
		//�}�e���A���Œ�o�b�t�@�r���[
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
			OutputDebugStringA("�t�@�C����������܂���");
		else if(hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
			OutputDebugStringA("�p�X��������܂���");
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
			OutputDebugStringA("�t�@�C����������܂���");
		else if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
			OutputDebugStringA("�p�X��������܂���");
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
	//�@���W�ϊ�
	tblDescRange[0].NumDescriptors = 1;									//�e�N�X�`��1��
	tblDescRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;		//��ʂ͒萔
	tblDescRange[0].BaseShaderRegister = 0;								//0�ԃX���b�g����
	tblDescRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//�@�}�e���A��
	tblDescRange[1].NumDescriptors = 1;									//�e�N�X�`��1��
	tblDescRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;		//��ʂ͒萔
	tblDescRange[1].BaseShaderRegister = 1;								//1�ԃX���b�g����
	tblDescRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	tblDescRange[2].NumDescriptors = 4;									//�e�N�X�`��4��
	tblDescRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;		//��ʂ̓e�N�X�`��
	tblDescRange[2].BaseShaderRegister = 0;								//0�ԃX���b�g����
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
	rSigDesc.pParameters		= rParams;		//�@���[�g�p�����[�^�̐擪�A�h���X
	rSigDesc.NumParameters		= prm_Cnt;			//�@���[�g�p�����[�^��
	rSigDesc.pStaticSamplers	= smprDescs;		//�@�T���v���̐擪�A�h���X
	rSigDesc.NumStaticSamplers	= smpr_Cnt;			//�@�T���v����

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

	gpsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;				//���g��0xffffffff

	//�@OutputManager����
	//�@�����_�[�^�[�Q�b�g
	gpsDesc.NumRenderTargets	= 1;//���͂P�̂�
	gpsDesc.RTVFormats[0]		= DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

	//�@�u�����h�ݒ�
	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.BlendState.RenderTarget->BlendEnable	= true;
	gpsDesc.BlendState.RenderTarget->SrcBlend		= D3D12_BLEND_SRC_ALPHA;
	gpsDesc.BlendState.RenderTarget->DestBlend		= D3D12_BLEND_INV_SRC_ALPHA;
	gpsDesc.BlendState.RenderTarget->BlendOp		= D3D12_BLEND_OP_ADD;

	//�@���X�^���C�U(RS)
	gpsDesc.RasterizerState.MultisampleEnable = false;										//�@�܂��A���`�F���͎g��Ȃ�
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;							//�@�J�����O���Ȃ�
	gpsDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;						//�@���g��h��Ԃ�
	gpsDesc.RasterizerState.DepthClipEnable = true;											//�@�[�x�����̃N���b�s���O�͗L����
	gpsDesc.RasterizerState.FrontCounterClockwise = false;
	gpsDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpsDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpsDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpsDesc.RasterizerState.AntialiasedLineEnable = false;
	gpsDesc.RasterizerState.ForcedSampleCount = 0;
	gpsDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//�@�[�x�X�e���V��
	gpsDesc.DepthStencilState.DepthEnable = true;		//�@�[�x�o�b�t�@�̎g�p
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsDesc.DepthStencilState.StencilEnable = false;

	gpsDesc.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
	gpsDesc.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

	gpsDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;		//�@�X�g���b�v���̃J�b�g�Ȃ�
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		//�@�O�p�`�ō\��

	gpsDesc.SampleDesc.Count = 1;			//�@�T���v�����O��1�s�N�Z���ɂ��P
	gpsDesc.SampleDesc.Quality = 0;			//�@�N�I���e�B�͍Œ�

	gpsDesc.pRootSignature = _pRootSig;

	ID3D12PipelineState* _pipeState = nullptr;
	hr = D3D12.Device->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_pipeState));
	if (FAILED(hr)) {
		assert(0);
		return 0;
	}

	D3D12_VIEWPORT viewport = {};
	viewport.Width = SCRW;					//�@�o�͐�̕�(�s�N�Z����)
	viewport.Height = SCRH;					//�@�o�͐�̍���(�s�N�Z����)
	viewport.TopLeftX = 0;					//�@�o�͐�̍�����WX
	viewport.TopLeftY = 0;					//�@�o�͐�̍�����WY
	viewport.MaxDepth = 1.0f;				//�@�[�x�ő�l
	viewport.MinDepth = 0.0f;				//�@�[�x�ŏ��l

	D3D12_RECT rcScissor = {};
	rcScissor.top = 0;						//�@�؂蔲������W
	rcScissor.left = 0;						//�@�؂蔲�������W
	rcScissor.right = rcScissor.left + SCRW;	//�@�؂蔲���E���W
	rcScissor.bottom = rcScissor.top + SCRH;	//�@�؂蔲�������W

	/*##########################################################
		XMMATRIX �̍\���̐錾
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
	//�@�s����e���R�s�[
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

	//�@�萔�o�b�t�@�r���[�̍쐬
	D3D12.Device->CreateConstantBufferView(&cbvDesc, basicHeapHandle);

	DebugOutputFormatString("Initialize Succesed\n");

	//�@�E�B���h�E�\��
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


			//DirectX����
			//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
			auto bbIdx = D3D12.SwapChain->GetCurrentBackBufferIndex();

			D3D12.cmdList->ResourceBarrier(1,
				&CD3DX12_RESOURCE_BARRIER::Transition(_backBuffs[bbIdx],
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

			D3D12.cmdList->SetPipelineState(_pipeState);

			//�����_�[�^�[�Q�b�g���w��
			auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
			rtvH.ptr += bbIdx * D3D12.Device->GetDescriptorHandleIncrementSize(
				D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

			D3D12.cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
			D3D12.cmdList->ClearDepthStencilView(dsvH,
				D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			//��ʃN���A
			float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };
			D3D12.cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

			D3D12.cmdList->RSSetViewports(1, &viewport);
			D3D12.cmdList->RSSetScissorRects(1, &rcScissor);

			D3D12.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			D3D12.cmdList->IASetVertexBuffers(0, 1, &vbView);
			D3D12.cmdList->IASetIndexBuffer(&ibView);

			D3D12.cmdList->SetGraphicsRootSignature(_pRootSig);

			//�@WVP�ϊ��s��
			D3D12.cmdList->SetDescriptorHeaps(1, &basicDescHeap);
			D3D12.cmdList->SetGraphicsRootDescriptorTable(0, 
				basicDescHeap->GetGPUDescriptorHandleForHeapStart());

			//�@�}�e���A��
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

			//���߂̃N���[�Y
			D3D12.cmdList->Close();



			//�R�}���h���X�g�̎��s
			ID3D12CommandList* cmdlists[] = { D3D12.cmdList };
			D3D12.cmdQueue->ExecuteCommandLists(1, cmdlists);
			////�҂�
			_fenceVal++;
			D3D12.cmdQueue->Signal(_fence, _fenceVal);

			if (_fence->GetCompletedValue() != _fenceVal) {
				auto eve = CreateEvent(nullptr, false, false, nullptr);
				_fence->SetEventOnCompletion(_fenceVal, eve);
				WaitForSingleObjectEx(eve, INFINITE, false);
				CloseHandle(eve);
			}

			//�t���b�v
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
		 �t���X�N���[���m�F
	 ===================================================================*/
	 bool isFullScreen = false;
	 if (MessageBox(nullptr, "�t���X�N���[���ɂ��܂����H", "�m�F",
		 MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
		 isFullScreen = true;

	 /*===================================================================
		 �E�B���h�E�쐬
	 ===================================================================*/
	 if(!m_Window.Create(_hInit, _cmdShow, _w, _h, "window"))
 }

 void System::Loop()
 {
 }

 void System::Release()
 {
 }
