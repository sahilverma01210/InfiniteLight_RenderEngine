#pragma once
#include "../_External/dx12/directX12.h"
#include "../_External/common.h"

// master list of leaf types that generates enum elements and various switches etc.
#define LEAF_ELEMENT_TYPES \
	X( Float_DC ) \
	X( Float2_DC ) \
	X( Float3_DC ) \
	X( Float4_DC ) \
	X( Matrix_DC ) \
	X( Bool_DC ) \
	X( Integer_DC )

namespace Renderer
{
	enum Type
	{
#define X(el) el,
		LEAF_ELEMENT_TYPES
#undef X
		Struct_DC,
		Array_DC,
		Empty_DC,
	};

	// static map of attributes of each leaf type
	template<Type type>
	struct Map
	{
		static constexpr bool valid = false;
	};
	template<> struct Map<Float_DC>
	{
		using SysType = float; // type used in the CPU side
		static constexpr size_t hlslSize = sizeof(SysType); // size of type on GPU side
		static constexpr const char* code = "F1"; // code used for generating signature of layout
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>
	};
	template<> struct Map<Float2_DC>
	{
		using SysType = XMFLOAT2;
		static constexpr size_t hlslSize = sizeof(SysType);
		static constexpr const char* code = "F2";
		static constexpr bool valid = true;
	};
	template<> struct Map<Float3_DC>
	{
		using SysType = XMFLOAT3;
		static constexpr size_t hlslSize = sizeof(SysType);
		static constexpr const char* code = "F3";
		static constexpr bool valid = true;
	};
	template<> struct Map<Float4_DC>
	{
		using SysType = XMFLOAT4;
		static constexpr size_t hlslSize = sizeof(SysType);
		static constexpr const char* code = "F4";
		static constexpr bool valid = true;
	};
	template<> struct Map<Matrix_DC>
	{
		using SysType = XMFLOAT4X4;
		static constexpr size_t hlslSize = sizeof(SysType);
		static constexpr const char* code = "M4";
		static constexpr bool valid = true;
	};
	template<> struct Map<Bool_DC>
	{
		using SysType = bool;
		static constexpr size_t hlslSize = 4u;
		static constexpr const char* code = "BL";
		static constexpr bool valid = true;
	};
	template<> struct Map<Integer_DC>
	{
		using SysType = int;
		static constexpr size_t hlslSize = sizeof(SysType);
		static constexpr const char* code = "IN";
		static constexpr bool valid = true;
	};

	// ensures that every leaf type in master list has an entry in the static attribute map
#define X(el) static_assert(Map<el>::valid,"Missing map implementation for " #el);
	LEAF_ELEMENT_TYPES
#undef X
	// enables reverse lookup from SysType to leaf type
	template<typename T>
	struct ReverseMap
	{
		static constexpr bool valid = false;
	};
#define X(el) \
	template<> struct ReverseMap<typename Map<el>::SysType> \
	{ \
		static constexpr Type type = el; \
		static constexpr bool valid = true; \
	};
	LEAF_ELEMENT_TYPES
#undef X

	// LayoutElements instances form a tree that describes the layout of the data buffer
		// supporting nested aggregates of structs and arrays
	class LayoutElement
	{
		friend class RawLayout;
		friend struct ExtraData;

	private:
		// this forms the polymorpic base for extra data that Struct and Array have
		struct ExtraDataBase
		{
			virtual ~ExtraDataBase() = default;
		};

