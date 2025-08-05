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
		else if (shaderType == ShaderType::ComputeShader) targetProfile = L"cs_6_6"; // Compute Shader Model 6.6
		else if (shaderType == ShaderType::LibraryShader) targetProfile = L"lib_6_6"; // Library Shader Model 6.6

		std::wstring assetsPath = Common::GetAssetsPath(Common::AssetType::Shader);

        std::vector<LPCWSTR> args = {
#ifdef _DEBUG
            DXC_ARG_DEBUG, 
            DXC_ARG_SKIP_OPTIMIZATIONS,
#endif
            L"-E", entryPoint.c_str(),
			L"-T", targetProfile.c_str(),
			L"-I", assetsPath.c_str(),
            L"-no-warnings",
            L"-Qembed_debug"  // Embed the PDB data for shader reflection
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
			std::string errorMessage = errors->GetStringPointer();
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

    DXGI_FORMAT D3D12Shader::GetDXGIFormatFromSignature(D3D_REGISTER_COMPONENT_TYPE type, BYTE mask)
    {
        // Determine the number of components from the mask.
        int componentCount = 0;
        if (mask & 1) componentCount++; // R
        if (mask & 2) componentCount++; // G
        if (mask & 4) componentCount++; // B
        if (mask & 8) componentCount++; // A

        switch (type)
        {
        case D3D_REGISTER_COMPONENT_UINT32:
        {
            switch (componentCount)
            {
            case 1: return DXGI_FORMAT_R32_UINT;
            case 2: return DXGI_FORMAT_R32G32_UINT;
            case 3: return DXGI_FORMAT_R32G32B32_UINT;
            case 4: return DXGI_FORMAT_R32G32B32A32_UINT;
            }
            break;
        }
        case D3D_REGISTER_COMPONENT_SINT32:
        {
            switch (componentCount)
            {
            case 1: return DXGI_FORMAT_R32_SINT;
            case 2: return DXGI_FORMAT_R32G32_SINT;
            case 3: return DXGI_FORMAT_R32G32B32_SINT;
            case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
            }
            break;
        }
        case D3D_REGISTER_COMPONENT_FLOAT32:
        {
            switch (componentCount)
            {
            case 1: return DXGI_FORMAT_R32_FLOAT;
            case 2: return DXGI_FORMAT_R32G32_FLOAT;
            case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
            case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            break;
        }
        }

        return DXGI_FORMAT_UNKNOWN;
    }

    D3D12_INPUT_LAYOUT_DESC D3D12Shader::GenerateInputLayoutFromDXC()
    {
        D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

        IDxcContainerReflection* pReflection;
        DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&pReflection));
        pReflection->Load(m_shaderBlob.Get());

        UINT32 shaderPart;
        pReflection->FindFirstPartKind(DXC_PART_PDB, &shaderPart);

        ID3D12ShaderReflection* pReflector;
        pReflection->GetPartReflection(shaderPart, IID_PPV_ARGS(&pReflector));

        D3D12_SHADER_DESC shaderDesc;
        pReflector->GetDesc(&shaderDesc);

        D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[shaderDesc.InputParameters];

        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
            pReflector->GetInputParameterDesc(i, &paramDesc);

            D3D12_INPUT_ELEMENT_DESC elementDesc = {};
            elementDesc.SemanticName = paramDesc.SemanticName;
            elementDesc.SemanticIndex = paramDesc.SemanticIndex;
            elementDesc.Format = GetDXGIFormatFromSignature(paramDesc.ComponentType, paramDesc.Mask);
            elementDesc.InputSlot = 0; // Assume all data comes from slot 0
            elementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
            elementDesc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            elementDesc.InstanceDataStepRate = 0;

            inputElements[i] = elementDesc;
        }

        inputLayoutDesc.pInputElementDescs = inputElements;
        inputLayoutDesc.NumElements = static_cast<UINT>(shaderDesc.InputParameters);

        return inputLayoutDesc;
    }
}