#pragma once
#include "../Common/ILUtility.h"

#include "Source.h"
#include "Bindable.h"
#include "BufferResource.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "RenderGraphCompileException.h"

namespace Renderer
{
	class Bindable;
	class Pass;

	class Sink
	{
	public:
		virtual ~Sink() = default;
		const std::string& GetRegisteredName() const noexcept;
		const std::string& GetPassName() const noexcept;
		const std::string& GetOutputName() const noexcept;
		void SetTarget(std::string passName, std::string outputName);
		virtual void Bind(Source& source) = 0;
		virtual void PostLinkValidate() const = 0;
		bool isVector = false;
	protected:
		Sink(std::string registeredName);

	private:
		std::string registeredName;
		std::string passName;
		std::string outputName;
	};

	template<class T>
	class DirectBufferSink : public Sink
	{
		static_assert(std::is_base_of_v<BufferResource, T>, "DirectBufferSink target type must be a BufferResource type");
	public:
		DirectBufferSink(std::string registeredName, std::shared_ptr<T>& bind)
			:
			Sink(std::move(registeredName)),
			target(bind)
		{
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::shared_ptr<T>& target)
		{
			return std::make_unique<DirectBufferSink>(std::move(registeredName), target);
		}
		void PostLinkValidate() const override
		{
			if (!linked)
			{
				throw RGC_EXCEPTION("Unlinked input: " + GetRegisteredName());
			}
		}
		void Bind(Source& source) override
		{
			auto p = std::dynamic_pointer_cast<T>(source.YieldBuffer());
			if (!p)
			{
				std::ostringstream oss;
				oss << "Binding input [" << GetRegisteredName() << "] to output [" << GetPassName() << "." << GetOutputName() << "] "
					<< " { " << typeid(T).name() << " } not compatible with { " << typeid(*source.YieldBuffer().get()).name() << " }";
				throw RGC_EXCEPTION(oss.str());
			}
			target = std::move(p);
			linked = true;
		}

	private:
		std::shared_ptr<T>& target;
		bool linked = false;
	};

	template<class T>
	class DirectBufferBucketSink : public Sink
	{
		static_assert(std::is_base_of_v<BufferResource, T>, "DirectBufferSink target type must be a BufferResource type");
	public:
		DirectBufferBucketSink(std::string registeredName, std::vector<std::shared_ptr<T>>& bindVector)
			:
			Sink(std::move(registeredName)),
			targetVector(bindVector)
		{
			isVector = true;
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::vector<std::shared_ptr<T>>& targetVector)
		{
			return std::make_unique<DirectBufferBucketSink>(std::move(registeredName), targetVector);
		}
		void PostLinkValidate() const override
		{
			if (!linked)
			{
				throw RGC_EXCEPTION("Unlinked input: " + GetRegisteredName());
			}
		}
		void Bind(Source& source) override
		{
			if (source.isVector)
			{
				for (std::shared_ptr<BufferResource> buffer : source.YieldBufferBucket())
				{
					auto p = std::dynamic_pointer_cast<T>(buffer);
					targetVector.push_back(std::move(p));
				}
				linked = true;
			}
		}

	private:
		std::vector<std::shared_ptr<T>>& targetVector;
		bool linked = false;
	};

	template<class T>
	class ContainerBindableSink : public Sink
	{
		static_assert(std::is_base_of_v<Bindable, T>, "DirectBindableSink target type must be a Bindable type");
	public:
		ContainerBindableSink(std::string registeredName, std::vector<std::shared_ptr<Bindable>>& container, size_t index)
			:
			Sink(std::move(registeredName)),
			container(container),
			index(index)
		{
		}
		void PostLinkValidate() const override
		{
			if (!linked)
			{
				throw RGC_EXCEPTION("Unlinked input: " + GetRegisteredName());
			}
		}
		void Bind(Source& source) override
		{
			auto p = std::dynamic_pointer_cast<T>(source.YieldBindable());
			if (!p)
			{
				std::ostringstream oss;
				oss << "Binding input [" << GetRegisteredName() << "] to output [" << GetPassName() << "." << GetOutputName() << "] "
					<< " { " << typeid(T).name() << " } does not match { " << typeid(*source.YieldBindable().get()).name() << " }";
				throw RGC_EXCEPTION(oss.str());
			}
			container[index] = std::move(p);
			linked = true;
		}

	private:
		std::vector<std::shared_ptr<Bindable>>& container;
		size_t index;
		bool linked = false;
	};

	template<class T>
	class DirectBindableSink : public Sink
	{
		static_assert(std::is_base_of_v<Bindable, T>, "DirectBindableSink target type must be a Bindable type");
	public:
		DirectBindableSink(std::string registeredName, std::shared_ptr<T>& target)
			:
			Sink(std::move(registeredName)),
			target(target)
		{
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::shared_ptr<T>& target)
		{
			return std::make_unique<DirectBindableSink>(std::move(registeredName), target);
		}
		void PostLinkValidate() const override
		{
			if (!linked)
			{
				throw RGC_EXCEPTION("Unlinked input: " + GetRegisteredName());
			}
		}
		void Bind(Source& source) override
		{
			auto p = std::dynamic_pointer_cast<T>(source.YieldBindable());
			if (!p)
			{
				std::ostringstream oss;
				oss << "Binding input [" << GetRegisteredName() << "] to output [" << GetPassName() << "." << GetOutputName() << "] "
					<< " { " << typeid(T).name() << " } does not match { " << typeid(*source.YieldBindable().get()).name() << " }";
				throw RGC_EXCEPTION(oss.str());
			}
			target = std::move(p);
			linked = true;
		}

	private:
		std::shared_ptr<T>& target;
		bool linked = false;
	};
}