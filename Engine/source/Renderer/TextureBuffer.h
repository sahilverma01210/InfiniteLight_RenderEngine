#pragma once
#include "BindableCodex.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "DescriptorTable.h"

namespace Renderer
{
	class MeshTextureBuffer : public D3D12Resource, public Bindable
	{
	public:
		MeshTextureBuffer(D3D12RHI& gfx, std::string filename);
		~MeshTextureBuffer() = default;
		bool HasAlpha() const noexcept(!IS_DEBUG);
		static std::shared_ptr<MeshTextureBuffer> Resolve(D3D12RHI& gfx, std::string filename);
		static std::string GenerateUID(std::string filename);
		std::string GetUID() const noexcept(!IS_DEBUG) override;

	private:
		bool m_hasAlpha = false;
		std::string m_filename;
		ScratchImage m_mipChain;
		ComPtr<ID3D12Resource> m_texureUploadBuffer;
	};

	class CubeMapTextureBuffer : public D3D12Resource, public Bindable
	{
	public:
		CubeMapTextureBuffer(D3D12RHI& gfx, const WCHAR* foldername);
		~CubeMapTextureBuffer() = default;
		static std::shared_ptr<CubeMapTextureBuffer> Resolve(D3D12RHI& gfx, const WCHAR* filename);
		static std::string GenerateUID(const WCHAR* filename);
		std::string GetUID() const noexcept(!IS_DEBUG) override;

	private:
		const WCHAR* m_foldername;
		ComPtr<ID3D12Resource> m_texureUploadBuffer;
	};

	class DepthCubeMapTextureBuffer : public D3D12Resource, public Bindable
	{
	public:
		DepthCubeMapTextureBuffer(D3D12RHI& gfx, UINT size);
		~DepthCubeMapTextureBuffer() = default;
		std::shared_ptr<DepthStencil> GetDepthBuffer(size_t index) const;

	private:
		std::vector<std::shared_ptr<DepthStencil>> m_depthBuffers;
	};
}