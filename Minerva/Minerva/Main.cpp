/*
	TODOs:
		ASAP - Abstract Pipeline Layout, clean up CommandBuffer::PushConstants, Descriptors
		1. Extrapolate color space/format settings to user
		2. Extrapolate image usage(swap chain) settings to user
		3. Extrapolate pipeline settings to users
		4. Extrapolate renderpass settings to users
*/

#include "Minerva/Minerva.h"

void LogError(const std::string_view _str)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

	std::cout << _str << "\n";
}

void LogWarn(const std::string_view _str)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	std::cout << _str << "\n";
}

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
};

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 prog;
};

struct PushConstant
{
	glm::mat4 MVP;
};


int main(int argc, const char* argv[])
{

		
		try
		{
			// Initialize Input Manager
			Minerva::Input::Initialize();


			// Create Instance
			Minerva::Instance instance("Homework 2", 0, true, true, LogWarn, LogError);

			// Create Device
			Minerva::Device device(instance, Minerva::Device::QueueFamily::RENDER_AND_SWAP, Minerva::Device::Type::DISCRETE_ONLY);

			// Create Window
			Minerva::Window window(device, false, true, 1280, 720);

			// Setup Renderpass
			std::array<float, 4> clearColor{ 0.f, 0.f, 0.f, 1.f };
			Minerva::Renderpass renderpass(device, window, clearColor.data());

			// Setup Attributes
			std::array<Minerva::VertexDescriptor::Attribute, 2> attributes{
				Minerva::VertexDescriptor::Attribute // Position
				{
					.m_Offset = offsetof(Vertex, pos),
					.m_Format = Minerva::VertexDescriptor::Format::FLOAT_3D
				},
				Minerva::VertexDescriptor::Attribute // Color
				{
					.m_Offset = offsetof(Vertex, color),
					.m_Format = Minerva::VertexDescriptor::Format::FLOAT_3D
				}
			};
			// Create Vertex Descriptor
			Minerva::VertexDescriptor vertexDescriptor(attributes, sizeof(Vertex), Minerva::VertexDescriptor::Topology::TRIANGLE_LIST);



			// Load Shaders
			std::vector<Minerva::Shader> shaders;
			shaders.emplace_back(device, "Assets\\Shaders\\vert.spv", Minerva::Shader::Type::VERTEX);
			shaders.emplace_back(device, "Assets\\Shaders\\frag.spv", Minerva::Shader::Type::FRAGMENT);
			// Setup Pipeline
			Minerva::Pipeline pipeline(device, window, renderpass, shaders.data(), shaders.size(), vertexDescriptor);

			// Vertices and Indices raw data
			const std::vector<Vertex> vertices{
				// front
				{ {-0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f} },
				{ {0.5f, -0.5f, 0.5f},	 {0.f, 1.f, 0.f} },
				{ {0.5f,  0.5f,  0.5f},  {0.f, 0.f, 1.f} },
				{ {-0.5f,  0.5f,  0.5f}, {1.f, 0.f, 0.f} },

				// back
				{ {-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f} },
				{ {-0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f} },
				{ {0.5f,  0.5f, -0.5f},  {0.f, 0.f, 1.f} },
				{ {0.5f, -0.5f, -0.5f},  {1.f, 0.f, 0.f}},

				// top
				{ {-0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f} },
				{ {-0.5f,  0.5f,  0.5f}, {0.f, 1.f, 0.f} },
				{ {0.5f,  0.5f,  0.5f},  {0.f, 0.f, 1.f} },
				{ {0.5f,  0.5f, -0.5f},  {1.f, 0.f, 0.f} },

				// bottom
				{ {-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f} },
				{ {0.5f, -0.5f, -0.5f},  {0.f, 1.f, 0.f} },
				{ {0.5f, -0.5f,  0.5f},  {0.f, 0.f, 1.f} },
				{ {-0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f} },

				// right
				{ {0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f} },
				{ {0.5f,  0.5f, -0.5f}, {0.f, 1.f, 0.f} },
				{ {0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f} },
				{ {0.5f, -0.5f,  0.5f}, {1.f, 0.f, 0.f} },

				// left
				{ {-0.5f, -0.5f, -0.5f}, {1.f, 0.f, 0.f} },
				{ {-0.5f, -0.5f,  0.5f}, {0.f, 1.f, 0.f} },
				{ {-0.5f,  0.5f,  0.5f}, {0.f, 0.f, 1.f} },
				{ {-0.5f,  0.5f, -0.5f}, {1.f, 0.f, 0.f} }
			};

			const std::vector<uint16_t> indices
			{
				0,  1,  2,      0,  2,  3,    // front
				4,  5,  6,      4,  6,  7,    // back
				8,  9,  10,     8,  10, 11,   // top
				12, 13, 14,     12, 14, 15,   // bottom
				16, 17, 18,     16, 18, 19,   // right
				20, 21, 22,     20, 22, 23    // left
			};

			// Setup buffers
			Minerva::Buffer vertexBuffer(device, Minerva::Buffer::Type::VERTEX, vertices.data(), vertices.size() * sizeof(Vertex));
			Minerva::Buffer indexBuffer(device, Minerva::Buffer::Type::INDEX, indices.data(), indices.size() * sizeof(uint16_t));

			//make a model view matrix for rendering the object
			//camera position
			glm::vec3 camPos = { 0.f,0.f,-2.f };
			float camRot = 0.f;
			// Define Camera matrix
			glm::mat4 view{1.f};

			//Define Projection matrix
			glm::mat4 projection;
			
			// Setup push constant
			PushConstant constants;
			float rotationVal = 0.f;

			// Mouse rotate static mesh variables
			std::array<int, 2> startRotationPos;
			glm::vec3 rotationStaticVal{0.f};
			glm::mat4 rotationStaticMat(1.f);

			// Render loop
			while (window.ProcessInput())
			{
				//! RENDER LOOP
				// Start render
				if (!window.BeginRender(pipeline)) continue; // Skip if minimized
				
				// Bind Commands
				Minerva::CommandBuffer cmdBuffer{ window.GetCommandBuffer() };
				cmdBuffer.BindGraphicsPipeline(pipeline);
				cmdBuffer.BindBuffer(vertexBuffer);
				cmdBuffer.BindBuffer(indexBuffer);

				// "Logic"

				if (Minerva::Input::IsPressed('Q'))
				{
					camRot += 0.5f;
				}

				if (Minerva::Input::IsPressed('E'))
				{
					camRot -= 0.5f;
				}

				if (Minerva::Input::IsPressed('A'))
				{
					camPos.x -= 0.25f;
				}

				if (Minerva::Input::IsPressed('D'))
				{
					camPos.x += 0.25f;
				}

				if (Minerva::Input::IsPressed('W'))
				{
					camPos.z += 0.25f;
				}

				if (Minerva::Input::IsPressed('S'))
				{
					camPos.z -= 0.25f;
				}

				if (Minerva::Input::IsTriggered(Minerva::Keycode::MOUSE_LEFT))
				{
					startRotationPos = Minerva::Input::GetMousePosition();
				}

				if (Minerva::Input::IsPressed(Minerva::Keycode::MOUSE_LEFT))
				{
					rotationStaticVal.x = -(Minerva::Input::GetMousePosition()[0] - startRotationPos[0]);
					rotationStaticVal.y =  -(Minerva::Input::GetMousePosition()[1] - startRotationPos[1]);

					glm::mat4 xRotMat(1.f);
					glm::mat4 yRotMat(1.f);
					xRotMat = glm::rotate(glm::mat4{1.f}, glm::radians(rotationStaticVal.x), glm::vec3{ 0.f, 1.f, 0.f });
					yRotMat = glm::rotate(glm::mat4{1.f}, glm::radians(rotationStaticVal.y), glm::vec3{ 1.f, 0.f, 0.f });

					rotationStaticMat = yRotMat * xRotMat;
				}




				// Calculate view matrix
				view = glm::rotate(glm::mat4{ 1.0f }, glm::radians(camRot), glm::vec3(0, 1, 0)) * glm::lookAt(camPos, { 0.f, 0.f, camPos.z+50.f }, { 0.f, 1.f, 0.f });

				// Calculate projection matrix
				projection = glm::perspective(45.f, (float)window.GetWidth() / (float)window.GetHeight(), -1.5f, 1.5f);

				// Render different cubes
				// Animated cube
				{
					// model rotation
					glm::mat4 model = glm::translate(glm::mat4(1.f), {-1.f, 0.f, 0.f}) * glm::rotate(glm::mat4{ 1.0f }, glm::radians(rotationVal), glm::vec3(0, 1, 0));
					rotationVal = rotationVal + 2.f > 360.f ? 0.f : rotationVal + 2.f; // Reset rotation
					// calculate final mesh matrix and update constant
					constants.MVP = projection * view * model;

					cmdBuffer.PushConstant(pipeline, Minerva::Shader::Type::VERTEX, 0, sizeof(PushConstant), &constants);
					cmdBuffer.DrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				}

				// Static cube
				{
					glm::mat4 model = glm::translate(glm::mat4(1.f), { 1.f, 0.f, 0.f }) * rotationStaticMat;
					constants.MVP = projection * view * model;
					cmdBuffer.PushConstant(pipeline, Minerva::Shader::Type::VERTEX, 0, sizeof(PushConstant), &constants);
					cmdBuffer.DrawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
				}
				
				
				// Page flip
				window.PageFlip(pipeline);
				//! END RENDER LOOP
			}
		}
		catch (const std::runtime_error& e)
		{
			LogError(e.what());
		}
}