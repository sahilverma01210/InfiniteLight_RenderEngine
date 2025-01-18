#pragma once
#include "../_External/dx12/directX12.h"
#include "../_External/common.h"
#include "../Common/ILUtility.h"

#ifdef _DEBUG
#define SHADER_DEBUG (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION)
#else
constexpr auto SHADER_DEBUG = 0;
#endif

namespace Renderer
{
	enum ShaderType
	{
		VertexShader,
		PixelShader
	};

	struct SHADER_MACRO
	{
		LPCWSTR Name;
        LPCWSTR Definition;
	};

	class D3D12Shader
	{
	public:
		D3D12Shader() = default;
        D3D12Shader(ShaderType shaderType, const std::wstring& path, const std::vector<SHADER_MACRO>& macros = {});
        CD3DX12_SHADER_BYTECODE GetShaderByteCode()
        {
            return m_shaderBlob ? CD3DX12_SHADER_BYTECODE(m_shaderBlob->GetBufferPointer(), m_shaderBlob->GetBufferSize()) : CD3DX12_SHADER_BYTECODE();
        }
	
	private:
		ShaderType m_shaderType;
		ComPtr<IDxcBlob> m_shaderBlob;
	};
}