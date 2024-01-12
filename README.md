
클라이언트 설명 <br/>
game thread와 recv thread 각각 1개 씩 작동하며 recv thread에서 서버로 부터 오는 패킷을 수신하여 게임 데이터를 업데이트 합니다.
game thread에서는 업데이트된 게임 데이터로 인게임 액터들을 업데이트, 렌더링 합니다.
프로그램이 가동되면 로그인 창이 열리고 서버로 socket connect 연결을 합니다. 그 후 id와 pw를 작성해서 로그인 버튼을 누르면 서버로
login패킷을 보내고 성공하면 월드에 접속하는 enterroom 패킷까지 보내게 만들어져 있습니다.
