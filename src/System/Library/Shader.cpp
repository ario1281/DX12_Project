#include "Shader.h"

#pragma region Shader

void Shader::Create(const std::string& filename, const RenderSetting& renderSetting, const std::vector<RangeType>& rangeType)
{
	LoadShaderFile(filename);
	
	// RootSignature作成
	m_upRootSignature = std::make_unique<RootSignature>();
	m_upRootSignature->Create(rangeType, m_cbvCount);

	// Pipeline作成
	m_upPipeline = std::make_unique<Pipeline>();
	m_upPipeline->SetRenderSettings(
		m_upRootSignature.get(),
		renderSetting.InputLayouts,
		renderSetting.CullMode,
		renderSetting.BlendMode,
		renderSetting.PrimitiveTopologyType
	);
	m_upPipeline->Create(
		{ m_pVSBlob ,m_pHSBlob ,m_pDSBlob ,m_pGSBlob ,m_pPSBlob },
		renderSetting.Formats,
		renderSetting.IsDepth,
		renderSetting.IsDepthMask,
		renderSetting.RTVCount,
		renderSetting.IsWireFrame
	);
}

void Shader::Begin(int width, int height)
{
	CMD->SetPipelineState(m_upPipeline->GetPipeline());
	CMD->SetGraphicsRootSignature(m_upRootSignature->GetRootSignature());

	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType
		= (D3D12_PRIMITIVE_TOPOLOGY_TYPE)m_upPipeline->GetTopologyType();

	switch (topologyType)
	{
	case D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:
		CMD->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;
	case D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:
		CMD->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		break;
	case D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
		CMD->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH:
		CMD->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		break;
	}

	D3D12_VIEWPORT viewport = {};
	D3D12_RECT rect = {};

	viewport.Width    = (float)width;
	viewport.Height   = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	rect.right  = width;
	rect.bottom = height;

	CMD->RSSetViewports(1, &viewport);
	CMD->RSSetScissorRects(1, &rect);
}

void Shader::DrawMesh(const Mesh& mesh)
{
	SetMaterial(mesh.GetMaterial());
	mesh.DrawInstanced(mesh.GetInstCount());
}

void Shader::DrawModel(const Model& model)
{
	// ノードをすべて描画
	for (auto&& node : model.GetNodes())
	{
		DrawMesh(*node.spMesh);
	}
}

void Shader::LoadShaderFile(const std::string& filePath)
{
	ID3DInclude* include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
	UINT         flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	ID3DBlob* pErrBlob = nullptr;

	std::string format      = ".hlsl";
	std::string currentPath = "Asset/Shader/";

	#pragma region シェーダーコンパイル

	// 頂点シェーダーのコンパイル
	{
		std::string fullPath = currentPath + filePath + "_VS" + format;
		auto hr = D3DCompileFromFile(
			stow(fullPath).c_str(), nullptr,
			include, "main",
			"vs_5_0", flag, 0,
			&m_pVSBlob, &pErrBlob
		);

		if (FAILED(hr)) {
			assert(0 && "頂点シェーダーのコンパイルに失敗しました");
			return;
		}
	}

	// ハルシェーダーのコンパイル
	{
		std::string fullPath = currentPath + filePath + "_HS" + format;
		auto hr = D3DCompileFromFile(
			stow(fullPath).c_str(), nullptr,
			include, "main",
			"hs_5_0", flag, 0,
			&m_pHSBlob, &pErrBlob
		);

		// ハルシェーダーはなくてもいい
	}

	// ドメインシェーダーのコンパイル
	{
		std::string fullPath = currentPath + filePath + "_DS" + format;
		auto hr = D3DCompileFromFile(
			stow(fullPath).c_str(), nullptr,
			include, "main",
			"ds_5_0", flag, 0,
			&m_pDSBlob, &pErrBlob
		);

		// ドメインシェーダーはなくてもいい
	}

	// ジオメトリシェーダーのコンパイル
	{
		std::string fullPath = currentPath + filePath + "_GS" + format;
		auto hr = D3DCompileFromFile(
			stow(fullPath).c_str(), nullptr,
			include, "main",
			"gs_5_0", flag, 0,
			&m_pGSBlob, &pErrBlob
		);

		// ジオメトリシェーダーはなくてもいい
	}

	// ピクセルシェーダーのコンパイル
	{
		std::string fullPath = currentPath + filePath + "_PS" + format;
		auto hr = D3DCompileFromFile(
			stow(fullPath).c_str(), nullptr,
			include, "main",
			"ps_5_0", flag, 0,
			&m_pPSBlob, &pErrBlob
		);

		if (FAILED(hr)) {
			assert(0 && "ピクセルシェーダーのコンパイルに失敗しました");
			return;
		}
	}

	#pragma endregion

}

