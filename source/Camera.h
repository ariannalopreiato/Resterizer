#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}

		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float moveFactor{ 1.f };
		float aspectRatio{};

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};
		Matrix worldMatrix{};

		float nearPlane{ 0.1f };
		float farPlane{ 100.f };

		void Initialize(float _aspectRatio, float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,0.f})
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;

			aspectRatio = _aspectRatio;
		}

		void CalculateViewMatrix()
		{
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, Vector3::UnitY);
			invViewMatrix = Matrix::Inverse(viewMatrix);		
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		/*void CalculateWorldMatrix()
		{
			right = Vector3::Cross({ 0, 1, 0 }, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();
			worldMatrix = { right, up, forward, origin };
		}*/

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			moveFactor = 0.5f;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
				origin.z += moveFactor;
			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
				origin.z -= moveFactor;
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
				origin.x -= moveFactor;
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
				origin.x += moveFactor;

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			if ((mouseState & SDL_BUTTON_LMASK) != 0)
			{
				if (pKeyboardState[SDL_SCANCODE_LSHIFT] || pKeyboardState[SDL_SCANCODE_RSHIFT])
					moveFactor += 4.f;

				if ((mouseState & SDL_BUTTON_RMASK) != 0)
				{
					if (mouseY < 0)
						origin.y -= moveFactor;
					if (mouseY > 0)
						origin.y += moveFactor;
				}
				else
				{
					if (mouseY < 0)
						origin.z += moveFactor;
					if (mouseY > 0)
						origin.z -= moveFactor;

					totalYaw -= mouseX * deltaTime;
				}
			}
			else
			{
				if ((mouseState & SDL_BUTTON_RMASK) != 0)
				{
					totalPitch += mouseY * deltaTime;
					totalYaw -= mouseX * deltaTime;
				}
			}
			const Matrix rotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0.f) };
			forward = rotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
			
			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
			//CalculateWorldMatrix();
			//const Matrix rotation{ Matrix::CreateRotation(totalPitch, totalYaw, 0.f) };
			//forward = rotation.TransformVector(Vector3::UnitZ);
			//forward.Normalize();
		}
	};
}
