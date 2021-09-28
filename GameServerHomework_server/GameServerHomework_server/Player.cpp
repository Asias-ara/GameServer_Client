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

/*�÷��̾��� ��ġ�� �����ϴ� �Լ��̴�. �÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带
���� �� ����ȴ�. �÷��̾��� �̵� ����(dwDirection)�� ���� �÷��̾ fDistance ��ŭ �̵��Ѵ�.*/
void CPlayer::Move(unsigned long nDirection, float fDistance, bool bVelocity)
{
	if (nDirection)
	{
		//ȭ��ǥ Ű ���衯�� ������ ���� z-�� �������� �̵�(����)�Ѵ�. ���顯�� ������ �ݴ� �������� �̵��Ѵ�. 
		if (nDirection & DIR_FORWARD) m_xmf3Position = Add(m_xmf3Position, m_xmf3Look, fDistance);
		if (nDirection & DIR_BACKWARD) m_xmf3Position = Add(m_xmf3Position, m_xmf3Look, -fDistance);
		//ȭ��ǥ Ű ���桯�� ������ ���� x-�� �������� �̵��Ѵ�. ���硯�� ������ �ݴ� �������� �̵��Ѵ�. 
		if (nDirection & DIR_RIGHT) m_xmf3Position = Add(m_xmf3Position, m_xmf3Right, fDistance);
		if (nDirection & DIR_LEFT) m_xmf3Position = Add(m_xmf3Position, m_xmf3Right, -fDistance);
	}
}

void CPlayer::printPosition(){
	std::cout << "��ǥ : (" << m_xmf3Position.x+1 << "," << m_xmf3Position.y-5 << "," << m_xmf3Position.z+1 << ")" << std::endl;
}

XMFLOAT3 CPlayer::sendPosition() {
	return m_xmf3Position;
}