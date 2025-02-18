#pragma once
#include "../_External/common.h"

#include "Bindable.h"
#include "Resource.h"
#include "RenderGraphException.h"

namespace Renderer
{
	class Source
	{
	public:
		virtual ~Source() = default;
		const std::string& GetName() const noexcept(!IS_DEBUG);
		virtual void PostLinkValidate() const = 0;
		virtual std::shared_ptr<Bindable> YieldBindable();
		virtual std::shared_ptr<RenderGraphResource> YieldBuffer();
		virtual std::vector<std::shared_ptr<RenderGraphResource>> YieldBufferBucket();
	protected:
		Source(std::string name);

	public:
		bool m_isVector = false;
	private:
		std::string m_name;
	};

	template<class T>
	class DirectBufferSource : public Source
	{
	public:
		DirectBufferSource(std::string name, std::shared_ptr<T>& buffer)
			:
			Source(std::move(name)),
			m_buffer(buffer)
		{
		}
		static std::unique_ptr<DirectBufferSource> Make(std::string name, std::shared_ptr<T>& ebuffer)
		{
			return std::make_unique<DirectBufferSource>(std::move(name), ebuffer);
		}
		void PostLinkValidate() const
		{}
		std::shared_ptr<RenderGraphResource> YieldBuffer() override
		{
			if (m_linked)
			{
				throw RG_EXCEPTION("Mutable output bound twice: " + GetName());
			}
			m_linked = true;
			return m_buffer;
		}

	private:
		std::shared_ptr<T>& m_buffer;
		bool m_linked = false;
	};

	template<class T>
	class DirectBufferBucketSource : public Source
	{
	public:
		DirectBufferBucketSource(std::string name, std::vector<std::shared_ptr<T>>& bufferVector)
			:
			Source(std::move(name)),
			m_bufferVector(bufferVector)
		{
			m_isVector = true;
		}
		static std::unique_ptr<DirectBufferBucketSource> Make(std::string name, std::vector<std::shared_ptr<T>>& ebufferVector)
		{
			return std::make_unique<DirectBufferBucketSource>(std::move(name), ebufferVector);
		}
		void PostLinkValidate() const
		{}
		std::vector<std::shared_ptr<RenderGraphResource>> YieldBufferBucket() override
		{
			if (m_linked)
			{
				throw RG_EXCEPTION("Mutable output bound twice: " + GetName());
			}
			m_linked = true;
			std::vector<std::shared_ptr<RenderGraphResource>> RenderGraphResources;
			RenderGraphResources.reserve(m_bufferVector.size());
			std::transform(m_bufferVector.begin(), m_bufferVector.end(),
				std::back_inserter(RenderGraphResources),
				[](const std::shared_ptr<T>& buffer) {
					return std::static_pointer_cast<RenderGraphResource>(buffer);
				});
			return RenderGraphResources;
		}

	private:
		std::vector<std::shared_ptr<T>>& m_bufferVector;
		bool m_linked = false;
	};

	template<class T>
	class DirectBindableSource : public Source
	{
	public:
		DirectBindableSource(std::string name, std::shared_ptr<T>& bind)
			:
			Source(std::move(name)),
			m_bind(bind)
		{
		}
		static std::unique_ptr<DirectBindableSource> Make(std::string name, std::shared_ptr<T>& buffer)
		{
			return std::make_unique<DirectBindableSource>(std::move(name), buffer);
		}
		void PostLinkValidate() const
		{}
		std::shared_ptr<Bindable> YieldBindable() override
		{
			return m_bind;
		}

	private:
		std::shared_ptr<T>& m_bind;
	};
}