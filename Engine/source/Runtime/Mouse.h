#pragma once
#include "../_External/common.h"

namespace Runtime
{
	class Mouse
	{
		friend class Window;

	public:
		struct RawDelta
		{
			int x, y;
		};
		class Event
		{
		public:
			enum class Type
			{
				LPress,
				LRelease,
				RPress,
				RRelease,
				WheelUp,
				WheelDown,
				Move,
				Enter,
				Leave,
				Invalid
			};

		public:
			Event() noexcept(!IS_DEBUG)
				:
				type(Type::Invalid),
				leftIsPressed(false),
				rightIsPressed(false),
				x(0),
				y(0)
			{}
			Event(Type type, const Mouse& parent) noexcept(!IS_DEBUG)
				:
				type(type),
				leftIsPressed(parent.leftIsPressed),
				rightIsPressed(parent.rightIsPressed),
				x(parent.x),
				y(parent.y)
			{}
			bool IsValid() const noexcept(!IS_DEBUG)
			{
				return type != Type::Invalid;
			}
			Type GetType() const noexcept(!IS_DEBUG)
			{
				return type;
			}
			std::pair<int, int> GetPos() const noexcept(!IS_DEBUG)
			{
				return{ x,y };
			}
			int GetPosX() const noexcept(!IS_DEBUG)
			{
				return x;
			}
			int GetPosY() const noexcept(!IS_DEBUG)
			{
				return y;
			}
			bool LeftIsPressed() const noexcept(!IS_DEBUG)
			{
				return leftIsPressed;
			}
			bool RightIsPressed() const noexcept(!IS_DEBUG)
			{
				return rightIsPressed;
			}

		private:
			Type type;
			bool leftIsPressed;
			bool rightIsPressed;
			int x;
			int y;
		};

	public:
		Mouse() = default;
		Mouse(const Mouse&) = delete;
		Mouse& operator=(const Mouse&) = delete;
		std::pair<int, int> GetPos() const noexcept(!IS_DEBUG);
		std::optional<RawDelta> ReadRawDelta() noexcept(!IS_DEBUG);
		int GetPosX() const noexcept(!IS_DEBUG);
		int GetPosY() const noexcept(!IS_DEBUG);
		bool IsInWindow() const noexcept(!IS_DEBUG);
		bool LeftIsPressed() const noexcept(!IS_DEBUG);
		bool RightIsPressed() const noexcept(!IS_DEBUG);
		Mouse::Event Read() noexcept(!IS_DEBUG);
		bool IsEmpty() const noexcept(!IS_DEBUG)
		{
			return buffer.empty();
		}
		void Flush() noexcept(!IS_DEBUG);
		void EnableRaw() noexcept(!IS_DEBUG);
		void DisableRaw() noexcept(!IS_DEBUG);
		bool RawEnabled() const noexcept(!IS_DEBUG);
	private:
		void OnMouseMove(int x, int y) noexcept(!IS_DEBUG);
		void OnMouseLeave() noexcept(!IS_DEBUG);
		void OnMouseEnter() noexcept(!IS_DEBUG);
		void OnRawDelta(int dx, int dy) noexcept(!IS_DEBUG);
		void OnLeftPressed(int x, int y) noexcept(!IS_DEBUG);
		void OnLeftReleased(int x, int y) noexcept(!IS_DEBUG);
		void OnRightPressed(int x, int y) noexcept(!IS_DEBUG);
		void OnRightReleased(int x, int y) noexcept(!IS_DEBUG);
		void OnWheelUp(int x, int y) noexcept(!IS_DEBUG);
		void OnWheelDown(int x, int y) noexcept(!IS_DEBUG);
		void TrimBuffer() noexcept(!IS_DEBUG);
		void TrimRawInputBuffer() noexcept(!IS_DEBUG);
		void OnWheelDelta(int x, int y, int delta) noexcept(!IS_DEBUG);

	private:
		static constexpr unsigned int bufferSize = 16u;
		int x = 0;
		int y = 0;
		bool leftIsPressed = false;
		bool rightIsPressed = false;
		bool isInWindow = false;
		int wheelDeltaCarry = 0;
		bool rawEnabled = false;
		std::queue<Event> buffer;
		std::queue<RawDelta> rawDeltaBuffer;
	};
}