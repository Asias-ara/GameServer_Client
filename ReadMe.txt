게임 서버 프로그래밍 2번째 과제 ReadMe
(Client)
 - Network.cpp, Network.h추가
	- netInit()함수 : IP주소를 입력 받은 후 소켓을 생성(socket())하고 서버와 연결(connect())
	- sendKey()함수 : 키보드 입력이 있으면 키보드 입력에 대한 데이터를 보낸 후
			서버에서 보내주는 데이터를 받는다
	- netclose()함수 : 프로그램이 종료될 때 소켓을 닫아준다(closesocket())
 - LabProject.cpp
	- wWinMain()함수 초반에 netInit()함수를 통해 서버와 연결을 한다
	  동일하게 맨 마지막 부근에 netclose()함수를 통해 서버를 닫아준다
 - GameFramework.cpp
	- ProcessInput()함수에서 키 입력이 있다면 sendKey()함수를 호출하고 인자로 누른 키를
	  넘겨주어서 누른 키를 서버에 보내주고 서버에서 받은 값으로 플레이어를 움직임