#include "pch.h"
#include "Mesh.h"
#include <cassert>

Mesh::Mesh(ID3D11Device* pDevice, Effect* pEffect, const std::vector<Vertex>& _vertices, const std::vector<uint32_t> _indices, const Vector3& position):
	vertices{_vertices},
	indices{_indices},
	vertices_out{}
{
	// Init the position
	Translate(position);

	// Hardware ---------------------------------------------------
	// Create an instance of the effect class
	m_pEffect = pEffect;
	//m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx");
	m_pTechnique = m_pEffect->GetTechnique();

	// Create vertex layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";						// VERT POSITION
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// VECTOR 3
	vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // AUTO ALIGN WITH LAST BYTE
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "NORMAL";							// NORMAL
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// VECTOR 3
	vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // AUTO ALIGN WITH LAST BYTE
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TANGENT";							// TANGENT
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// VECTOR 3
	vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // AUTO ALIGN WITH LAST BYTE
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXCOORD";						// UV
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;				// VECTOR 2
	vertexDesc[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; // AUTO ALIGN WITH LAST BYTE
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create input layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout
	);

	if(FAILED(result))
		assert(false);

	// Create vertex buffer
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());  // Warning, sizeof not matching pwp, pos3 or pos4?
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = vertices.data();

	result = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pVertexBuffer);
	if(FAILED(result))
		assert(false);


	// Create index buffer
	m_NumIndices = static_cast<uint32_t>(indices.size());
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	initData.pSysMem = indices.data();

	result = pDevice->CreateBuffer(&bufferDesc, &initData, &m_pIndexBuffer);
	if(FAILED(result))
		assert(false);

}

Mesh::~Mesh()
{
	m_pVertexBuffer->Release();
	m_pIndexBuffer->Release();
	m_pInputLayout->Release();
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, Matrix worldViewProjMatrix, Matrix viewInverseMatrix)
{
	// 1. Set Primitive Topolgy
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// 3. Set Vertex buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// 4. Set IndexBuffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 5. Draw
	// We reinterpret the pointer, not the object itself?
	m_pEffect->GetWorldViewProjVariable()->SetMatrix(reinterpret_cast<float*>(&worldViewProjMatrix));
	Matrix worldMatrix(GetWorldMatrix());
	m_pEffect->GetWorldMatrixVariable()->SetMatrix(reinterpret_cast<float*>(&worldMatrix));
	m_pEffect->GetViewInverseMatrixVariable()->SetMatrix(reinterpret_cast<float*>(&viewInverseMatrix));

	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pTechnique->GetDesc(&techDesc);
	for(UINT p{ 0 }; p < techDesc.Passes; ++p)
	{
		m_pTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}
