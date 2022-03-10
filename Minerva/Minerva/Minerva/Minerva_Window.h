#pragma once

namespace Minerva
{
	class CommandBuffer;
	class Pipeline;

	class Window
	{
	public:
		enum class RenderStatus : uint8_t
		{
			WINDOW_RESIZED = 0,
			WINDOW_MINIMIZED ,
			RENDER_OK
		};

		Window(Minerva::Device& _device, bool _fullscreen = false, bool _vsync = true, int _width = 1920, int _height = 1080);
		bool ProcessInput();
        
		inline std::shared_ptr<Minerva::Vulkan::Window> GetVKWindowHandle() const;

		inline void SetFullscreen(bool _fullscreen);
		inline void SetVSync(bool _vsync);

		inline int GetWidth() const;
		inline int GetHeight() const;
		inline void SetWidth(int _width);
		inline void SetHeight(int _height);

		inline Minerva::CommandBuffer GetCommandBuffer();
		inline bool BeginRender(Minerva::Pipeline& _pipeline);
		inline void PageFlip(Minerva::Pipeline& _pipeline);

	private:
		

		std::shared_ptr<Minerva::Vulkan::Window> m_VKWindowHandle;
	};
}
