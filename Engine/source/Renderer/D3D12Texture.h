#pragma once
#include "GraphicsResource.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "DescriptorTable.h"

namespace Renderer
{
	struct TextureDesc
	{
		bool isSRGB = false;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	};

	class MeshTexture : public D3D12Resource, public GraphicsResource
	{
	public:
		MeshTexture(D3D12RHI& gfx, std::string filename, TextureDesc desc = TextureDesc{});
		~MeshTexture() = default;

	private:
		D3D12RHI& m_gfx;
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
		D3D12RHI& m_gfx;
		const WCHAR* m_foldername;
		ComPtr<ID3D12Resource> m_texureUploadBuffer;
	};
}