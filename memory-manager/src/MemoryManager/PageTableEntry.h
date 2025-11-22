#pragma once

struct PTE
{
	uint32_t raw = 0;

	static constexpr uint32_t P = 1u << 0;
	static constexpr uint32_t RW = 1u << 1;
	static constexpr uint32_t US = 1u << 2;
	static constexpr uint32_t A = 1u << 5;
	static constexpr uint32_t D = 1u << 6;
	static constexpr uint32_t NX = 1u << 31;

	static constexpr uint32_t FRAME_SHIFT = 12;
	static constexpr uint32_t FRAME_MASK = 0xFFFFF000u;

	[[nodiscard]] uint32_t GetFrame() const noexcept
	{
		return (raw & FRAME_MASK) >> FRAME_SHIFT;
	}

	void SetFrame(uint32_t fn) noexcept
	{
		raw = (raw & ~FRAME_MASK) | (fn << FRAME_SHIFT);
	}

	[[nodiscard]] bool IsPresent() const noexcept
	{
		return raw & P;
	}
	void SetPresent(bool value) noexcept
	{
		raw = value ? (raw | P) : (raw & ~P);
	}

	[[nodiscard]] bool IsWritable() const noexcept
	{
		return raw & RW;
	}
	void SetWritable(bool value) noexcept
	{
		raw = value ? (raw | RW) : (raw & ~RW);
	}

	[[nodiscard]] bool IsUser() const noexcept
	{
		return raw & US;
	}
	void SetUser(bool value) noexcept
	{
		raw = value ? (raw | US) : (raw & ~US);
	}

	[[nodiscard]] bool IsAccessed() const noexcept
	{
		return raw & A;
	}
	void SetAccessed(bool value) noexcept
	{
		raw = value ? (raw | A) : (raw & ~A);
	}

	[[nodiscard]] bool IsDirty() const noexcept
	{
		return raw & D;
	}
	void SetDirty(bool value) noexcept
	{
		raw = value ? (raw | D) : (raw & ~D);
	}

	[[nodiscard]] bool IsNX() const noexcept
	{
		return raw & NX;
	}
	void SetNX(bool value) noexcept
	{
		raw = value ? (raw | NX) : (raw & ~NX);
	}
};