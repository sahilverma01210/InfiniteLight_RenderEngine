#define DVTX_SOURCE_FILE
#include "Vertex.h"

namespace Renderer
{
	// VertexLayout Definitions.

	const VertexLayout::Element& VertexLayout::ResolveByIndex(size_t i) const noexcept
	{
		return elements[i];
	}

	VertexLayout& VertexLayout::Append(ElementType type) noexcept
	{
		if (!Has(type))
		{
			elements.emplace_back(type, Size());
		}
		return *this;
	}

	size_t VertexLayout::Size() const noexcept
	{
		return elements.empty() ? 0u : elements.back().GetOffsetAfter();
	}

	size_t VertexLayout::GetElementCount() const noexcept
	{
		return elements.size();
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> VertexLayout::GetD3DLayout() const noexcept
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> desc;
		desc.reserve(GetElementCount());
		for (const auto& e : elements)
		{
			desc.push_back(e.GetDesc());
		}
		return desc;
	}

	std::string VertexLayout::GetCode() const noexcept
	{
		std::string code;
		for (const auto& e : elements)
		{
			code += e.GetCode();
		}
		return code;
	}

	bool VertexLayout::Has(ElementType type) const noexcept
	{
		for (auto& e : elements)
		{
			if (e.GetType() == type)
			{
				return true;
			}
		}
		return false;
	}

	// VertexLayout::Element Definitions.

	VertexLayout::Element::Element(ElementType type, size_t offset)
		:
		type(type),
		offset(offset)
	{}

	size_t VertexLayout::Element::GetOffsetAfter() const noexcept
	{
		return offset + Size();
	}

	size_t VertexLayout::Element::GetOffset() const
	{
		return offset;
	}

	size_t VertexLayout::Element::Size() const noexcept
	{
		return SizeOf(type);
	}

	VertexLayout::ElementType VertexLayout::Element::GetType() const noexcept
	{
		return type;
	}

	template<VertexLayout::ElementType type>
	struct SysSizeLookup
	{
		static constexpr auto Exec() noexcept
		{
			return sizeof(VertexLayout::Map<type>::SysType);
		}
	};
	constexpr size_t VertexLayout::Element::SizeOf(ElementType type) noexcept
	{
		return Bridge<SysSizeLookup>(type);
	}
	template<VertexLayout::ElementType type>
	struct CodeLookup
	{
		static constexpr auto Exec() noexcept
		{
			return VertexLayout::Map<type>::code;
		}
	};
	const char* VertexLayout::Element::GetCode() const noexcept
	{
		return Bridge<CodeLookup>(type);
	}
	template<VertexLayout::ElementType type> struct DescGenerate {
		static constexpr D3D12_INPUT_ELEMENT_DESC Exec(size_t offset) noexcept {
			return {
				VertexLayout::Map<type>::semantic,0,
				VertexLayout::Map<type>::dxgiFormat,
				0,(UINT)offset,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA ,0
			};
		}
	};
	D3D12_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noexcept
	{
		return Bridge<DescGenerate>(type, GetOffset());
	}

	// Vertex Definitions.

	Vertex::Vertex(char* pData, const VertexLayout& layout) noexcept
		:
		pData(pData),
		layout(layout)
	{
		assert(pData != nullptr);
	}

	ConstVertex::ConstVertex(const Vertex& v) noexcept
		:
		vertex(v)
	{}

	// VertexBuffer Definitions.

	VertexRawBuffer::VertexRawBuffer() noexcept
	{
	}

	VertexRawBuffer::VertexRawBuffer(VertexLayout layout) noexcept
		:
		layout(std::move(layout))
	{}

	template<VertexLayout::ElementType type>
	struct AttributeAiMeshFill
	{
		static constexpr void Exec(VertexRawBuffer* pBuf, const aiMesh& mesh) noexcept
		{
			for (auto end = mesh.mNumVertices, i = 0u; i < end; i++)
			{
				(*pBuf)[i].Attr<type>() = VertexLayout::Map<type>::Extract(mesh, i);
			}
		}
	};

	VertexRawBuffer::VertexRawBuffer(VertexLayout layout_in, const aiMesh& mesh)
		:
		layout(std::move(layout_in))
	{
		Resize(mesh.mNumVertices);
		for (size_t i = 0, end = layout.GetElementCount(); i < end; i++)
		{
			VertexLayout::Bridge<AttributeAiMeshFill>(layout.ResolveByIndex(i).GetType(), this, mesh);
		}
	}

	const char* VertexRawBuffer::GetData() const noexcept
	{
		return buffer.data();
	}

	void VertexRawBuffer::SetLayout(VertexLayout pLayout) noexcept
	{
		layout = std::move(pLayout);
	}

	const VertexLayout& VertexRawBuffer::GetLayout() const noexcept
	{
		return layout;
	}

	size_t VertexRawBuffer::Size() const noexcept
	{
		return buffer.size() / layout.Size();
	}

	size_t VertexRawBuffer::SizeBytes() const noexcept
	{
		return buffer.size();
	}

	void VertexRawBuffer::Resize(size_t newSize)
	{
		const auto size = Size();
		if (size < newSize)
		{
			buffer.resize(buffer.size() + layout.Size() * (newSize - size));
		}
	}

	Vertex VertexRawBuffer::Back() noexcept
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data() + buffer.size() - layout.Size(),layout };
	}

	Vertex VertexRawBuffer::Front() noexcept
	{
		assert(buffer.size() != 0u);
		return Vertex{ buffer.data(),layout };
	}

	Vertex VertexRawBuffer::operator[](size_t i) noexcept
	{
		assert(i < Size());
		return Vertex{ buffer.data() + layout.Size() * i,layout };
	}

	ConstVertex VertexRawBuffer::Back() const noexcept
	{
		return const_cast<VertexRawBuffer*>(this)->Back();
	}

	ConstVertex VertexRawBuffer::Front() const noexcept
	{
		return const_cast<VertexRawBuffer*>(this)->Front();
	}

	ConstVertex VertexRawBuffer::operator[](size_t i) const noexcept
	{
		return const_cast<VertexRawBuffer&>(*this)[i];
	}
}