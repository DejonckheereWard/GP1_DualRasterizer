#pragma once
#include "MathHelpers.h"
#include <vector>
#include "EffectVehicle.h"

using namespace dae;

class Texture;

struct Vertex
{
	Vector3 position{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector2 uv{};
};

struct Vertex_Out
{
	Vector4 position{};
	Vector4 worldPosition{};
	Vector3 normal{};
	Vector3 tangent{};
	Vector2 uv{};
};


class Mesh final
{
public:
	Mesh(ID3D11Device* pDevice, Effect* pEffect, const std::vector<Vertex>& vertices, const std::vector<uint32_t> indices);

	~Mesh();
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	void Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjMatrix, Matrix viewInverseMatrix);

	Effect* GetEffect() const { return m_pEffect; }

	Matrix GetWorldMatrix() const { return m_WorldMatrix; };
	void SetWorldMatrix(const Matrix& worldMatrix) { m_WorldMatrix = worldMatrix; };


private:
	Effect* m_pEffect;

	ID3DX11EffectTechnique* m_pTechnique;

	ID3D11InputLayout* m_pInputLayout;

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	uint32_t m_NumIndices;

	Matrix m_WorldMatrix;


};

