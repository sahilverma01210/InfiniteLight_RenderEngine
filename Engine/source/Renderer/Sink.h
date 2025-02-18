#pragma once
#include "../Common/ILUtility.h"

#include "Source.h"
#include "CommonBindables.h"
#include "Resource.h"

namespace Renderer
{
	class Sink
	{
	public:
		virtual ~Sink() = default;
		const std::string& GetRegisteredName() const noexcept(!IS_DEBUG);
		const std::string& GetPassName() const noexcept(!IS_DEBUG);
		const std::string& GetOutputName() const noexcept(!IS_DEBUG);
		void SetTarget(std::string passName, std::string outputName);
		virtual void Bind(Source& source) = 0;
		virtual void PostLinkValidate() const = 0;		
	protected:
		Sink(std::string registeredName);

	public:
		bool m_isVector = false;
	private:
		std::string m_registeredName;
		std::string m_passName;
		std::string m_outputName;
	};

	template<class T>
	class DirectBufferSink : public Sink
	{
		static_assert(std::is_base_of_v<RenderGraphResource, T>, "DirectBufferSink target type must be a RenderGraphResource type");
	public:
		DirectBufferSink(std::string registeredName, std::shared_ptr<T>& bind)
			:
			Sink(std::move(registeredName)),
			m_target(bind)
		{
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::shared_ptr<T>& target)
		{
			return std::make_unique<DirectBufferSink>(std::move(registeredName), target);
		}
		void PostLinkValidate() const override
		{
			if (!m_linked)
			{
				throw RG_EXCEPTION("Unlinked input: " + GetRegisteredName());
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
				throw RG_EXCEPTION(oss.str());
			}
			m_target = std::move(p);
			m_linked = true;
		}

	private:
		std::shared_ptr<T>& m_target;
		bool m_linked = false;
	};

	template<class T>
	class DirectBufferBucketSink : public Sink
	{
		static_assert(std::is_base_of_v<RenderGraphResource, T>, "DirectBufferSink target type must be a RenderGraphResource type");
	public:
		DirectBufferBucketSink(std::string registeredName, std::vector<std::shared_ptr<T>>& bindVector)
			:
			Sink(std::move(registeredName)),
			m_targetVector(bindVector)
		{
			m_isVector = true;
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::vector<std::shared_ptr<T>>& targetVector)
		{
			return std::make_unique<DirectBufferBucketSink>(std::move(registeredName), targetVector);
		}
		void PostLinkValidate() const override
		{
			if (!m_linked)
			{
				throw RG_EXCEPTION("Unlinked input: " + GetRegisteredName());
			}
		}
		void Bind(Source& source) override
		{
			if (source.m_isVector)
			{
				for (std::shared_ptr<RenderGraphResource> buffer : source.YieldBufferBucket())
				{
					auto p = std::dynamic_pointer_cast<T>(buffer);
					m_targetVector.push_back(std::move(p));
				}
				m_linked = true;
			}
		}

	private:
		std::vector<std::shared_ptr<T>>& m_targetVector;
		bool m_linked = false;
	};

	template<class T>
	class ContainerBindableSink : public Sink
	{
		static_assert(std::is_base_of_v<Bindable, T>, "DirectBindableSink target type must be a Bindable type");
	public:
		ContainerBindableSink(std::string registeredName, std::vector<std::shared_ptr<Bindable>>& container, size_t index)
			:
			Sink(std::move(registeredName)),
			m_container(container),
			m_index(index)
		{
		}
		void PostLinkValidate() const override
		{
			if (!m_linked)
			{
				throw RG_EXCEPTION("Unlinked input: " + GetRegisteredName());
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
				throw RG_EXCEPTION(oss.str());
			}
			m_container[m_index] = std::move(p);
			m_linked = true;
		}

	private:
		std::vector<std::shared_ptr<Bindable>>& m_container;
		size_t m_index;
		bool m_linked = false;
	};

	template<class T>
	class DirectBindableSink : public Sink
	{
		static_assert(std::is_base_of_v<Bindable, T>, "DirectBindableSink target type must be a Bindable type");
	public:
		DirectBindableSink(std::string registeredName, std::shared_ptr<T>& target)
			:
			Sink(std::move(registeredName)),
			m_target(target)
		{
		}
		static std::unique_ptr<Sink> Make(std::string registeredName, std::shared_ptr<T>& target)
		{
			return std::make_unique<DirectBindableSink>(std::move(registeredName), target);
		}
		void PostLinkValidate() const override
		{
			if (!m_linked)
			{
				throw RG_EXCEPTION("Unlinked input: " + GetRegisteredName());
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
				throw RG_EXCEPTION(oss.str());
			}
			m_target = std::move(p);
			m_linked = true;
		}

	private:
		std::shared_ptr<T>& m_target;
		bool m_linked = false;
	};
}