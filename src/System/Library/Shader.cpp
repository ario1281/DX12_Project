#include "Shader.h"

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
