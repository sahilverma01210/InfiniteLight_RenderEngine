#pragma once
#include "../_External/common.h"

#include "RenderGraphException.h"

namespace Renderer
{
	class Bindable;
	class BufferResource;

	class Source
	{
	public:
		virtual ~Source() = default;
		const std::string& GetName() const noexcept(!IS_DEBUG);
		virtual void PostLinkValidate() const = 0;
		virtual std::shared_ptr<Bindable> YieldBindable();
		virtual std::shared_ptr<BufferResource> YieldBuffer();
		virtual std::vector<std::shared_ptr<BufferResource>> YieldBufferBucket();
	protected:
		Source(std::string name);

	public:
		bool isVector = false;
	private:
		std::string name;
	};

	template<class T>
	class DirectBufferSource : public Source
	{
	public:
		DirectBufferSource(std::string name, std::shared_ptr<T>& buffer)
			:
			Source(std::move(name)),
			buffer(buffer)
		{
		}
		static std::unique_ptr<DirectBufferSource> Make(std::string name, std::shared_ptr<T>& ebuffer)
		{
			return std::make_unique<DirectBufferSource>(std::move(name), ebuffer);
		}
		void PostLinkValidate() const
		{}
		std::shared_ptr<BufferResource> YieldBuffer() override
		{
			if (linked)
			{
				throw RG_EXCEPTION("Mutable output bound twice: " + GetName());
			}
			linked = true;
			return buffer;
		}

	private:
		std::shared_ptr<T>& buffer;
		bool linked = false;
	};

	template<class T>
	class DirectBufferBucketSource : public Source
	{
	public:
		DirectBufferBucketSource(std::string name, std::vector<std::shared_ptr<T>>& bufferVector)
			:
			Source(std::move(name)),
			bufferVector(bufferVector)
		{
			isVector = true;
		}
		static std::unique_ptr<DirectBufferBucketSource> Make(std::string name, std::vector<std::shared_ptr<T>>& ebufferVector)
		{
			return std::make_unique<DirectBufferBucketSource>(std::move(name), ebufferVector);
		}
		void PostLinkValidate() const
		{}
		std::vector<std::shared_ptr<BufferResource>> YieldBufferBucket() override
		{
			if (linked)
			{
				throw RG_EXCEPTION("Mutable output bound twice: " + GetName());
			}
			linked = true;
			std::vector<std::shared_ptr<BufferResource>> bufferResources;
			bufferResources.reserve(bufferVector.size());
			std::transform(bufferVector.begin(), bufferVector.end(),
				std::back_inserter(bufferResources),
				[](const std::shared_ptr<T>& buffer) {
					return std::static_pointer_cast<BufferResource>(buffer);
				});
			return bufferResources;
		}

	private:
		std::vector<std::shared_ptr<T>>& bufferVector;
		bool linked = false;
	};

	template<class T>
	class DirectBindableSource : public Source
	{
	public:
		DirectBindableSource(std::string name, std::shared_ptr<T>& bind)
			:
			Source(std::move(name)),
			bind(bind)
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
			return bind;
		}

	private:
		std::shared_ptr<T>& bind;
	};
}