#include "Pass.h"

namespace Renderer
{
	Pass::Pass(std::string name) noexcept(!IS_DEBUG)
		:
		name(std::move(name))
	{}

	void Pass::Reset() noexcept(!IS_DEBUG)
	{}

	const std::string& Pass::GetName() const noexcept(!IS_DEBUG)
	{
		return name;
	}

	void Pass::Finalize()
	{
		for (auto& in : sinks)
		{
			in->PostLinkValidate();
		}
		for (auto& out : sources)
		{
			out->PostLinkValidate();
		}
	}

	Pass::~Pass()
	{}

	const std::vector<std::unique_ptr<Sink>>& Pass::GetSinks() const
	{
		return sinks;
	}

	Source& Pass::GetSource(const std::string& name) const
	{
		for (auto& src : sources)
		{
			if (src->GetName() == name)
			{
				return *src;
			}
		}

		std::ostringstream oss;
		oss << "Output named [" << name << "] not found in pass: " << GetName();
		throw RG_EXCEPTION(oss.str());
	}

	Sink& Pass::GetSink(const std::string& registeredName) const
	{
		for (auto& si : sinks)
		{
			if (si->GetRegisteredName() == registeredName)
			{
				return *si;
			}
		}

		std::ostringstream oss;
		oss << "Input named [" << registeredName << "] not found in pass: " << GetName();
		throw RG_EXCEPTION(oss.str());
	}

	void Pass::RegisterSink(std::unique_ptr<Sink> sink)
	{
		// check for overlap of input names
		for (auto& si : sinks)
		{
			if (si->GetRegisteredName() == sink->GetRegisteredName())
			{
				throw RG_EXCEPTION("Registered input overlaps with existing: " + sink->GetRegisteredName());
			}
		}

		sinks.push_back(std::move(sink));
	}

	void Pass::RegisterSource(std::unique_ptr<Source> source)
	{
		// check for overlap of output names
		for (auto& src : sources)
		{
			if (src->GetName() == source->GetName())
			{
				throw RG_EXCEPTION("Registered output overlaps with existing: " + source->GetName());
			}
		}

		sources.push_back(std::move(source));
	}

	void Pass::SetSinkLinkage(const std::string& registeredName, const std::string& target)
	{
		auto& sink = GetSink(registeredName);
		auto targetSplit = SplitString(target, ".");
		if (targetSplit.size() != 2u)
		{
			throw RG_EXCEPTION("Input target has incorrect format");
		}
		sink.SetTarget(std::move(targetSplit[0]), std::move(targetSplit[1]));
	}
}