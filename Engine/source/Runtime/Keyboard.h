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
				type(Type::Invalid),
				code(0u)
			{}
			Event(Type type, unsigned char code) noexcept(!IS_DEBUG)
				:
				type(type),
				code(code)
			{}
			bool IsPress() const noexcept(!IS_DEBUG)
			{
				return type == Type::Press;
			}
			bool IsRelease() const noexcept(!IS_DEBUG)
			{
				return type == Type::Release;
			}
			bool IsValid() const noexcept(!IS_DEBUG)
			{
				return type != Type::Invalid;
			}
			unsigned char GetCode() const noexcept(!IS_DEBUG)
			{
				return code;
			}

		private:
			Type type;
			unsigned char code;
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
		static constexpr unsigned int nKeys = 256u;
		static constexpr unsigned int bufferSize = 16u;
		bool autorepeatEnabled = false;
		std::bitset<nKeys> keystates;
		std::queue<Event> keybuffer;
		std::queue<char> charbuffer;
	};
}