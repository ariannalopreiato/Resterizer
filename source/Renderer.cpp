//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include <array>

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[m_Width * m_Height];

	m_pTexture = Texture::LoadFromFile("./Resources/uv_grid_2.png");

	//Initialize Camera
	m_Camera.Initialize({ float(m_Width) / float(m_Height) }, 60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	delete m_pTexture;
	m_pTexture = nullptr;
	delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
	
	for (int i = 0; i < m_Width * m_Height; ++i)
		m_pDepthBufferPixels[i] = FLT_MAX;

	//RENDER LOGIC
	//Render_W1_Part1();
	//Render_W1_Part2();
	//Render_W1_Part3();
	//Render_W1_Part4();
	//Render_W1_Part5();

	//Render_W2_Part1();
	//Render_W2_Part2();
	//Render_W2_Part3();

	Render_W3_Part1();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Render_W1_Part1() //rasterization stage
{
	ColorRGB finalColor{};

	std::vector<Vector3> vertices_ndc
	{
		{0.f, 0.5f, 1.f},
		{0.5f, -0.5f, 1.f},
		{-0.5f, -0.5f, 1.f}
	};

	for (auto& v : vertices_ndc)
	{
		//convert the points to raster space
		v.x = ((v.x + 1) / 2) * m_Width;
		v.y = ((1 - v.y) / 2) * m_Height;
	}

	for (size_t i = 0; i < vertices_ndc.size(); ++i)
	{
		//define the triangle
		Vector2 v1 = { vertices_ndc[i].x, vertices_ndc[i].y }; //top
		Vector2 v2 = { vertices_ndc[++i].x, vertices_ndc[i].y }; //right
		Vector2 v3 = { vertices_ndc[++i].x, vertices_ndc[i].y }; //left

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				/*float gradient = px / static_cast<float>(m_Width);
				gradient += py / static_cast<float>(m_Width);
				gradient /= 2.0f;*/

				//pixel position
				Vector2 position{ float(px), float(py) };

				//edges
				const Vector2 v1v2{ v2 - v1 };
				const Vector2 v2v3{ v3 - v2 };
				const Vector2 v3v1{ v1 - v3 };

				//vector from vertex to pixel
				const Vector2 vertexToPixel1{ position - v1 };
				const Vector2 vertexToPixel2{ position - v2 };
				const Vector2 vertexToPixel3{ position - v3 };

				//cross of vertex to pixel and vertex
				auto signedArea1{ Vector2::Cross(vertexToPixel1, v1v2) };
				auto signedArea2{ Vector2::Cross(vertexToPixel2, v2v3) };
				auto signedArea3{ Vector2::Cross(vertexToPixel3, v3v1) };

				if (signedArea1 < 0 && signedArea2 < 0 && signedArea3 < 0)
					finalColor = { 1.f, 1.f, 1.f };
				else
					finalColor = { 0.f, 0.f, 0.f };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}
void Renderer::Render_W1_Part2() //projection stage
{
	ColorRGB finalColor{};

	std::vector<Vertex> vertices_ndc
	{
		{{0.f, 2.f, 0.f},{}},
		{{1.f, 0.f, 0.f}, {}},
		{{-1.f, 0.f, 0.f}, {} }
	};

	std::vector<Vertex> vertices{ vertices_ndc };

	VertexTransformationFunction(vertices_ndc, vertices);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		//define the triangle
		Vector2 v1 = { vertices[i].position.x, vertices[i].position.y }; //top
		Vector2 v2 = { vertices[++i].position.x, vertices[i].position.y }; //left
		Vector2 v3 = { vertices[++i].position.x, vertices[i].position.y }; //right

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				//pixel position
				Vector2 position{ float(px), float(py) };

				//edges
				const Vector2 v1v2{ v2 - v1 };
				const Vector2 v2v3{ v3 - v2 };
				const Vector2 v3v1{ v1 - v3 };

				//vector from vertex to pixel
				const Vector2 vertexToPixel1{ position - v1 };
				const Vector2 vertexToPixel2{ position - v2 };
				const Vector2 vertexToPixel3{ position - v3 };

				//cross of vertex to pixel and vertex
				auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
				auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
				auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

				if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
					finalColor = { 1.f, 1.f, 1.f };
				else
					finalColor = { 0.f, 0.f, 0.f };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}
void Renderer::Render_W1_Part3() //barycentric coordinates 
{
	ColorRGB finalColor{};

	std::vector<Vertex> vertices_ndc
	{
		{{0.f, 4.f, 2.f},{1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0}},
		{{-3.f, -2.f, 2.f}, {0, 0, 1} }
	};

	std::vector<Vertex> vertices{ vertices_ndc };

	VertexTransformationFunction(vertices_ndc, vertices);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		//define the triangle
		Vector2 v1 = { vertices[i].position.x, vertices[i].position.y }; //top
		Vertex vertex1 = vertices[i];
		Vector2 v2 = { vertices[++i].position.x, vertices[i].position.y }; //right
		Vertex vertex2 = vertices[i];
		Vector2 v3 = { vertices[++i].position.x, vertices[i].position.y }; //left
		Vertex vertex3 = vertices[i];

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				//pixel position
				Vector2 position{ float(px), float(py) };

				//edges
				const Vector2 v1v2{ v2 - v1 };
				const Vector2 v2v3{ v3 - v2 };
				const Vector2 v3v1{ v1 - v3 };

				//vector from vertex to pixel
				const Vector2 vertexToPixel1{ position - v1 };
				const Vector2 vertexToPixel2{ position - v2 };
				const Vector2 vertexToPixel3{ position - v3 };

				//cross of vertex to pixel and vertex
				auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
				auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
				auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

				if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
				{
					float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

					//weights
					float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
					float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
					float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

					/*Vector2 pInTriangle{ (w1 * v1) + (w2 * v2) + (w3 * v3) };
					float totalWeight{ w1 + w2 + w3 };*/

					finalColor = { vertex1.color.r * w1, vertex2.color.g * w2, vertex3.color.b * w3 };
				}
				else
					finalColor = { 0.f, 0.f, 0.f };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}
void Renderer::Render_W1_Part4() //depth buffer
{
	ColorRGB finalColor{};

	std::vector<Vertex> vertices_ndc
	{
		//triangle 1
		{{0.f, 2.f, 0.f}, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0}},
		{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

		//triangle 2
		{{0.f, 4.f, 2.f},{1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0}},
		{{-3.f, -2.f, 2.f}, {0, 0, 1} }
	};

	//convert all the vertices
	std::vector<Vertex> vertices{ vertices_ndc };
	VertexTransformationFunction(vertices_ndc, vertices);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		//define the triangle
		Vector2 v1 = { vertices[i].position.x, vertices[i].position.y }; //top
		Vertex vertex1 = vertices[i];

		Vector2 v2 = { vertices[++i].position.x, vertices[i].position.y }; //right
		Vertex vertex2 = vertices[i];

		Vector2 v3 = { vertices[++i].position.x, vertices[i].position.y }; //left
		Vertex vertex3 = vertices[i];

		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				//pixel position
				Vector2 position{ float(px), float(py) };

				//edges
				const Vector2 v1v2{ v2 - v1 };
				const Vector2 v2v3{ v3 - v2 };
				const Vector2 v3v1{ v1 - v3 };

				//vector from vertex to pixel
				const Vector2 vertexToPixel1{ position - v1 };
				const Vector2 vertexToPixel2{ position - v2 };
				const Vector2 vertexToPixel3{ position - v3 };

				//cross of vertex to pixel and vertex
				auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
				auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
				auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

				//if pixel is in triangle
				if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
				{
					float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

					//weights
					float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
					float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
					float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

					float depth{ vertex1.position.z * w1 + vertex2.position.z * w2 + vertex3.position.z * w3 };

					int currentPixel{ px + py * m_Width };
					if (currentPixel < m_Width * m_Height)
					{
						if (depth < m_pDepthBufferPixels[currentPixel])
						{
							m_pDepthBufferPixels[currentPixel] = depth;
							finalColor = w1 * vertex1.color + w2 * vertex2.color + w3 * vertex3.color;

							//Update Color in Buffer
							m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255),
								static_cast<uint8_t>(finalColor.g * 255),
								static_cast<uint8_t>(finalColor.b * 255));
						}
					}
				}
			}
		}
	}
}
void Renderer::Render_W1_Part5() //optimization
{
	ColorRGB finalColor{};

	std::vector<Vertex> vertices_ndc
	{
		//triangle 1
		{{0.f, 2.f, 0.f}, {1, 0, 0}},
		{{1.5f, -1.f, 0.f}, {1, 0, 0}},
		{{-1.5f, -1.f, 0.f}, {1, 0, 0}},

		//triangle 2
		{{0.f, 4.f, 2.f},{1, 0, 0}},
		{{3.f, -2.f, 2.f}, {0, 1, 0}},
		{{-3.f, -2.f, 2.f}, {0, 0, 1} }
	};

	//convert all the vertices
	std::vector<Vertex> vertices{ vertices_ndc };
	VertexTransformationFunction(vertices_ndc, vertices);

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		//define the triangle
		Vector2 v1 = { vertices[i].position.x, vertices[i].position.y }; //top
		Vertex vertex1 = vertices[i];

		Vector2 v2 = { vertices[++i].position.x, vertices[i].position.y }; //right
		Vertex vertex2 = vertices[i];

		Vector2 v3 = { vertices[++i].position.x, vertices[i].position.y }; //left
		Vertex vertex3 = vertices[i];

		Vector2 topLeft{ vertex1.position.x, vertex1.position.y };
		Vector2 bottomRight{ vertex1.position.x, vertex1.position.y };
		auto vertices = { vertex2.position, vertex3.position };
		for (const auto& v : vertices)
		{
			topLeft.x = std::min(topLeft.x, v.x);
			topLeft.y = std::min(topLeft.y, v.y);
			bottomRight.x = std::max(bottomRight.x, v.x);
			bottomRight.y = std::max(bottomRight.y, v.y);
		}

		if (topLeft.x >= 0 && topLeft.x < m_Width - 1 && bottomRight.x >= 0 && bottomRight.x < m_Width - 1 &&
			topLeft.y >= 0 && topLeft.y < m_Height - 1 && bottomRight.y >= 0 && bottomRight.y < m_Height - 1)
		{
			for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
			{
				for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
				{
					//pixel position
					Vector2 position{ float(px), float(py) };

					//edges
					const Vector2 v1v2{ v2 - v1 };
					const Vector2 v2v3{ v3 - v2 };
					const Vector2 v3v1{ v1 - v3 };

					//vector from vertex to pixel
					const Vector2 vertexToPixel1{ position - v1 };
					const Vector2 vertexToPixel2{ position - v2 };
					const Vector2 vertexToPixel3{ position - v3 };

					//cross of vertex to pixel and vertex
					auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
					auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
					auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

					//if pixel is in triangle
					if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
					{
						float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

						//weights
						float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
						float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
						float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

						float depth{ vertex1.position.z * w1 + vertex2.position.z * w2 + vertex3.position.z * w3 };

						int currentPixel{ px + py * m_Width };
						if (currentPixel < m_Width * m_Height)
						{
							if (depth < m_pDepthBufferPixels[currentPixel])
							{
								m_pDepthBufferPixels[currentPixel] = depth;
								finalColor = w1 * vertex1.color + w2 * vertex2.color + w3 * vertex3.color;

								//Update Color in Buffer
								m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
									static_cast<uint8_t>(finalColor.r * 255),
									static_cast<uint8_t>(finalColor.g * 255),
									static_cast<uint8_t>(finalColor.b * 255));
							}
						}
					}
				}
			}
		}
	}
}

