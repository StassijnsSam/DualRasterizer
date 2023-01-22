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

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{};
		float totalYaw{};
		float aspectRatio{};
		float nearPlane{0.1f};
		float farPlane{100.f};
		float movementSpeed{ 15.0f };
		const float boostedMovementSpeed{ 25.f };
		const float normalMovementSpeed{ 15.0f };
		const float rotationSpeed{ 0.4f };

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = {0.f,0.f,-10.f}, float _aspectRatio = 1920.f/1080.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			aspectRatio = _aspectRatio;

			origin = _origin;
		}

		void CalculateViewMatrix()
		{
			//ONB => invViewMatrix
			Matrix combinedRotation = Matrix::CreateRotation(totalPitch, totalYaw, 0);
			forward = combinedRotation.TransformVector(Vector3::UnitZ).Normalized();

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			invViewMatrix = Matrix(Vector4(right, 0), Vector4(up, 0), Vector4(forward, 0), Vector4(origin, 1));
			//Inverse(ONB) => ViewMatrix
			viewMatrix = Matrix::Inverse(invViewMatrix);
			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
			// Far = 1 and Near = 0 according to DirectX
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);
		}

		void Update(const Timer* pTimer)
		{
			//Camera Update Logic
			const float deltaTime = pTimer->GetElapsed();

			//Movementspeed set to normal
			movementSpeed = normalMovementSpeed;

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_LSHIFT]) {
				movementSpeed = boostedMovementSpeed;
			}
			if (pKeyboardState[SDL_SCANCODE_W]) {
				origin += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S]) {
				origin -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A]) {
				origin -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D]) {
				origin += right * movementSpeed * deltaTime;
			}
			
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if ((mouseState & SDL_BUTTON_RMASK) != 0 && (mouseState & SDL_BUTTON_LMASK) != 0) {
				origin += mouseY / 5.f * up * movementSpeed * deltaTime;
			}
			if ((mouseState & SDL_BUTTON_RMASK) != 0 && (mouseState & SDL_BUTTON_LMASK) == 0) {
				totalYaw += mouseX * rotationSpeed * deltaTime;
				totalPitch += mouseY * rotationSpeed * deltaTime;
			}
			if ((mouseState & SDL_BUTTON_LMASK) != 0 && (mouseState & SDL_BUTTON_RMASK) == 0) {
				origin += mouseY / 5.f * forward * movementSpeed * deltaTime;
				totalYaw += mouseX * rotationSpeed * deltaTime;
			}
			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix();
		}
	};
}