void Shader::SetMaterial(const Material& material)
{
	material.spBaseColor->Set(m_cbvCount);
	//material.spNormal->Set(m_cbvCount + 1);
	//material.spScaling->Set(m_cbvCount + 2);
	//material.spEmissive->Set(m_cbvCount + 3);
}

#pragma endregion

#pragma region Pipeline

void Pipeline::SetRenderSettings(RootSignature* pRootSignature, const std::vector<InputLayout>& inputLayouts, CullMode cullMode, BlendMode blendMode, PrimitiveTopologyType topologyType)
{
	m_pRootSignature = pRootSignature;
	m_inputLayouts   = inputLayouts;
	m_cullMode       = cullMode;
	m_blendMode      = blendMode;
	m_topologyType   = topologyType;
}

void Pipeline::Create(std::vector<ID3DBlob*> pBlobs, const std::vector<DXGI_FORMAT> formats, bool isDepth, bool isDepthMask, int rtvCount, bool isWireFrame)
{
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDesc;
	SetInputLayout(inputElementDesc, m_inputLayouts);

	// GraphicsPipelineStateの各種設定
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};

	#pragma region シェーダー系

	// 頂点シェーダーをセット
	pipelineStateDesc.VS.pShaderBytecode = pBlobs[0]->GetBufferPointer();
	pipelineStateDesc.VS.BytecodeLength  = pBlobs[0]->GetBufferSize();

	// ハルシェーダーをセット
	if (pBlobs[1]) {
		pipelineStateDesc.HS.pShaderBytecode = pBlobs[1]->GetBufferPointer();
		pipelineStateDesc.HS.BytecodeLength  = pBlobs[1]->GetBufferSize();
	}

	// ドメインシェーダーをセット
	if (pBlobs[2]) {
		pipelineStateDesc.DS.pShaderBytecode = pBlobs[2]->GetBufferPointer();
		pipelineStateDesc.DS.BytecodeLength  = pBlobs[2]->GetBufferSize();
	}

	// ジオメトリシェーダーをセット
	if (pBlobs[3]) {
		pipelineStateDesc.GS.pShaderBytecode = pBlobs[3]->GetBufferPointer();
		pipelineStateDesc.GS.BytecodeLength  = pBlobs[3]->GetBufferSize();
	}

	// ピクセルシェーダーをセット
	pipelineStateDesc.PS.pShaderBytecode = pBlobs[4]->GetBufferPointer();
	pipelineStateDesc.PS.BytecodeLength  = pBlobs[4]->GetBufferSize();

	#pragma endregion

	pipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// カリングモードをセット
	pipelineStateDesc.RasterizerState.CullMode = (D3D12_CULL_MODE)m_cullMode;

	// フィルターモードをセット
	pipelineStateDesc.RasterizerState.FillMode = isWireFrame ?
		D3D12_FILL_MODE_WIREFRAME : // 中身を塗りつぶさない
		D3D12_FILL_MODE_SOLID;      // 中身を塗りつぶす

	// 深度設定をセット
	if (isDepth) {
		pipelineStateDesc.RasterizerState.DepthClipEnable = true;
		pipelineStateDesc.DepthStencilState.DepthEnable   = true;

		pipelineStateDesc.DepthStencilState.DepthWriteMask = isDepthMask ?
			D3D12_DEPTH_WRITE_MASK_ALL :
			D3D12_DEPTH_WRITE_MASK_ZERO;

		pipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		pipelineStateDesc.DSVFormat                   = DXGI_FORMAT_D32_FLOAT;
	}
	else {
		pipelineStateDesc.RasterizerState.DepthClipEnable = false;
		pipelineStateDesc.DepthStencilState.DepthEnable   = false;
	}

	pipelineStateDesc.BlendState.AlphaToCoverageEnable  = false;
	pipelineStateDesc.BlendState.IndependentBlendEnable = false;

	// ブレンド設定
	D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
	SetBlendMode(blendDesc, m_blendMode);

	pipelineStateDesc.BlendState.RenderTarget[0] = blendDesc;

	pipelineStateDesc.InputLayout.pInputElementDescs = inputElementDesc.data();    // レイアウト先頭アドレス
	pipelineStateDesc.InputLayout.NumElements        = (int)m_inputLayouts.size(); // レイアウト配列の要素数

	pipelineStateDesc.PrimitiveTopologyType = (pBlobs[3] && pBlobs[4]) ?
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH :
		(D3D12_PRIMITIVE_TOPOLOGY_TYPE)m_topologyType;

	// RTV数をセット
	pipelineStateDesc.NumRenderTargets = rtvCount;

	// RTVフォーマットのセット
	for (int i = 0; i < rtvCount; ++i) {
		pipelineStateDesc.RTVFormats[i] = formats[i];
	}

	pipelineStateDesc.SampleDesc.Count = 1; // サンプリングは1ピクセルにつき1
	pipelineStateDesc.pRootSignature   = m_pRootSignature->GetRootSignature();

	auto hr = DEV->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_pPipelineState));

	if (FAILED(hr)) {
		assert(0 && "パイプラインステートの作成に失敗しました");
		return;
	}
}

