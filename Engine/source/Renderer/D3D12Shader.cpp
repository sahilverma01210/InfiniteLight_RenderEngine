#include "D3D12Shader.h"

namespace Renderer
{
	D3D12Shader::D3D12Shader(ShaderType shaderType, const std::wstring& path, const std::vector<SHADER_MACRO>& macros)
        :
        m_shaderType(shaderType)
    {
        // Initialize DXC components
        ComPtr<IDxcUtils> dxcUtils;
        ComPtr<IDxcCompiler3> dxcCompiler;
        ComPtr<IDxcIncludeHandler> includeHandler;

        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
        dxcUtils->CreateDefaultIncludeHandler(&includeHandler);

        // Read shader source code from file
        std::ifstream shaderFile(path, std::ios::binary);
        std::vector<char> shaderCode((std::istreambuf_iterator<char>(shaderFile)),
            std::istreambuf_iterator<char>());

        // Prepare the input source
        DxcBuffer sourceBuffer;
        sourceBuffer.Ptr = shaderCode.data();
        sourceBuffer.Size = shaderCode.size();
        sourceBuffer.Encoding = DXC_CP_ACP; // Assume ASCII/UTF-8

        // Compile the shader
        ComPtr<IDxcResult> compileResult;

        std::wstring entryPoint = L"main";
        std::wstring targetProfile;

		if (shaderType == ShaderType::VertexShader) targetProfile = L"vs_6_6"; // Vertex Shader Model 6.6
		else if (shaderType == ShaderType::PixelShader) targetProfile = L"ps_6_6"; // Pixel Shader Model 6.6

		std::wstring assetsPath = Common::GetAssetsPath();

        std::vector<LPCWSTR> args = {
            L"-E", entryPoint.c_str(),
			L"-T", targetProfile.c_str(),
			L"-I", assetsPath.c_str()
        };

        for (const auto& macro : macros) {
			args.push_back(L"-D");
			args.push_back(macro.Name);
			args.push_back(L"=");
			args.push_back(macro.Definition);
        }

		if (FAILED(dxcCompiler->Compile(&sourceBuffer, args.data(), args.size(), includeHandler.Get(), IID_PPV_ARGS(&compileResult)))) {
			throw std::exception("Failed to compile shader.");
		}

        // Handle the compilation result
        ComPtr<IDxcBlobUtf8> errors;
        compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
        if (errors && errors->GetStringLength() > 0) {
            char const* err_msg = errors->GetStringPointer();
            throw std::exception("Shader compilation failed.");
        }

        compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&m_shaderBlob), nullptr);
    }
}