		// friend relationships are used liberally throught the DynamicConstant system
		// instead of seeing the various classes in this system as encapsulated decoupled
		// units, they must be viewed as aspect of one large monolithic system
		// the reason for the friend relationships is generally so that intermediate
		// classes that the client should not create can have their constructors made
		// private, so that Finalize() cannot be called on arbitrary LayoutElements, etc.
	public:
		// get a string signature for this element (recursive); when called on the root
		// element of a layout tree, generates a uniquely-identifying string for the layout
		std::string GetSignature() const ;
		// Check if element is "real"
		bool Exists() const noexcept(!IS_DEBUG);
		// calculate array indexing offset
		std::pair<size_t, const LayoutElement*> CalculateIndexingOffset(size_t offset, size_t index) const ;
		// [] only works for Structs; access member (child node in tree) by name
		LayoutElement& operator[](const std::string& key) ;
		const LayoutElement& operator[](const std::string& key) const ;
		// T() only works for Arrays; gets the array type layout object
		// needed to further configure an array's type
		LayoutElement& T() ;
		const LayoutElement& T() const ;
		// offset based- functions only work after finalization!
		size_t GetOffsetBegin() const ;
		size_t GetOffsetEnd() const ;
		// get size in bytes derived from offsets
		size_t GetSizeInBytes() const ;
		// only works for Structs; add LayoutElement to struct
		LayoutElement& Add(Type addedType, std::string name) ;
		template<Type typeAdded>
		LayoutElement& Add(std::string key) 
		{
			return Add(typeAdded, std::move(key));
		}
		// only works for Arrays; set the type and the # of elements
		LayoutElement& Set(Type addedType, size_t size) ;
		template<Type typeAdded>
		LayoutElement& Set(size_t size) 
		{
			return Set(typeAdded, size);
		}
		// returns offset of leaf types for read/write purposes w/ typecheck in Debug
		template<typename T>
		size_t Resolve() const 
		{
			switch (type)
			{
#define X(el) case el: assert(typeid(Map<el>::SysType) == typeid(T)); return *offset;
				LEAF_ELEMENT_TYPES
#undef X
			default:
				assert("Tried to resolve non-leaf element" && false);
				return 0u;
			}
		}
	private:
		// construct an empty layout element
		LayoutElement() noexcept(!IS_DEBUG) = default;
		LayoutElement(Type typeIn) ;
		// sets all offsets for element and subelements, prepending padding when necessary
		// returns offset directly after this element
		size_t Finalize(size_t offsetIn) ;
		// implementations for GetSignature for aggregate types
		std::string GetSignatureForStruct() const ;
		std::string GetSignatureForArray() const ;
		// implementations for Finalize for aggregate types
		size_t FinalizeForStruct(size_t offsetIn);
		size_t FinalizeForArray(size_t offsetIn);
		// returns singleton instance of empty layout element
		static LayoutElement& GetEmptyElement() noexcept(!IS_DEBUG)
		{
			static LayoutElement empty{};
			return empty;
		}
		// returns the value of offset bumped up to the next 16-byte boundary (if not already on one)
		static size_t AdvanceToBoundary(size_t offset) noexcept(!IS_DEBUG);
		// return true if a memory block crosses a boundary
		static bool CrossesBoundary(size_t offset, size_t size) noexcept(!IS_DEBUG);
		// advance an offset to next boundary if block crosses a boundary
		static size_t AdvanceIfCrossesBoundary(size_t offset, size_t size) noexcept(!IS_DEBUG);
		// check string for validity as a struct key
		static bool ValidateSymbolName(const std::string& name) noexcept(!IS_DEBUG);

	private:
		// each element stores its own offset. this makes lookup to find its position in the byte buffer
		// fast. Special handling is required for situations where arrays are involved
		std::optional<size_t> offset;
		Type type = Empty_DC;
		std::unique_ptr<ExtraDataBase> pExtraData;
	};

	// the layout class serves as a shell to hold the root of the LayoutElement tree
	// client does not create LayoutElements directly, create a raw layout and then
	// use it to access the elements and add on from there. When building is done,
	// raw layout is moved to Codex (usually via Buffer::Make), and the internal layout
	// element tree is "delivered" (finalized and moved out). Codex returns a baked
	// layout, which the buffer can then use to initialize itself. Baked layout can
	// also be used to directly init multiple Buffers. Baked layouts are conceptually
	// immutable. Base Layout class cannot be constructed.
	class Layout
	{
		friend class LayoutCodex;
		friend class Buffer;

