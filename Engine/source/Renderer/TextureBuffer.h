#pragma once
#include "BindableCodex.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "ShaderResourceView.h"

namespace Renderer
{
	class TextureBuffer : public Bindable
	{
	public:
		TextureBuffer(D3D12RHI& gfx, const WCHAR* filename);
		~TextureBuffer() = default;
		ID3D12Resource* GetBuffer();
		bool HasAlpha() const noexcept(!IS_DEBUG);
		bool HasAlphaChannel(const Image& image);
		static std::shared_ptr<TextureBuffer> Resolve(D3D12RHI& gfx, const WCHAR* filename);
		static std::string GenerateUID(const WCHAR* filename);
		std::string GetUID() const noexcept(!IS_DEBUG) override;

	private:
		bool m_hasAlpha = false;
		const WCHAR* m_filename;
		ComPtr<ID3D12Resource> m_texureBuffer;
	};

	class CubeMapTextureBuffer : public Bindable
	{
	public:
		CubeMapTextureBuffer(D3D12RHI& gfx, const WCHAR* foldername);
		~CubeMapTextureBuffer() = default;
		ID3D12Resource* GetBuffer();
		static std::shared_ptr<CubeMapTextureBuffer> Resolve(D3D12RHI& gfx, const WCHAR* filename);
		static std::string GenerateUID(const WCHAR* filename);
		std::string GetUID() const noexcept(!IS_DEBUG) override;

	private:
		const WCHAR* m_foldername;
		ComPtr<ID3D12Resource> m_texureBuffer;
	};

	class DepthCubeMapTextureBuffer : public Bindable
	{
	public:
		DepthCubeMapTextureBuffer(D3D12RHI& gfx, UINT size);
		~DepthCubeMapTextureBuffer() = default;
		ID3D12Resource* GetBuffer();
		std::shared_ptr<DepthStencil> GetDepthBuffer(size_t index) const;

	private:
		ComPtr<ID3D12Resource> m_texureBuffer;
		std::vector<std::shared_ptr<DepthStencil>> m_depthBuffers;
	};

	class TargetCubeMapTextureBuffer : public Bindable
	{
	public:
		TargetCubeMapTextureBuffer(D3D12RHI& gfx, UINT size);
		~TargetCubeMapTextureBuffer() = default;
		ID3D12Resource* GetBuffer();
		std::shared_ptr<RenderTarget> GetRenderTarget(size_t index) const;

	private:
		ComPtr<ID3D12Resource> m_texureBuffer;
		std::vector<std::shared_ptr<RenderTarget>> m_renderTargets;
	};
}