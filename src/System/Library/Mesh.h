#pragma once

class Texture;

struct MeshVertex {
	Vector3 pos;
	Vector2 uv;
	Vector3 normal;
	UINT    color = 0xffffff;
	Vector3 tan;
};

struct MeshFace {
	UINT index[3];
};

struct Material
{
	std::string                name;

	shared_ptr<Texture> spBaseColor;
	Vector4                    baseColor = { 1,1,1,1 };

	shared_ptr<Texture> spScaling;
	float                      metalic = 0.0f;
	float                      rough   = 1.0f;

	shared_ptr<Texture> spEmissive;
	Vector3                    emissive = { 0,0,0 };

	shared_ptr<Texture> spNormal;
};


class Mesh
{
public:
	void Create(
		const std::vector<MeshVertex>& verteces,
		const std::vector<MeshFace>&   faces,
		const Material                 material
	);

	void DrawInstanced(UINT verCount) const;

	#pragma region Žæ“¾Œn

	UINT GetInstCount() const { return m_instCount; }

	const Material& GetMaterial() const { return m_material; }

	#pragma endregion

private:
	com_ptr<ID3D12Resource>  m_pVBuffer = nullptr;
	com_ptr<ID3D12Resource>  m_pIBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_vbView;
	D3D12_INDEX_BUFFER_VIEW  m_ibView;

	UINT     m_instCount;
	Material m_material;
};