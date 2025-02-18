#include "D3D12Shader.h"

namespace Renderer
{
	D3D12Shader::D3D12Shader(ShaderType shaderType, std::wstring fileName, const std::vector<SHADER_MACRO>& macros)
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
        std::ifstream shaderFile(Common::GetAssetFullPath(fileName.c_str(), Common::AssetType::Shader), std::ios::binary);
        std::vector<char> shaderCode((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

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

		std::wstring assetsPath = Common::GetAssetsPath(Common::AssetType::Shader);

        std::vector<LPCWSTR> args = {
#ifdef _DEBUG
            DXC_ARG_DEBUG, 
            DXC_ARG_SKIP_OPTIMIZATIONS,
#endif
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
            throw std::exception(errors->GetStringPointer());
        }

        compileResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&m_shaderBlob), nullptr);

#ifdef _DEBUG
        {
            std::wstring pdbFolderPath = assetsPath + m_pdbFolderName.c_str();
            CreateDirectory(pdbFolderPath.c_str(), NULL);

            ComPtr<IDxcBlob> pdb_blob;
            ComPtr<IDxcBlobUtf16> pdb_path_utf16;
            if (SUCCEEDED(compileResult->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(pdb_blob.GetAddressOf()), pdb_path_utf16.GetAddressOf())))
            {
                ComPtr<IDxcBlobUtf8> pdb_path_utf8;
                if (SUCCEEDED(dxcUtils->GetBlobAsUtf8(pdb_path_utf16.Get(), pdb_path_utf8.GetAddressOf())))
                {
                    char pdb_path[256];
                    sprintf_s(pdb_path, "%s%s", Common::ToNarrow(pdbFolderPath + L"\\").c_str(), pdb_path_utf8->GetStringPointer());

                    FILE* pdb_file = nullptr;
                    fopen_s(&pdb_file, pdb_path, "wb");
                    if (pdb_file)
                    {
                        fwrite(pdb_blob->GetBufferPointer(), pdb_blob->GetBufferSize(), 1, pdb_file);
                        fclose(pdb_file);
                    }
                }
            }
        }
#endif
    }
}