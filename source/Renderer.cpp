//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

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

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
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

	//RENDER LOGIC

	//Render_W1_Part1();
	//Render_W1_Part2();
	Render_W1_Part3();
	//Render_W1_Part4();
	//Render_W1_Part5();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::Render_W1_Part1()
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

	for(size_t i = 0; i < vertices_ndc.size(); ++i)
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

				if(signedArea1 < 0 && signedArea2 < 0 && signedArea3 < 0)
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
void Renderer::Render_W1_Part2()
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

void dae::Renderer::Render_W1_Part3()
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
		Vector2 v2 = { vertices[++i].position.x, vertices[i].position.y }; //left
		Vertex vertex2 = vertices[i];
		Vector2 v3 = { vertices[++i].position.x, vertices[i].position.y }; //right
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

void dae::Renderer::Render_W1_Part4()
{

}

void dae::Renderer::Render_W1_Part5()
{

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
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