void Pipeline::SetInputLayout(std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElements, const std::vector<InputLayout>& inputLayouts)
{
	for (int i = 0; i < (int)inputLayouts.size(); ++i) {
		if (inputLayouts[i] == InputLayout::POSITION) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"POSITION", 0,
				DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::TEXCOORD) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"TEXCOORD", 0,
				DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT, 
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::NORMAL) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"NORMAL", 0,
				DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::COLOR) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"COLOR", 0,
				DXGI_FORMAT_R8G8B8A8_UNORM, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::TANGENT) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"TANGENT", 0,
				DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::SKININDEX) {
			// 
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"SKININDEX", 0,
				DXGI_FORMAT_R16G16B16A16_UINT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
		else if (inputLayouts[i] == InputLayout::SKINWEIGHT) {
			//
			inputElements.emplace_back(D3D12_INPUT_ELEMENT_DESC{
				"SKINWEIGHT", 0,
				DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D12_APPEND_ALIGNED_ELEMENT,
				D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
			});
		}
	}
}

void Pipeline::SetBlendMode(D3D12_RENDER_TARGET_BLEND_DESC& blendDesc, BlendMode blendMode)
{
	blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.BlendEnable           = true;

	switch (blendMode)
	{
	case BlendMode::Add:
		// 加算合成
		blendDesc.BlendOp   = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlend  = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_ONE;

		blendDesc.BlendOpAlpha   = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha  = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.LogicOp        = D3D12_LOGIC_OP_NOOP;
		break;
	case BlendMode::Alpha:
		// 半透明
		blendDesc.BlendOp   = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlend  = D3D12_BLEND_SRC_ALPHA;
		blendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;

		blendDesc.BlendOpAlpha   = D3D12_BLEND_OP_ADD;
		blendDesc.SrcBlendAlpha  = D3D12_BLEND_ONE;
		blendDesc.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.LogicOp        = D3D12_LOGIC_OP_NOOP;
		break;
	}
}

#pragma endregion

#pragma region RootSignature

