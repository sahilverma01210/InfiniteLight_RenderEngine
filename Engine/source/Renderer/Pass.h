#pragma once
#include "Sink.h"

namespace Renderer
{
	class Pass
	{
	public:
		Pass(std::string name) noexcept(!IS_DEBUG);
		virtual void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) = 0;
		virtual void Reset() noexcept(!IS_DEBUG);
		const std::string& GetName() const noexcept(!IS_DEBUG);
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
		std::vector<std::unique_ptr<Sink>> m_sinks;
		std::vector<std::unique_ptr<Source>> m_sources;
		std::string m_name;
	};
}