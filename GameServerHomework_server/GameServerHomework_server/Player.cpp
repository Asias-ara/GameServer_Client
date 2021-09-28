#include "Player.h"

CPlayer::CPlayer()
{
	m_xmf3Position = XMFLOAT3(-1.0f, 5.0f, -1.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
}

CPlayer::~CPlayer()
{
}

XMFLOAT3 Add(XMFLOAT3& xmf3Vector1, XMFLOAT3& xmf3Vector2, float fScalar)
{
	XMFLOAT3 xmf3Result;
	xmf3Result.x = xmf3Vector1.x + (xmf3Vector2.x * fScalar);
	xmf3Result.y = xmf3Vector1.y + (xmf3Vector2.y * fScalar);
	xmf3Result.z = xmf3Vector1.z + (xmf3Vector2.z * fScalar);
	return(xmf3Result);
}

/*플레이어의 위치를 변경하는 함수이다. 플레이어의 위치는 기본적으로 사용자가 플레이어를 이동하기 위한 키보드를
누를 때 변경된다. 플레이어의 이동 방향(dwDirection)에 따라 플레이어를 fDistance 만큼 이동한다.*/
void CPlayer::Move(unsigned long nDirection, float fDistance, bool bVelocity)
{
	if (nDirection)
	{
		//화살표 키 ‘↑’를 누르면 로컬 z-축 방향으로 이동(전진)한다. ‘↓’를 누르면 반대 방향으로 이동한다. 
		if (nDirection & DIR_FORWARD) m_xmf3Position = Add(m_xmf3Position, m_xmf3Look, fDistance);
		if (nDirection & DIR_BACKWARD) m_xmf3Position = Add(m_xmf3Position, m_xmf3Look, -fDistance);
		//화살표 키 ‘→’를 누르면 로컬 x-축 방향으로 이동한다. ‘←’를 누르면 반대 방향으로 이동한다. 
		if (nDirection & DIR_RIGHT) m_xmf3Position = Add(m_xmf3Position, m_xmf3Right, fDistance);
		if (nDirection & DIR_LEFT) m_xmf3Position = Add(m_xmf3Position, m_xmf3Right, -fDistance);
	}
}

void CPlayer::printPosition(){
	std::cout << "좌표 : (" << m_xmf3Position.x+1 << "," << m_xmf3Position.y-5 << "," << m_xmf3Position.z+1 << ")" << std::endl;
}

XMFLOAT3 CPlayer::sendPosition() {
	return m_xmf3Position;
}