#pragma once
#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#include <iostream>

struct XMFLOAT3
{
	float x;
	float y;
	float z;

	XMFLOAT3() = default;

	XMFLOAT3(const XMFLOAT3&) = default;
	XMFLOAT3& operator=(const XMFLOAT3&) = default;

	XMFLOAT3(XMFLOAT3&&) = default;
	XMFLOAT3& operator=(XMFLOAT3&&) = default;

	constexpr XMFLOAT3(float _x, float _y, float _z) noexcept : x(_x), y(_y), z(_z) {}
};

XMFLOAT3 Add(const XMFLOAT3& xmf3Vector1, const XMFLOAT3& xmf3Vector2);

XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar);

class CPlayer
{
	//플레이어의 위치 벡터, x-축(Right), y-축(Up), z-축(Look) 벡터이다. 
	XMFLOAT3 m_xmf3Position;
	XMFLOAT3 m_xmf3Right;
	XMFLOAT3 m_xmf3Up;
	XMFLOAT3 m_xmf3Look;

	//플레이어가 로컬 x-축(Right), y-축(Up), z-축(Look)으로 얼마만큼 회전했는가를 나타낸다. 
	float m_fPitch;
	float m_fYaw;
	float m_fRoll;

	//플레이어의 이동 속도를 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Velocity;
	//플레이어에 작용하는 중력을 나타내는 벡터이다. 
	XMFLOAT3 m_xmf3Gravity;
	//xz-평면에서 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다. 
	float m_fMaxVelocityXZ;
	//y-축 방향으로 (한 프레임 동안) 플레이어의 이동 속력의 최대값을 나타낸다.
	float m_fMaxVelocityY;
	//플레이어에 작용하는 마찰력을 나타낸다. 
	float m_fFriction;

public:
	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3 GetPosition() { return(m_xmf3Position); }
	XMFLOAT3 GetLookVector() { return(m_xmf3Look); }
	XMFLOAT3 GetUpVector() { return(m_xmf3Up); }
	XMFLOAT3 GetRightVector() { return(m_xmf3Right); }

	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }

	/*플레이어의 위치를 xmf3Position 위치로 설정한다. xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현
	재 플레이어의 위치에서 xmf3Position 방향으로의 벡터가 된다. 현재 플레이어의 위치에서 이 벡터 만큼 이동한다.*/

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() { return(m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }

	//플레이어를 이동하는 함수이다. 
	void Move(unsigned long nDirection, float fDistance, bool bVelocity = false);

	// 플레이어의 현재 위치를 나타내준다
	void printPosition();

	XMFLOAT3 sendPosition();
};