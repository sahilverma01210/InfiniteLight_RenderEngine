#pragma once
#include "../_External/framework.h"
#include "../Common/ILUtility.h"

namespace Renderer
{
	class D3D12RHI;
	class RenderTarget;
	class DepthStencil;
	class Sink;
	class Source;

	class Pass
	{
	public:
		Pass(std::string name) noexcept;
		virtual void Execute(D3D12RHI& gfx) const noexcept = 0;
		virtual void Reset() noexcept;
		const std::string& GetName() const noexcept;
		const std::vector<std::unique_ptr<Sink>>& GetSinks() const;
		Source& GetSource(const std::string& registeredName) const;
		Sink& GetSink(const std::string& registeredName) const;
		void SetSinkLinkage(const std::string& registeredName, const std::string& target);
		virtual void Finalize();
		virtual ~Pass();
	protected:
		void RegisterSink(std::unique_ptr<Sink> sink);
		void RegisterSource(std::unique_ptr<Source> source);
	private:
		std::vector<std::unique_ptr<Sink>> sinks;
		std::vector<std::unique_ptr<Source>> sources;
		std::string name;
	};
}