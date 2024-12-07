#pragma once
#include "../_External/common.h"
#include "../Common/ILTimer.h"



namespace Renderer
{
	class PerfLog
	{
	private:
		struct Entry
		{
			Entry(std::string s, float t)
				:
				label(std::move(s)),
				time(t)
			{}
			void WriteTo(std::ostream& out) const noexcept(!IS_DEBUG)
			{
				using namespace std::string_literals;
				if (label.empty())
				{
					out << time * 1000.0f << "ms\n";
				}
				else
				{
					out << std::setw(16) << std::left << "["s + label + "] "s << std::setw(6) << std::right << time * 1000.0f << "ms\n";
				}
			}
			std::string label;
			float time;
		};

	private:
		PerfLog() noexcept(!IS_DEBUG)
		{
			entries.reserve(3000);
		}
		~PerfLog()
		{
			Flush_();
		}
		static PerfLog& Get_() noexcept(!IS_DEBUG)
		{
			static PerfLog log;
			return log;
		}
		void Start_(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			entries.emplace_back(label, 0.0f);
			timer.Mark();
		}
		void Mark_(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			float t = timer.Peek();
			entries.emplace_back(label, t);
		}
		void Flush_()
		{
			std::ofstream file("logs\\performance.txt");
			file << std::setprecision(3) << std::fixed;
			for (const auto& e : entries)
			{
				e.WriteTo(file);
			}
		}
	public:
		static void Start(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			Get_().Start_(label);
		}
		static void Mark(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			Get_().Mark_(label);
		}

	private:
		ILTimer timer;
		std::vector<Entry> entries;
	};
}