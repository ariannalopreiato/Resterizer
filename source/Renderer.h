#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		void Render_W1_Part1();
		void Render_W1_Part2();
		void Render_W1_Part3();
		void Render_W1_Part4();
		void Render_W1_Part5();

		void Render_W2_Part1();
		void Render_W2_Part2();
		void Render_W2_Part3();

		void Render_W3_Part1();
		void Render_W3_Part2();

		void Render_W4_Part1();

		bool FrustumCulling(const Vertex_Out& vertex);

		void ConvertToRasterSpace(Vertex_Out& vertex);

		ColorRGB PixelShading(const Vertex_Out* vertex);

		void CycleTexture();

		void ToggleRotation();

		void ToggleNormalMap();

		void CycleShadingMode();

		float Remap(float depth, float min = 0.985f, float max = 1.f);

		bool SaveBufferToImage() const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		Texture* m_pTexture{};
		Texture* m_pNormal{};
		Texture* m_pDiffuse{};
		Texture* m_pGloss{};
		Texture* m_pSpecular{};

		bool m_IsShowingTexture{ true };
		bool m_IsRotating{ false };
		bool m_IsUsingNormalMap{ false };

		enum class ShadingMode
		{
			observedArea, diffuse, specular, combined
		};

		ShadingMode m_ShadingMode{ ShadingMode::observedArea };

		std::vector<Mesh> m_Meshes;

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const; 
		void VertexTransformationFunction(std::vector<Mesh>& meshes_in) const;
	};
}
