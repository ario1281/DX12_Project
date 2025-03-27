#include "Pipeline.h"

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