void RootSignature::Create(const std::vector<RangeType>& rangeTypes, UINT& cbvCount)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignDesc = {};
	int rangeCount = (int)rangeTypes.size();

	// レンジ数分だけルートパラメータ、レンジを生成
	std::vector<D3D12_ROOT_PARAMETER>   rootParams(rangeCount);
	std::vector<D3D12_DESCRIPTOR_RANGE> ranges(rangeCount);

	// SRVの数だけSamplerDescを生成
	int sampCount = 0;

	for (int i = 0; i < (int)rangeTypes.size(); ++i)
	{
		if (rangeTypes[i] == RangeType::SRV) { ++sampCount; }
	}

	// 指定された順に設定
	sampCount = 0;

	bool bSampler = false;
	int uavCount = 0;

	for (int i = 0; i < rangeCount; ++i)
	{
		switch (rangeTypes[i])
		{
		case RangeType::CBV:
			ranges[i] = CreateRange(RangeType::CBV, cbvCount);
			++cbvCount;
			break;
		case RangeType::SRV:
			ranges[i] = CreateRange(RangeType::SRV, sampCount);
			++sampCount;
			bSampler = true;
			break;
		case RangeType::UAV:
			ranges[i] = CreateRange(RangeType::UAV, uavCount);
			++uavCount;
			break;
		default:
			break;
		}
		rootParams[i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[i].DescriptorTable.pDescriptorRanges = &ranges[i];
		rootParams[i].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}

	std::array<D3D12_STATIC_SAMPLER_DESC, 4> pStaticSamplerDescs = {};

	if (bSampler)
	{
		pStaticSamplerDescs[0] = CreateStaticSampler(TextureAddressMode::Wrap, D3D12Filter::Point, 0);
		pStaticSamplerDescs[1] = CreateStaticSampler(TextureAddressMode::Clamp, D3D12Filter::Point, 1);
		pStaticSamplerDescs[2] = CreateStaticSampler(TextureAddressMode::Wrap, D3D12Filter::Linear, 2);
		pStaticSamplerDescs[3] = CreateStaticSampler(TextureAddressMode::Clamp, D3D12Filter::Linear, 3);
	}

	rootSignDesc.pStaticSamplers   = bSampler ? pStaticSamplerDescs.data() : nullptr;
	rootSignDesc.NumStaticSamplers = bSampler ? 4 : 0;
	rootSignDesc.Flags             = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignDesc.pParameters       = rootParams.data();
	rootSignDesc.NumParameters     = (int)rangeTypes.size();

	ID3DBlob* pErrBlob = nullptr;
	auto hr = D3D12SerializeRootSignature(
		&rootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&m_pRootBlob, &pErrBlob
	);
	if (FAILED(hr)) {
		assert(0 && "ルートシグネチャ初期化失敗");
	}

	hr = DEV->CreateRootSignature(
		0,
		m_pRootBlob->GetBufferPointer(),
		m_pRootBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_pRootSign)
	);
	if (FAILED(hr)) {
		assert(0 && "ルートシグネチャ作成失敗");
	}
}

const D3D12_DESCRIPTOR_RANGE&
RootSignature::CreateRange(RangeType type, int count)
{
	D3D12_DESCRIPTOR_RANGE descRange = {};
	descRange.NumDescriptors                    = 1;
	descRange.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descRange.BaseShaderRegister                = count;
	descRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	return descRange;
}

const D3D12_STATIC_SAMPLER_DESC&
RootSignature::CreateStaticSampler(TextureAddressMode mode, D3D12Filter filter, int count)
{
	D3D12_TEXTURE_ADDRESS_MODE _mode = {};
	_mode = mode == TextureAddressMode::Wrap ?
		D3D12_TEXTURE_ADDRESS_MODE_WRAP :
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	D3D12_FILTER _filter = {};
	_filter = filter == D3D12Filter::Point ?
		D3D12_FILTER_MIN_MAG_MIP_POINT :
		D3D12_FILTER_MIN_MAG_MIP_LINEAR;

	D3D12_STATIC_SAMPLER_DESC sampDesc = {};
	sampDesc.AddressU         = _mode;
	sampDesc.AddressV         = _mode;
	sampDesc.AddressW         = _mode;
	sampDesc.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampDesc.Filter           = _filter;
	sampDesc.MaxLOD           = D3D12_FLOAT32_MAX;
	sampDesc.MinLOD           = 0.0f;
	sampDesc.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
	sampDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	sampDesc.MaxAnisotropy    = 16;
	sampDesc.ShaderRegister   = count;

	return sampDesc;
}

#pragma endregion