	public:
		size_t GetSizeInBytes() const noexcept(!IS_DEBUG);
		std::string GetSignature() const ;
	protected:
		Layout(std::shared_ptr<LayoutElement> pRoot) noexcept(!IS_DEBUG);

	protected:
		std::shared_ptr<LayoutElement> pRoot;
	};

	// Raw layout represents a layout that has not yet been finalized and registered
	// structure can be edited by adding layout nodes
	class RawLayout : public Layout
	{
		friend class LayoutCodex;

	public:
		RawLayout() noexcept(!IS_DEBUG);
		// key into the root Struct
		LayoutElement& operator[](const std::string& key) ;
		// add an element to the root Struct
		template<Type type>
		LayoutElement& Add(const std::string& key) 
		{
			return pRoot->Add<type>(key);
		}
	private:
		// reset this object with an empty struct at its root
		void ClearRoot() noexcept(!IS_DEBUG);
		// finalize the layout and then relinquish (by yielding the root layout element)
		std::shared_ptr<LayoutElement> DeliverRoot() noexcept(!IS_DEBUG);
	};

	// CookedLayout represend a completed and registered Layout shell object
	// layout tree is fixed
	class CookedLayout : public Layout
	{
		friend class LayoutCodex;
		friend class Buffer;

	public:
		// key into the root Struct (const to disable mutation of the layout)
		const LayoutElement& operator[](const std::string& key) const ;
		// get a share on layout tree root
		std::shared_ptr<LayoutElement> ShareRoot() const noexcept(!IS_DEBUG);
	private:
		// this ctor used by Codex to return cooked layouts
		CookedLayout(std::shared_ptr<LayoutElement> pRoot) noexcept(!IS_DEBUG);
		// use to pilfer the layout tree
		std::shared_ptr<LayoutElement> RelinquishRoot() const noexcept(!IS_DEBUG);
	};

	class LayoutCodex
	{
	public:
		static CookedLayout Resolve(RawLayout&& layout);
	private:
		static LayoutCodex& Get_() noexcept(!IS_DEBUG);

	private:
		std::unordered_map<std::string, std::shared_ptr<LayoutElement>> map;
	};

	// proxy type that is emitted when keying/indexing into a Buffer
	// implement conversions/assignment that allows manipulation of the
	// raw bytes of the Buffer. This version is const, only supports reading
	// Refs can be further keyed/indexed to traverse the layout structure
	class ConstElementRef
	{
		friend class Buffer;
		friend class ElementRef;

	public:
		// this is a proxy type emitted when you use addressof& on the Ref
		// it allows conversion to pointer type, useful for using Buffer
		// elements with ImGui widget functions etc.
		class Ptr
		{
			friend ConstElementRef;

		public:
			// conversion for getting read-only pointer to supported SysType
			template<typename T>
			operator const T* () const 
			{
				static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
				return &static_cast<const T&>(*ref);
			}
		private:
			Ptr(const ConstElementRef* ref) noexcept(!IS_DEBUG);

		private:
			const ConstElementRef* ref;
		};

	public:
		// check if the indexed element actually exists
		// this is possible because if you key into a Struct with a nonexistent key
		// it will still return an Empty LayoutElement that will enable this test
		// but will not enable any other kind of access
		bool Exists() const noexcept(!IS_DEBUG);
		// key into the current element as a struct
		ConstElementRef operator[](const std::string& key) const ;
		// index into the current element as an array
		ConstElementRef operator[](size_t index) const ;
		// emit a pointer proxy object
		Ptr operator&() const ;
		// conversion for reading as a supported SysType
		template<typename T>
		operator const T& () const 
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
			return *reinterpret_cast<const T*>(pBytes + offset + pLayout->Resolve<T>());
		}
	private:
		// refs should only be constructable by other refs or by the buffer
		ConstElementRef(const LayoutElement* pLayout, const char* pBytes, size_t offset) noexcept(!IS_DEBUG);

