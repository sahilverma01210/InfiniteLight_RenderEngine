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
		elements.emplace_back(type, Size());
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

	constexpr size_t VertexLayout::Element::SizeOf(ElementType type) noexcept
	{
		using namespace DirectX;
		switch (type)
		{
		case Position2D:
			return sizeof(Map<Position2D>::SysType);
		case Position3D:
			return sizeof(Map<Position3D>::SysType);
		case Texture2D:
			return sizeof(Map<Texture2D>::SysType);
		case Normal:
			return sizeof(Map<Normal>::SysType);
		case Tangent:
			return sizeof(Map<Tangent>::SysType);
		case Bitangent:
			return sizeof(Map<Bitangent>::SysType);
		case Float3Color:
			return sizeof(Map<Float3Color>::SysType);
		case Float4Color:
			return sizeof(Map<Float4Color>::SysType);
		case BGRAColor:
			return sizeof(Map<BGRAColor>::SysType);
		}
		assert("Invalid element type" && false);
		return 0u;
	}

	VertexLayout::ElementType VertexLayout::Element::GetType() const noexcept
	{
		return type;
	}

	const char* VertexLayout::Element::GetCode() const noexcept
	{
		switch (type)
		{
		case Position2D:
			return Map<Position2D>::code;
		case Position3D:
			return Map<Position3D>::code;
		case Texture2D:
			return Map<Texture2D>::code;
		case Normal:
			return Map<Normal>::code;
		case Tangent:
			return Map<Tangent>::code;
		case Bitangent:
			return Map<Bitangent>::code;
		case Float3Color:
			return Map<Float3Color>::code;
		case Float4Color:
			return Map<Float4Color>::code;
		case BGRAColor:
			return Map<BGRAColor>::code;
		}
		assert("Invalid element type" && false);
		return "Invalid";
	}

	D3D12_INPUT_ELEMENT_DESC VertexLayout::Element::GetDesc() const noexcept
	{
		switch (type)
		{
		case Position2D:
			return GenerateDesc<Position2D>(GetOffset());
		case Position3D:
			return GenerateDesc<Position3D>(GetOffset());
		case Texture2D:
			return GenerateDesc<Texture2D>(GetOffset());
		case Normal:
			return GenerateDesc<Normal>(GetOffset());
		case Tangent:
			return GenerateDesc<Tangent>(GetOffset());
		case Bitangent:
			return GenerateDesc<Bitangent>(GetOffset());
		case Float3Color:
			return GenerateDesc<Float3Color>(GetOffset());
		case Float4Color:
			return GenerateDesc<Float4Color>(GetOffset());
		case BGRAColor:
			return GenerateDesc<BGRAColor>(GetOffset());
		}
		assert("Invalid element type" && false);
		return { "INVALID",0,DXGI_FORMAT_UNKNOWN,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 };
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