#pragma once
#include "GraphicsResource.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "DescriptorTable.h"

namespace Renderer
{
	class MeshTexture : public D3D12Resource, public GraphicsResource
	{
	public:
		MeshTexture(D3D12RHI& gfx, std::string filename, bool isSRGB = false);
		~MeshTexture() = default;

	private:
		std::string m_filename;
		ScratchImage m_mipChain;
		ComPtr<ID3D12Resource> m_texureUploadBuffer;
	};

	class CubeMapTexture : public D3D12Resource, public GraphicsResource
	{
	public:
		CubeMapTexture(D3D12RHI& gfx, const WCHAR* foldername);
		~CubeMapTexture() = default;

	private:
		const WCHAR* m_foldername;
		ComPtr<ID3D12Resource> m_texureUploadBuffer;
	};
}