void Renderer::Render_W2_Part1() //plane
{
	ColorRGB finalColor{};

	//define mesh
	//std::vector<Mesh> meshes_world
	//{
	//	Mesh{
	//		{
	//		Vertex{{-3,3,-2}},
	//		Vertex{{0,3,-2}},
	//		Vertex{{3,3,-2}},
	//		Vertex{{-3,0,-2}},
	//		Vertex{{0,0,-2}},
	//		Vertex{{3,0,-2}},
	//		Vertex{{-3,-3,-2}},
	//		Vertex{{0,-3,-2}},
	//		Vertex{{3,-3,-2}}
	//	},
	//	{
	//		3,0,4,1,5,2,
	//		2,6,
	//		6,3,7,4,8,5
	//	},
	//	PrimitiveTopology::TriangleStrip
	//}
	//};

	//define mesh
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{
			Vertex{{-3,3,-2}},
			Vertex{{0,3,-2}},
			Vertex{{3,3,-2}},
			Vertex{{-3,0,-2}},
			Vertex{{0,0,-2}},
			Vertex{{3,0,-2}},
			Vertex{{-3,-3,-2}},
			Vertex{{0,-3,-2}},
			Vertex{{3,-3,-2}}
		},
		{
			3,0,1,   1,4,3,   4,1,2,
			2,5,4,   6,3,4,   4,7,6,
			7,4,5,   5,8,7
		},
		PrimitiveTopology::TriangleList
	}
	};

	//convert all the vertices
	VertexTransformationFunction(meshes_world);

	//for every mesh
	for (const auto& mesh : meshes_world)
	{
		//all the converted vertices
		auto vertices = mesh.vertices_out;

		for (size_t i = 0; i < mesh.indices.size() - 2; ++i)
		{
			//define the triangle

			//----------------------------------------------------------------------------------------------------
			// USING TRIANGLE STRIP
			//----------------------------------------------------------------------------------------------------
			/*int count1{};
			int count2{};
			if (i % 2 == 0)
			{
				count1 = i + 1;
				count2 = i + 2;
			}
			else
			{
				count1 = i + 2;
				count2 = i + 1;
			}

			Vector2 v1 = { vertices[mesh.indices[i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex1 = vertices[mesh.indices[i]];

			Vector2 v2 = { vertices[mesh.indices[count1]].position.x, vertices[mesh.indices[count1]].position.y };
			Vertex_Out vertex2 = vertices[mesh.indices[count1]];

			Vector2 v3 = { vertices[mesh.indices[count2]].position.x, vertices[mesh.indices[count2]].position.y };
			Vertex_Out vertex3 = vertices[mesh.indices[count2]];*/
			//----------------------------------------------------------------------------------------------------
			//---------------------------------------------------------------------------------------------------- 
			//----------------------------------------------------------------------------------------------------


			//----------------------------------------------------------------------------------------------------
			// USING TRIANGLE LIST
			//----------------------------------------------------------------------------------------------------
			Vector2 v1 = { vertices[mesh.indices[i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex1 = vertices[mesh.indices[i]];

			Vector2 v2 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex2 = vertices[mesh.indices[i]];

			Vector2 v3 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex3 = vertices[mesh.indices[i]];
			//----------------------------------------------------------------------------------------------------
			//---------------------------------------------------------------------------------------------------- 
			//----------------------------------------------------------------------------------------------------

			Vector2 topLeft{ vertex1.position.x, vertex1.position.y };
			Vector2 bottomRight{ vertex1.position.x, vertex1.position.y };
			auto vertices = { vertex2.position, vertex3.position };
			for (const auto& v : vertices)
			{
				topLeft.x = std::min(topLeft.x, v.x);
				topLeft.y = std::min(topLeft.y, v.y);
				bottomRight.x = std::max(bottomRight.x, v.x);
				bottomRight.y = std::max(bottomRight.y, v.y);
			}

			if (topLeft.x >= 0 && topLeft.x < m_Width - 1 && bottomRight.x >= 0 && bottomRight.x < m_Width - 1 &&
				topLeft.y >= 0 && topLeft.y < m_Height - 1 && bottomRight.y >= 0 && bottomRight.y < m_Height - 1)
			{
				for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
				{
					for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
					{
						//pixel position
						Vector2 position{ float(px), float(py) };

						//edges
						const Vector2 v1v2{ v2 - v1 };
						const Vector2 v2v3{ v3 - v2 };
						const Vector2 v3v1{ v1 - v3 };

						//vector from vertex to pixel
						const Vector2 vertexToPixel1{ position - v1 };
						const Vector2 vertexToPixel2{ position - v2 };
						const Vector2 vertexToPixel3{ position - v3 };

						//cross of vertex to pixel and vertex
						auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
						auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
						auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

						//if pixel is in triangle
						if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
						{
							float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

							//weights
							float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
							float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
							float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

							float depth{ vertex1.position.z * w1 + vertex2.position.z * w2 + vertex3.position.z * w3 };

							int currentPixel{ px + py * m_Width };
							if (currentPixel < m_Width * m_Height)
							{
								if (depth < m_pDepthBufferPixels[currentPixel])
								{
									m_pDepthBufferPixels[currentPixel] = depth;
									finalColor = w1 * vertex1.color + w2 * vertex2.color + w3 * vertex3.color;

									//Update Color in Buffer
									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(finalColor.r * 255),
										static_cast<uint8_t>(finalColor.g * 255),
										static_cast<uint8_t>(finalColor.b * 255));
								}
							}
						}
					}
				}
			}
		}
	}
}
void Renderer::Render_W2_Part2() //texture
{
	ColorRGB finalColor{};

	//define mesh
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{
			Vertex{{-3,3,-2}, {}, {0, 0}},
			Vertex{{0,3,-2}, {}, {0.5f, 0}},
			Vertex{{3,3,-2}, {}, {1, 0}},
			Vertex{{-3,0,-2}, {}, {0, 0.5f}},
			Vertex{{0,0,-2}, {}, {0.5f, 0.5f}},
			Vertex{{3,0,-2}, {}, {1, 0.5f}},
			Vertex{{-3,-3,-2}, {}, {0, 1}},
			Vertex{{0,-3,-2}, {}, {0.5f, 1}},
			Vertex{{3,-3,-2}, {}, {1, 1}}
		},
		{
			3,0,1,   1,4,3,   4,1,2,
			2,5,4,   6,3,4,   4,7,6,
			7,4,5,   5,8,7
		},
		PrimitiveTopology::TriangleList
	}
	};

	//convert all the vertices
	VertexTransformationFunction(meshes_world);

	//for every mesh
	for (const auto& mesh : meshes_world)
	{
		//all the converted vertices
		auto vertices = mesh.vertices_out;

		for (size_t i = 0; i < mesh.indices.size() - 2; ++i)
		{
			//define the triangle
			//----------------------------------------------------------------------------------------------------
			// USING TRIANGLE LIST
			//----------------------------------------------------------------------------------------------------
			Vector2 v1 = { vertices[mesh.indices[i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex1 = vertices[mesh.indices[i]];

			Vector2 v2 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex2 = vertices[mesh.indices[i]];

			Vector2 v3 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex3 = vertices[mesh.indices[i]];
			//----------------------------------------------------------------------------------------------------
			//---------------------------------------------------------------------------------------------------- 
			//----------------------------------------------------------------------------------------------------

			Vector2 topLeft{ vertex1.position.x, vertex1.position.y };
			Vector2 bottomRight{ vertex1.position.x, vertex1.position.y };
			auto vertices = { vertex2.position, vertex3.position };
			for (const auto& v : vertices)
			{
				topLeft.x = std::min(topLeft.x, v.x);
				topLeft.y = std::min(topLeft.y, v.y);
				bottomRight.x = std::max(bottomRight.x, v.x);
				bottomRight.y = std::max(bottomRight.y, v.y);
			}

			if (topLeft.x >= 0 && topLeft.x < m_Width - 1 && bottomRight.x >= 0 && bottomRight.x < m_Width - 1 &&
				topLeft.y >= 0 && topLeft.y < m_Height - 1 && bottomRight.y >= 0 && bottomRight.y < m_Height - 1)
			{
				for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
				{
					for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
					{
						//pixel position
						Vector2 position{ float(px), float(py) };

						//edges
						const Vector2 v1v2{ v2 - v1 };
						const Vector2 v2v3{ v3 - v2 };
						const Vector2 v3v1{ v1 - v3 };

						//vector from vertex to pixel
						const Vector2 vertexToPixel1{ position - v1 };
						const Vector2 vertexToPixel2{ position - v2 };
						const Vector2 vertexToPixel3{ position - v3 };

						//cross of vertex to pixel and vertex
						auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
						auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
						auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

						//if pixel is in triangle
						if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
						{
							float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

							//weights
							float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
							float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
							float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

							float depth{ vertex1.position.z * w1 + vertex2.position.z * w2 + vertex3.position.z * w3 };

							int currentPixel{ px + py * m_Width };
							if (currentPixel < m_Width * m_Height)
							{
								if (depth < m_pDepthBufferPixels[currentPixel])
								{
									m_pDepthBufferPixels[currentPixel] = depth;

									//finalColor = w1 * vertex1.color + w2 * vertex2.color + w3 * vertex3.color;
									auto uv = (vertex1.uv * w1 + vertex2.uv * w2 + vertex3.uv * w3);							
									finalColor = m_pTexture->Sample(uv);

									//Update Color in Buffer
									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(finalColor.r * 255),
										static_cast<uint8_t>(finalColor.g * 255),
										static_cast<uint8_t>(finalColor.b * 255));
								}
							}
						}
					}
				}
			}
		}
	}
}
void Renderer::Render_W2_Part3() //depth interpolation
{
	ColorRGB finalColor{};

	//define mesh
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{
			Vertex{{-3,3,-2}, {}, {0, 0}},
			Vertex{{0,3,-2}, {}, {0.5f, 0}},
			Vertex{{3,3,-2}, {}, {1, 0}},
			Vertex{{-3,0,-2}, {}, {0, 0.5f}},
			Vertex{{0,0,-2}, {}, {0.5f, 0.5f}},
			Vertex{{3,0,-2}, {}, {1, 0.5f}},
			Vertex{{-3,-3,-2}, {}, {0, 1}},
			Vertex{{0,-3,-2}, {}, {0.5f, 1}},
			Vertex{{3,-3,-2}, {}, {1, 1}}
		},
		{
			3,0,1,   1,4,3,   4,1,2,
			2,5,4,   6,3,4,   4,7,6,
			7,4,5,   5,8,7
		},
		PrimitiveTopology::TriangleList
	}
	};

	//convert all the vertices
	VertexTransformationFunction(meshes_world);

	//for every mesh
	for (const auto& mesh : meshes_world)
	{
		//all the converted vertices
		auto vertices = mesh.vertices_out;

		for (size_t i = 0; i < mesh.indices.size() - 2; ++i)
		{
			//define the triangle
			//----------------------------------------------------------------------------------------------------
			// USING TRIANGLE LIST
			//----------------------------------------------------------------------------------------------------
			Vector2 v1 = { vertices[mesh.indices[i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex1 = vertices[mesh.indices[i]];

			Vector2 v2 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex2 = vertices[mesh.indices[i]];

			Vector2 v3 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex3 = vertices[mesh.indices[i]];
			//----------------------------------------------------------------------------------------------------
			//---------------------------------------------------------------------------------------------------- 
			//----------------------------------------------------------------------------------------------------

			Vector2 topLeft{ vertex1.position.x, vertex1.position.y };
			Vector2 bottomRight{ vertex1.position.x, vertex1.position.y };
			auto vertices = { vertex2.position, vertex3.position };
			for (const auto& v : vertices)
			{
				topLeft.x = std::min(topLeft.x, v.x);
				topLeft.y = std::min(topLeft.y, v.y);
				bottomRight.x = std::max(bottomRight.x, v.x);
				bottomRight.y = std::max(bottomRight.y, v.y);
			}

			if (topLeft.x >= 0 && topLeft.x < m_Width - 1 && bottomRight.x >= 0 && bottomRight.x < m_Width - 1 &&
				topLeft.y >= 0 && topLeft.y < m_Height - 1 && bottomRight.y >= 0 && bottomRight.y < m_Height - 1)
			{
				for (int px{ int(topLeft.x) }; px < int(bottomRight.x); ++px)
				{
					for (int py{ int(topLeft.y) }; py < int(bottomRight.y); ++py)
					{
						//pixel position
						Vector2 position{ float(px), float(py) };

						//edges
						const Vector2 v1v2{ v2 - v1 };
						const Vector2 v2v3{ v3 - v2 };
						const Vector2 v3v1{ v1 - v3 };

						//vector from vertex to pixel
						const Vector2 vertexToPixel1{ position - v1 };
						const Vector2 vertexToPixel2{ position - v2 };
						const Vector2 vertexToPixel3{ position - v3 };

						//cross of vertex to pixel and vertex
						auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
						auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
						auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

						//if pixel is in triangle
						if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
						{
							float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

							//weights
							float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
							float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
							float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });

							float depth{ 1 / ((w1 / vertex1.position.z) + (w2 / vertex2.position.z) + (w3 / vertex3.position.z)) };
							
							int currentPixel{ px + py * m_Width };
							if (currentPixel < m_Width * m_Height)
							{
								if (depth < m_pDepthBufferPixels[currentPixel])
								{
									m_pDepthBufferPixels[currentPixel] = depth;

									auto uv = ((vertex1.uv / vertex1.position.z) * w1 + (vertex2.uv / vertex2.position.z) * w2 + (vertex3.uv / vertex3.position.z) * w3) * depth;
									finalColor = m_pTexture->Sample(uv);

									//Update Color in Buffer
									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(finalColor.r * 255),
										static_cast<uint8_t>(finalColor.g * 255),
										static_cast<uint8_t>(finalColor.b * 255));
								}
							}
						}
					}
				}
			}
		}
	}
}

