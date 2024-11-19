//#include "TestCube.h"
//#include "DynamicConstant.h"
//#include "TechniqueProbe.h"
//
//namespace Renderer
//{
//	TestCube::TestCube(D3D12RHI& gfx, float size)
//	{
//		auto model = Cube::MakeIndependentTextured();
//		model.Transform(XMMatrixScaling(size, size, size));
//		model.SetNormalsIndependentFlat();
//		const auto geometryTag = "$cube." + std::to_string(size);
//
//		m_numIndices = model.indices.size() * sizeof(model.indices[0]);
//		VertexRawBuffer vbuf = model.vertices;
//
//		// Define the vertex input layout.
//		std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
//		D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];
//
//		for (size_t i = 0; i < vec.size(); ++i) {
//			inputElementDescs[i] = vec[i];
//		}
//
//		//model.Transform(XMMatrixScaling(1.04f, 1.04f, 1.04f));
//		//model.SetNormalsIndependentFlat();
//		//const auto outline_geometryTag = "$outline_cube." + std::to_string(size);
//		//VertexRawBuffer outline_vbuf = model.vertices;
//
//		topologyBindable = std::move(Topology::Resolve(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
//		indexBufferBindable = std::move(IndexBuffer::Resolve(gfx, geometryTag, model.indices.size() * sizeof(model.indices[0]), model.indices));
//		vertexBufferBindable = std::move(VertexBuffer::Resolve(gfx, geometryTag, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size()));
//
//		{
//			Technique shade("Shade");
//			{
//				Step only(0);
//				{
//					std::unique_ptr<ShaderResourceView> srvBindablePtr;
//					// Add Pipeline State Obejct
//					{
//						ID3DBlob* vertexShader;
//						ID3DBlob* pixelShader;
//
//						// Compile Shaders.
//						D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongDif_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
//						D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongDif_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);
//
//						pipelineDesc.vertexShader = vertexShader;
//						pipelineDesc.pixelShader = pixelShader;
//						pipelineDesc.inputElementDescs = inputElementDescs;
//						pipelineDesc.numElements = vec.size();
//						pipelineDesc.numConstants = 1;
//						pipelineDesc.numConstantBufferViews = 3;
//						pipelineDesc.numSRVDescriptors = 1;
//						pipelineDesc.backFaceCulling = false;
//						pipelineDesc.depthStencilMode = Mode::Off;
//
//						rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
//						psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));						
//					}
//
//					{
//						srvBindablePtr = std::make_unique<ShaderResourceView>(gfx, 4, 1);
//						auto tex = TextureBuffer::Resolve(gfx, L"brickwall.jpg");
//						only.AddBindable(tex);
//						srvBindablePtr->AddResource(gfx, 0, dynamic_cast<TextureBuffer*>(tex.get())->GetBuffer());
//						only.AddBindable(std::move(srvBindablePtr));
//					}
//
//					only.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
//
//					RawLayout lay;
//					lay.Add<Float3>("specularColor");
//					lay.Add<Float>("specularWeight");
//					lay.Add<Float>("specularGloss");
//					auto buf = Buffer(std::move(lay));
//					buf["specularColor"] = dx::XMFLOAT3{ 1.0f,1.0f,1.0f };
//					buf["specularWeight"] = 0.1f;
//					buf["specularGloss"] = 20.0f;
//					only.AddBindable(std::make_shared<ConstantBuffer>(gfx, 2, (UINT)buf.GetRootLayoutElement().GetSizeInBytes(), (&buf)->GetData()));
//				}
//				shade.AddStep(std::move(only));
//			}
//			AddTechnique(std::move(shade));
//		}
//		
//		//{
//		//	Technique outline("Outline");
//		//	{
//		//		Step mask(1);
//		//		{
//		//			// Add Pipeline State Obejct
//		//			{
//		//				ID3DBlob* vertexShader;
//		//				ID3DBlob* pixelShader = nullptr;
//		//
//		//				// Compile Shaders.
//		//				D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidVS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
//		//				//D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongPS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);
//		//
//		//				PipelineDescription pipelineDesc{ vertexShader, pixelShader, inputElementDescs, vec.size(), 1, 2, 0, false, Mode::Mask };
//		//
//		//				mask.AddVertexBufferObject(std::move(VertexBuffer::Resolve(gfx, geometryTag, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size())));
//		//				mask.AddRootSignatureObject(std::move(std::make_unique<RootSignature>(gfx, pipelineDesc)));
//		//				mask.AddPipelineStateObject(std::move(std::make_unique<PipelineState>(gfx, pipelineDesc)));
//		//			}
//		//
//		//			mask.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
//		//		}
//		//		outline.AddStep(std::move(mask));
//		//
//		//		Step draw(2);
//		//		{
//		//			// Add Pipeline State Obejct
//		//			{
//		//				ID3DBlob* vertexShader;
//		//				ID3DBlob* pixelShader;
//		//		
//		//				// Compile Shaders.
//		//				D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidVS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
//		//				D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidPS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);
//		//		
//		//				PipelineDescription pipelineDesc{ vertexShader, pixelShader, inputElementDescs, vec.size(), 1, 3, 0, false, Mode::Write };
//		//		
//		//				draw.AddVertexBufferObject(std::move(VertexBuffer::Resolve(gfx, outline_geometryTag, outline_vbuf.GetData(), UINT(outline_vbuf.SizeBytes()), (UINT)outline_vbuf.GetLayout().Size())));
//		//				draw.AddRootSignatureObject(std::move(std::make_unique<RootSignature>(gfx, pipelineDesc)));
//		//				draw.AddPipelineStateObject(std::move(std::make_unique<PipelineState>(gfx, pipelineDesc)));
//		//			}
//		//		
//		//			draw.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
//		//
//		//			RawLayout lay;
//		//			lay.Add<Float4>("color");
//		//			auto buf = Buffer(std::move(lay));
//		//			buf["color"] = DirectX::XMFLOAT4{ 1.0f,0.4f,0.4f,1.0f };
//		//		
//		//			draw.AddBindable(std::make_shared<ConstantBuffer>(gfx, 2, (UINT)buf.GetRootLayoutElement().GetSizeInBytes(), (&buf)->GetData()));
//		//		}
//		//		outline.AddStep(std::move(draw));
//		//	}
//		//	AddTechnique(std::move(outline));
//		//}
//	}
//
//	void TestCube::SetPos(DirectX::XMFLOAT3 pos) noexcept
//	{
//		this->pos = pos;
//	}
//
//	void TestCube::SetRotation(float roll, float pitch, float yaw) noexcept
//	{
//		this->roll = roll;
//		this->pitch = pitch;
//		this->yaw = yaw;
//	}
//
//	DirectX::XMMATRIX TestCube::GetTransformXM() const noexcept
//	{
//		return XMMatrixRotationRollPitchYaw(roll, pitch, yaw) * XMMatrixTranslation(pos.x, pos.y, pos.z);
//	}
//
//	void TestCube::SpawnControlWindow(D3D12RHI& gfx, const char* name) noexcept
//	{
//		if (ImGui::Begin(name))
//		{
//			ImGui::Text("Position");
//			ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
//			ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
//			ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
//			ImGui::Text("Orientation");
//			ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
//			ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
//			ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
//			/*ImGui::Text("Shading");
//			bool changed0 = ImGui::SliderFloat("Spec. Int.", &pmc.specularIntensity, 0.0f, 1.0f);
//			bool changed1 = ImGui::SliderFloat("Spec. Power", &pmc.specularPower, 0.0f, 100.0f);
//			bool checkState = pmc.normalMappingEnabled == TRUE;
//			bool changed2 = ImGui::Checkbox("Enable Normal Map", &checkState);
//			pmc.normalMappingEnabled = checkState ? TRUE : FALSE;
//			if (changed0 || changed1 || changed2)
//			{
//				QueryBindable<ConstantBuffer>()->Update(gfx, &pmc);
//			}*/
//		}
//		ImGui::End();
//	}
//}