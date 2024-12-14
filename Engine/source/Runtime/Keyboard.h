#pragma once
#include "../_External/common.h"

namespace Runtime
{
	class Keyboard
	{
		friend class Window;

	public:
		class Event
		{
		public:
			enum class Type
			{
				Press,
				Release,
				Invalid
			};

		public:
			Event() noexcept(!IS_DEBUG)
				:
				m_type(Type::Invalid),
				m_code(0u)
			{}
			Event(Type type, unsigned char code) noexcept(!IS_DEBUG)
				:
				m_type(type),
				m_code(code)
			{}
			bool IsPress() const noexcept(!IS_DEBUG)
			{
				return m_type == Type::Press;
			}
			bool IsRelease() const noexcept(!IS_DEBUG)
			{
				return m_type == Type::Release;
			}
			bool IsValid() const noexcept(!IS_DEBUG)
			{
				return m_type != Type::Invalid;
			}
			unsigned char GetCode() const noexcept(!IS_DEBUG)
			{
				return m_code;
			}

		private:
			Type m_type;
			unsigned char m_code;
		};

	public:
		Keyboard() = default;
		Keyboard(const Keyboard&) = delete;
		Keyboard& operator=(const Keyboard&) = delete;
		// key event stuff
		bool KeyIsPressed(unsigned char keycode) const noexcept(!IS_DEBUG);
		Event ReadKey() noexcept(!IS_DEBUG);
		bool KeyIsEmpty() const noexcept(!IS_DEBUG);
		void FlushKey() noexcept(!IS_DEBUG);
		// char event stuff
		char ReadChar() noexcept(!IS_DEBUG);
		bool CharIsEmpty() const noexcept(!IS_DEBUG);
		void FlushChar() noexcept(!IS_DEBUG);
		void Flush() noexcept(!IS_DEBUG);
		// autorepeat control
		void EnableAutorepeat() noexcept(!IS_DEBUG);
		void DisableAutorepeat() noexcept(!IS_DEBUG);
		bool AutorepeatIsEnabled() const noexcept(!IS_DEBUG);
	private:
		void OnKeyPressed(unsigned char keycode) noexcept(!IS_DEBUG);
		void OnKeyReleased(unsigned char keycode) noexcept(!IS_DEBUG);
		void OnChar(char character) noexcept(!IS_DEBUG);
		void ClearState() noexcept(!IS_DEBUG);
		template<typename T>
		static void TrimBuffer(std::queue<T>& buffer) noexcept(!IS_DEBUG);

	private:
		static constexpr unsigned int m_nKeys = 256u;
		static constexpr unsigned int m_bufferSize = 16u;
		bool m_autorepeatEnabled = false;
		std::bitset<m_nKeys> m_keystates;
		std::queue<Event> m_keybuffer;
		std::queue<char> m_charbuffer;
	};
}