	private:
		// this offset is the offset that is built up by indexing into arrays
		// accumulated for every array index in the path of access into the structure
		size_t offset;
		const LayoutElement* pLayout;
		const char* pBytes;
	};

	// version of ConstElementRef that also allows writing to the bytes of Buffer
	// see above in ConstElementRef for detailed description
	class ElementRef
	{
		friend class Buffer;

	public:
		class Ptr
		{
			friend ElementRef;

		public:
			// conversion to read/write pointer to supported SysType
			template<typename T>
			operator T* () const 
			{
				static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
				return &static_cast<T&>(*ref);
			}
		private:
			Ptr(ElementRef* ref) noexcept(!IS_DEBUG);

		private:
			ElementRef* ref;
		};

	public:
		operator ConstElementRef() const noexcept(!IS_DEBUG);
		bool Exists() const noexcept(!IS_DEBUG);
		ElementRef operator[](const std::string& key) const ;
		ElementRef operator[](size_t index) const ;
		// optionally set value if not an empty Ref
		template<typename S>
		bool SetIfExists(const S& val) 
		{
			if (Exists())
			{
				*this = val;
				return true;
			}
			return false;
		}
		Ptr operator&() const ;
		// conversion for reading/writing as a supported SysType
		template<typename T>
		operator T& () const 
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
			return *reinterpret_cast<T*>(pBytes + offset + pLayout->Resolve<T>());
		}
		// assignment for writing to as a supported SysType
		template<typename T>
		T& operator=(const T& rhs) const 
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment");
			return static_cast<T&>(*this) = rhs;
		}
	private:
		// refs should only be constructable by other refs or by the buffer
		ElementRef(const LayoutElement* pLayout, char* pBytes, size_t offset) noexcept(!IS_DEBUG);

	private:
		size_t offset;
		const LayoutElement* pLayout;
		char* pBytes;
	};

	// The buffer object is a combination of a raw byte buffer with a LayoutElement
	// tree structure which acts as an view/interpretation/overlay for those bytes
	// operator [] indexes into the root Struct, returning a Ref shell that can be
	// used to further index if struct/array, returning further Ref shells, or used
	// to access the data stored in the buffer if a Leaf element type
	class Buffer
	{
	public:
		// various resources can be used to construct a Buffer
		Buffer(RawLayout&& lay) ;
		Buffer(const CookedLayout& lay) ;
		Buffer(CookedLayout&& lay) ;
		Buffer(const Buffer&) noexcept(!IS_DEBUG);
		// have to be careful with this one...
		// the buffer that has once been pilfered must not be used :x
		Buffer(Buffer&&) noexcept(!IS_DEBUG);
		// how you begin indexing into buffer (root is always Struct)
		ElementRef operator[](const std::string& key) ;
		// if Buffer is const, you only get to index into the buffer with a read-only proxy
		ConstElementRef operator[](const std::string& key) const ;
		// get the raw bytes
		const char* GetData() const noexcept(!IS_DEBUG);
		// size of the raw byte buffer
		size_t GetSizeInBytes() const noexcept(!IS_DEBUG);
		const LayoutElement& GetRootLayoutElement() const noexcept(!IS_DEBUG);
		// copy bytes from another buffer (layouts must match)
		void CopyFrom(const Buffer&) ;
		// return another sptr to the layout root
		std::shared_ptr<LayoutElement> ShareLayoutRoot() const noexcept(!IS_DEBUG);

	private:
		std::shared_ptr<LayoutElement> pLayoutRoot;
		std::vector<char> bytes;
	};
}

#ifndef DCB_IMPL_SOURCE
#undef LEAF_ELEMENT_TYPES
#endif