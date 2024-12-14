#pragma once
#include "../_External/common.h"

#include "ILTimer.h"

namespace Common
{
	class ILPerfLog
	{
	private:
		struct Entry
		{
			Entry(std::string s, float t)
				:
				m_label(std::move(s)),
				m_time(t)
			{}
			void WriteTo(std::ostream& out) const noexcept(!IS_DEBUG)
			{
				using namespace std::string_literals;
				if (m_label.empty())
				{
					out << m_time * 1000.0f << "ms\n";
				}
				else
				{
					out << std::setw(16) << std::left << "["s + m_label + "] "s << std::setw(6) << std::right << m_time * 1000.0f << "ms\n";
				}
			}
			std::string m_label;
			float m_time;
		};

	private:
		ILPerfLog() noexcept(!IS_DEBUG)
		{
			m_entries.reserve(3000);
		}
		~ILPerfLog()
		{
			Flush_();
		}
		static ILPerfLog& Get_() noexcept(!IS_DEBUG)
		{
			static ILPerfLog log;
			return log;
		}
		void Start_(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			m_entries.emplace_back(label, 0.0f);
			m_timer.Mark();
		}
		void Mark_(const std::string& label = "") noexcept(!IS_DEBUG)
		{
			float t = m_timer.Peek();
			m_entries.emplace_back(label, t);
		}
		void Flush_()
		{
			std::ofstream file("logs\\performance.txt");
			file << std::setprecision(3) << std::fixed;
			for (const auto& e : m_entries)
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
		ILTimer m_timer;
		std::vector<Entry> m_entries;
	};
}