void dae::Renderer::Render_W3_Part1() //depth interpolation
{
	ColorRGB finalColor{};

	//define mesh
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{
			Vertex{{-3,3,-2}, {}, {0, 0}},
			Vertex{{0,3,-2}, {}, {0.5f, 0}},
			Vertex{{3,3,-2}, {}, {1, 0}},
			Vertex{{-3,0,-2}, {}, {0, 0.5f}},
			Vertex{{0,0,-2}, {}, {0.5f, 0.5f}},
			Vertex{{3,0,-2}, {}, {1, 0.5f}},
			Vertex{{-3,-3,-2}, {}, {0, 1}},
			Vertex{{0,-3,-2}, {}, {0.5f, 1}},
			Vertex{{3,-3,-2}, {}, {1, 1}}
		},
		{
			3,0,1,   1,4,3,   4,1,2,
			2,5,4,   6,3,4,   4,7,6,
			7,4,5,   5,8,7
		},
		PrimitiveTopology::TriangleList,

		{}, //vertices out

		{ //world matrix
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		}
		}
	};

	//convert all the vertices
	//projetion stage -> vertices to NDC
	VertexTransformationFunction(meshes_world);

	//for every mesh
	for (const auto& mesh : meshes_world)
	{
		//all the converted vertices
		auto vertices = mesh.vertices_out;

		for (size_t i = 0; i < mesh.indices.size() - 2; ++i)
		{
			//define the triangle
			//----------------------------------------------------------------------------------------------------
			// USING TRIANGLE LIST
			//----------------------------------------------------------------------------------------------------
			Vector2 v1 = { vertices[mesh.indices[i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex1 = vertices[mesh.indices[i]];

			Vector2 v2 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex2 = vertices[mesh.indices[i]];

			Vector2 v3 = { vertices[mesh.indices[++i]].position.x, vertices[mesh.indices[i]].position.y };
			Vertex_Out vertex3 = vertices[mesh.indices[i]];
			//----------------------------------------------------------------------------------------------------
			//----------------------------------------------------------------------------------------------------

			//check if the vertices are inside the frustum
			/*if (!FrustumCulling(vertex1) || !FrustumCulling(vertex2) || !FrustumCulling(vertex3))
				continue;*/

				//convert the points to raster space
			ConvertToRasterSpace(vertex1);
			ConvertToRasterSpace(vertex2);
			ConvertToRasterSpace(vertex3);

			Vector2 topLeft{ };
			Vector2 bottomRight{ };

			topLeft.x = std::min(vertex3.position.x, std::min(vertex1.position.x, vertex2.position.x));
			topLeft.y = std::min(vertex3.position.y, std::min(vertex1.position.y, vertex2.position.y));
			bottomRight.x = std::max(vertex3.position.x, std::max(vertex1.position.x, vertex2.position.x));
			bottomRight.y = std::max(vertex3.position.y, std::max(vertex1.position.y, vertex2.position.y));

			topLeft.x = Clamp(topLeft.x, 0.f, m_Width - 1.f);
			topLeft.y = Clamp(topLeft.y, 0.f, m_Height - 1.f);
			bottomRight.x = Clamp(bottomRight.x, 0.f, m_Width - 1.f);
			bottomRight.y = Clamp(bottomRight.y, 0.f, m_Height - 1.f);

			for (int px{ int(topLeft.x) }; px <= int(bottomRight.x); ++px)
			{
				for (int py{ int(topLeft.y) }; py <= int(bottomRight.y); ++py)
				{
					//pixel position
					Vector2 position{ float(px), float(py) };

					//edges
					const Vector2 v1v2{ v2 - v1 };
					const Vector2 v2v3{ v3 - v2 };
					const Vector2 v3v1{ v1 - v3 };

					////vector from vertex to pixel
					const Vector2 vertexToPixel1{ position - v1 };
					const Vector2 vertexToPixel2{ position - v2 };
					const Vector2 vertexToPixel3{ position - v3 };

					//cross of vertex to pixel and vertex
					auto signedArea1{ Vector2::Cross(v1v2, vertexToPixel1) };
					auto signedArea2{ Vector2::Cross(v2v3, vertexToPixel2) };
					auto signedArea3{ Vector2::Cross(v3v1, vertexToPixel3) };

					//if pixel is in triangle
					if (signedArea1 > 0 && signedArea2 > 0 && signedArea3 > 0)
					{
						//view space
						float totalArea{ Vector2::Cross(v3v1, v1v2) / 2 };

						//weights
						float w1 = std::abs({ Vector2::Cross(v2v3, position - v2) / 2 / totalArea });
						float w2 = std::abs({ Vector2::Cross(v3v1, position - v3) / 2 / totalArea });
						float w3 = std::abs({ Vector2::Cross(v1v2, position - v1) / 2 / totalArea });


						float depth{ 1 / ((w1 / vertex1.position.z) + (w2 / vertex2.position.z) + (w3 / vertex3.position.z)) };

						int currentPixel{ px + py * m_Width };
						if (currentPixel < m_Width * m_Height)
						{
							//frustum clipping
							if (depth > 0 && depth < 1)
							{
								if (depth < m_pDepthBufferPixels[currentPixel])
								{
									m_pDepthBufferPixels[currentPixel] = depth;

									float w{ 1 / ((w1 / vertex1.position.w) + (w2 / vertex2.position.w) + (w3 / vertex3.position.w)) };
									auto uv = ((vertex1.uv / vertex1.position.w) * w1 + (vertex2.uv / vertex2.position.w) * w2 + (vertex3.uv / vertex3.position.w) * w3) * w;
									finalColor = m_pTexture->Sample(uv);

									//Update Color in Buffer
									m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
										static_cast<uint8_t>(finalColor.r * 255),
										static_cast<uint8_t>(finalColor.g * 255),
										static_cast<uint8_t>(finalColor.b * 255));
								}
							}
						}
					}
				}
			}
		}
	}
}

bool Renderer::FrustumCulling(const Vertex_Out& vertex)
{
	//check if vertices are inside the frustum [-1, 1] for x and y, [near, far] for z
	if (vertex.position.x < -1 && vertex.position.x > 1 &&
		vertex.position.y < -1 && vertex.position.y > 1 &&
		vertex.position.z < m_Camera.nearPlane && vertex.position.z > m_Camera.farPlane)
	{
		return false;
	}
	return true;
}

void dae::Renderer::ConvertToRasterSpace(Vertex_Out& vertex)
{
	vertex.position.x = ((vertex.position.x + 1) / 2) * m_Width;
	vertex.position.y = ((1 - vertex.position.y) / 2) * m_Height;
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	const float aspectRatio{ float(m_Width) / float(m_Height) };

	for (int i = 0; i < int(vertices_in.size()); ++i)
	{
		vertices_out[i].position = m_Camera.viewMatrix.TransformPoint(vertices_in[i].position);

		//from world space to view space
		vertices_out[i].position.x = vertices_out[i].position.x / vertices_out[i].position.z;
		vertices_out[i].position.y = vertices_out[i].position.y / vertices_out[i].position.z;

		//from view space to clipping space
		vertices_out[i].position.x = vertices_out[i].position.x / (aspectRatio * m_Camera.fov);
		vertices_out[i].position.y = vertices_out[i].position.y / m_Camera.fov;
	
		//convert the points to raster space
		vertices_out[i].position.x = ((vertices_out[i].position.x + 1) / 2) * m_Width;
		vertices_out[i].position.y = ((1 - vertices_out[i].position.y) / 2) * m_Height;

		vertices_out[i].position.z = vertices_in[i].position.z;
	}
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes_in) const
{
	const float aspectRatio{ float(m_Width) / float(m_Height) };
	
	//for each mesh
	for ( auto& mesh : meshes_in)
	{
		mesh.vertices_out.resize(mesh.vertices.size());
		Matrix worldViewProjMatrix = mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix;

		for (int i = 0; i < int(mesh.vertices.size()); ++i)
		{				
			//from world space to view space
			 Vector4 v = worldViewProjMatrix.TransformPoint(mesh.vertices[i].position.ToPoint4());
			 v.x /= v.w;
			 v.y /= v.w;
			 v.z /= v.w;

			 mesh.vertices_out[i].position.x = v.x; //[-1, 1]
			 mesh.vertices_out[i].position.y = v.y; //[-1, 1]
			 mesh.vertices_out[i].position.z = v.z; //[0, 1]
			 mesh.vertices_out[i].position.w = v.w;

			//pass uv coordinate
			mesh.vertices_out[i].uv = mesh.vertices[i].uv;			
		}
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}