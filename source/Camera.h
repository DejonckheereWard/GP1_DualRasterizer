#pragma once
//#include "MathHelpers.h"
#include "Math.h"
#include "Camera.h"

using namespace dae;


class Camera final
{
public:
	Camera() = default;
	Camera(const Vector3& _origin = {}, float _fovAngle = 45.0f, float _near = 0.1f, float _far = 100.0f, float _aspectRatio = 1.0f):
		m_Origin{ _origin },
		m_FovAngle{ _fovAngle },
		m_FovRatio{ tanf((_fovAngle * TO_RADIANS) / 2.f) },
		m_NearPlane{ _near },
		m_FarPlane{ _far },
		m_AspectRatio{ _aspectRatio }
	{
		CalculateProjectionMatrix();
		CalculateViewMatrix();
	}

	// Rule of 5
	~Camera() = default;
	Camera(const Camera&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(Camera&&) = delete;

	void Update(const Timer* pTimer);

	Matrix GetViewMatrix() const { return m_ViewMatrix; };
	Matrix GetInverseViewMatrix() const { return Matrix::Inverse(m_ViewMatrix); };
	Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; };

private:
	// Camera Settings
	
	Vector3 m_Origin;
	const float m_FovAngle;
	const float m_FovRatio;
	float m_AspectRatio;
	
	// Camera frustum planes
	const float m_NearPlane;
	const float m_FarPlane;

	// Camera control settings
	const float m_MovementSpeed{ 15.0f };
	const float m_RotationSpeed{ 30.0f };
	const float m_KeyboardRotationSpeed{ 120.0f };

	// Current camera orientation  {pitch, yaw, roll};
	Vector3 m_CameraOrientation{ 0.f, 0.f, 0.f };
	
	// Camera directions
	Vector3 m_Forward{ Vector3::UnitZ };
	Vector3 m_Up{ Vector3::UnitY };
	Vector3 m_Right{ Vector3::UnitX };

	// Matrices
	Matrix m_InvViewMatrix{};
	Matrix m_ViewMatrix{};
	Matrix m_ProjectionMatrix{};

	void CalculateViewMatrix();
	void CalculateProjectionMatrix();

};

