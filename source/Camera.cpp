#include "pch.h"
#include "Camera.h"

void Camera::Update(const Timer* pTimer)
{
	const float deltaTime{ pTimer->GetElapsed() };

	bool hasMoved{ false };

	// Catch keyboard input
	const uint8_t* pKeyboardState{ SDL_GetKeyboardState(nullptr) };

	// Catch mouse input
	int mouseX{}, mouseY{};
	const uint32_t mouseState{ SDL_GetRelativeMouseState(&mouseX, &mouseY) };

	// Keyboard movement of the camera
	if(pKeyboardState[SDL_SCANCODE_W])
	{
		m_Origin += m_Forward * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_S])
	{
		m_Origin -= m_Forward * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_D])
	{
		m_Origin += m_Right * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_A])
	{
		m_Origin -= m_Right * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_SPACE])
	{
		m_Origin += m_Up * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_LSHIFT])
	{
		m_Origin -= m_Up * m_MovementSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_UP])
	{
		m_CameraOrientation.x += m_KeyboardRotationSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_DOWN])
	{
		m_CameraOrientation.x -= m_KeyboardRotationSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_LEFT])
	{
		m_CameraOrientation.y -= m_KeyboardRotationSpeed * deltaTime;
		hasMoved = true;
	}
	if(pKeyboardState[SDL_SCANCODE_RIGHT])
	{
		m_CameraOrientation.y += m_KeyboardRotationSpeed * deltaTime;
		hasMoved = true;
	}

	// Mouse movements / rotation of the camera
	if((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
	{
		// mouseX yaw left & right, mouse Y moves forwards & backwards
		const float upwards = -mouseY * m_MovementSpeed * deltaTime;
		m_Origin += m_Up * upwards;
		hasMoved = true;
	}
	else if(mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
	{
		// mouseX yaw left & right, mouse Y moves forwards & backwards
		const float forwards = -mouseY * deltaTime;
		const float yaw = mouseX * deltaTime;

		m_Origin += m_Forward * forwards;
		m_CameraOrientation.y += yaw;
		hasMoved = true;
	}
	else if(mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
	{
		// Look around the current origin
		const float pitch = -mouseY * m_RotationSpeed * deltaTime;
		const float yaw = mouseX * m_RotationSpeed * deltaTime;

		m_CameraOrientation.x += pitch;
		m_CameraOrientation.y += yaw;
		hasMoved = true;
	}

	// Update the camera stuff only when an input was received;
	if(hasMoved)
	{
		m_CameraOrientation.x = Clamp(m_CameraOrientation.x, -89.9f, 89.9f);
		m_CameraOrientation.y = Wrap(m_CameraOrientation.y, -180.f, 180.f);
		//m_CameraOrientation.z = Wrap(m_CameraOrientation.z, -10.0f, 10.0f);

		//const Matrix finalRotation = Matrix::CreateRotation(m_CameraOrientation * TO_RADIANS);
		const Matrix finalRotation = Matrix::CreateRotationX(m_CameraOrientation.x * TO_RADIANS) * Matrix::CreateRotationY(m_CameraOrientation.y * TO_RADIANS);
		m_Forward = finalRotation.TransformVector(Vector3::UnitZ);
		m_Forward.Normalize();

		m_Up = finalRotation.GetAxisY();
		m_Right = finalRotation.GetAxisX();

		//UpdateViewMatrix = true;
		//CalculateProjectionMatrix();
		CalculateViewMatrix();
	}

}

void Camera::CalculateViewMatrix()
{
	m_InvViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Up);
	m_ViewMatrix = m_InvViewMatrix.Inverse();
}

void Camera::CalculateProjectionMatrix()
{
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_FovRatio, m_AspectRatio, m_NearPlane, m_FarPlane);
}
