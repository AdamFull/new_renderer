#pragma once

#include <cstdint>

namespace engine
{
	class Stopwatch
	{
	public:
		Stopwatch();
		~Stopwatch();

		float ElapsedSeconds() const;
		float ElapsedMilliseconds() const;
		float ElapsedMicroseconds() const;
	private:
		// Opaque data type to hide platform-specific details
		struct StopwatchImpl;
		StopwatchImpl* impl;  // Pointer to the implementation
	};

	class FPSCounter : public Stopwatch
	{
	public:
		FPSCounter();
		void Frame();
		float GetFps() const;
		float GetFrameTime() const;
		uint32_t GetTotalFrames() const;
	private:
		uint32_t total_frame_count;
		int32_t frame_count;
		float last_time, delta_time, accumulated_time;
		float fps;
	};
}