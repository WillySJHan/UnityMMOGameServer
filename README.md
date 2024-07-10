# UnityMMOGameServer
C++20, IOCP, Protobuf, nlohmann(json), SQL-Server

시현 영상 : https://www.youtube.com/watch?v=qsoEZy7cWtU

구현 사항
- TaskQueue

Room의 GameLogic은 싱글 스레드로 구현 WorkerThread들은 TaskQueue에 Task를 Push하고
만약 해당 TaskQueue를 점유하고있는 WorkerThread가 없다면 일정시간 동안 해당 TaskQueue의 

Task를 처리하고 TaskQueue의 TaskQueue에 Push

- 이동 동기화

클라이언트에서 이동키를 누르면 셀단위(1칸) 먼저 이동후 서버에게 전송
서버는 주변 클라이언트에게 Braodcasting

- 원거리 공격

서버에서 총알을 움직이고 클라이언트에게 Broadcasting 만약 총알이 더 이상 앞으로 이동 불가능 한데 만약 앞을 막은 객체가 플레이어이거나 몬스터이면 피격

- 시야처리

Broadcasting 부하를 줄이기위해서 시야를 도입
Room를 일정 크기의 Area들로 나누고 VisualField는 자신 속한 Area안의 객체들만 거리를계산하여

VisualField안의 객체들과 정보 송수신

0.2초에 한번씩 VisualField를 최신화하여 despawn, spawn 객체 이동 최신화 

- 몬스터 AI, 몬스터 길찾기

유한 상태 기계와 A*를 이용하여 몬스터 AI, 몬스터 길찾기 구현

- 더미 클라이언트를 통한 부하 테스트

더미 클라이언트도 서버와 같은 IOCP를 활용하여 구현
각 Room에 몬스터 500마리와 더미클라이언트 1000개를 연결 ( 총 더미클라이언트 2000개)
