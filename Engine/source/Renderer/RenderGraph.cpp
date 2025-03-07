#include "RenderGraph.h"

namespace Renderer
{
	RenderGraph::RenderGraph(D3D12RHI& gfx)
	{
		for (UINT n = 0; n < gfx.GetTargetBuffers().size(); n++)
		{
			RenderTarget* rt = new RenderTarget(gfx, gfx.GetTargetBuffers()[n].Get());
			gfx.LoadResource(std::shared_ptr<RenderTarget>(rt), ResourceType::Texture);
		}

		gfx.LoadResource(std::make_shared<DepthStencil>(gfx), ResourceType::Texture);
	}

	RenderGraph::~RenderGraph()
	{}

	void RenderGraph::SetSinkTarget(const std::string& sinkName, const std::string& target)
	{
		const auto finder = [&sinkName](const std::unique_ptr<Sink>& p) {
			return p->GetRegisteredName() == sinkName;
			};
		const auto i = std::find_if(m_globalSinks.begin(), m_globalSinks.end(), finder);
		if (i == m_globalSinks.end())
		{
			throw RG_EXCEPTION("Global sink does not exist: " + sinkName);
		}
		auto targetSplit = SplitString(target, ".");
		if (targetSplit.size() != 2u)
		{
			throw RG_EXCEPTION("Input target has incorrect format");
		}
		(*i)->SetTarget(targetSplit[0], targetSplit[1]);
	}

	void RenderGraph::AddGlobalSource(std::unique_ptr<Source> out)
	{
		m_globalSources.push_back(std::move(out));
	}

	void RenderGraph::AddGlobalSink(std::unique_ptr<Sink> in)
	{
		m_globalSinks.push_back(std::move(in));
	}

	void RenderGraph::Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		assert(m_finalized);
		for (auto& pass : m_passes)
		{
			pass->Execute(gfx);
		}
	}

	void RenderGraph::Reset() noexcept(!IS_DEBUG)
	{
		assert(m_finalized);
		for (auto& pass : m_passes)
		{
			pass->Reset();
		}
	}

	void RenderGraph::AppendPass(std::unique_ptr<Pass> pass)
	{
		assert(!m_finalized);
		// validate name uniqueness
		for (const auto& existingPass : m_passes)
		{
			if (pass->GetName() == existingPass->GetName())
			{
				throw RG_EXCEPTION("Pass name already exists: " + pass->GetName());
			}
		}

		// link outputs from m_passes (and global outputs) to pass inputs
		LinkSinks(*pass);

		// add to container of m_passes
		m_passes.push_back(std::move(pass));
	}

	Pass& RenderGraph::FindPassByName(const std::string& name)
	{
		const auto i = std::find_if(m_passes.begin(), m_passes.end(), [&name](auto& p) {
			return p->GetName() == name;
			});
		if (i == m_passes.end())
		{
			throw std::runtime_error{ "Failed to find pass name" };
		}
		return **i;
	}

	void RenderGraph::LinkSinks(Pass& pass)
	{
		for (auto& sink : pass.GetSinks())
		{
			const auto& inputSourcePassName = sink->GetPassName();

			if (inputSourcePassName.empty())
			{
				std::ostringstream oss;
				oss << "In pass named [" << pass.GetName() << "] sink named [" << sink->GetRegisteredName() << "] has no target source set.";
				throw RG_EXCEPTION(oss.str());
			}

			// check check whether target source is global
			if (inputSourcePassName == "$")
			{
				bool bound = false;
				for (auto& source : m_globalSources)
				{
					if (source->GetName() == sink->GetOutputName())
					{
						sink->Bind(*source);
						bound = true;
						break;
					}
				}
				if (!bound)
				{
					std::ostringstream oss;
					oss << "Output named [" << sink->GetOutputName() << "] not found in globals";
					throw RG_EXCEPTION(oss.str());
				}
			}
			else // find source from within existing m_passes
			{
				bool bound = false;
				for (auto& existingPass : m_passes)
				{
					if (existingPass->GetName() == inputSourcePassName)
					{
						auto& source = existingPass->GetSource(sink->GetOutputName());
						sink->Bind(source);
						bound = true;
						break;
					}
				}
				if (!bound)
				{
					std::ostringstream oss;
					oss << "Pass named [" << inputSourcePassName << "] not found";
					throw RG_EXCEPTION(oss.str());
				}
			}
		}
	}

	void RenderGraph::LinkGlobalSinks()
	{
		for (auto& sink : m_globalSinks)
		{
			const auto& inputSourcePassName = sink->GetPassName();
			for (auto& existingPass : m_passes)
			{
				if (existingPass->GetName() == inputSourcePassName)
				{
					auto& source = existingPass->GetSource(sink->GetOutputName());
					sink->Bind(source);
					break;
				}
			}
		}
	}

	void RenderGraph::Finalize()
	{
		assert(!m_finalized);
		for (const auto& pass : m_passes)
		{
			pass->Finalize();
		}
		LinkGlobalSinks();
		m_finalized = true;
	}

	RenderPass& RenderGraph::GetRenderQueue(const std::string& passName)
	{
		try
		{
			for (const auto& pass : m_passes)
			{
				if (pass->GetName() == passName)
				{
					return dynamic_cast<RenderPass&>(*pass);
				}
			}
		}
		catch (std::bad_cast&)
		{
			throw RG_EXCEPTION("In RenderGraph::GetRenderQueue, pass was not RenderPass: " + passName);
		}
		throw RG_EXCEPTION("In RenderGraph::GetRenderQueue, pass not found: " + passName);
	}
}