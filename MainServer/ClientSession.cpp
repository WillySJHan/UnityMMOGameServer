#include "pch.h"
#include "ClientSession.h"
#include "ClientProtocolManager.h"
#include "ClientSessionManager.h"
#include "DataManager.h"
#include "Player.h"
#include "Room.h"
#include "DBBind.h"
#include "DBConnector.h"
#include "DBManager.h"
#include "DBTransaction.h"
#include "Inventory.h"
#include "Item.h"
#include "VisualField.h"

ClientSession::~ClientSession()
{
	std::cout << "~ClientSession" << std::endl;
}

void ClientSession::OnConnected()
{
	std::cout << "Client Connected" << std::endl;
	{
		Protocol::MS_CONNECTED connectedProto;
		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(connectedProto);
		Send(sendBuffer);
	}

	auto session = std::static_pointer_cast<ClientSession>(shared_from_this());

	//g_roomManager->ReservePush(5000, [session]()
	//{
	//	session->Ping();
	//});

	g_sessionManager.Add(std::static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnDisconnected()
{
	std::cout << "ondisconnected" << std::endl;
	auto player = _player.load();
	if (player == nullptr)
		return;

	std::shared_ptr<BaseObject> baseObject = std::static_pointer_cast<BaseObject>(player);
	g_roomManager->Find(ID)->PushTask(&Room::LeaveGame, baseObject);
	g_objectManager.Remove(player->_objectInfo->object_id());
	
	if (player->_visualField->_task != nullptr)
	{
		player->_visualField->_task->_cancel = true;
		player->_visualField->_task = nullptr;
	}
	player->_visualField->_previousObjects.clear();

	g_sessionManager.Remove(std::static_pointer_cast<ClientSession>(shared_from_this()));
}

void ClientSession::OnRecvProtocol(BYTE* buffer, int32_t len)
{
	std::shared_ptr<ProtocolSession> session = GetProtocolSessionSptr();
	ProtocolHeader* header = reinterpret_cast<ProtocolHeader*>(buffer);

	ClientProtocolManager::ManageProtocol(session, buffer, len);
}

void ClientSession::OnSend(int32_t len)
{
	//std::cout << "OnSend Len = " << len << std::endl;
}

void ClientSession::ManageLogin(Protocol::UC_LOGIN& proto)
{
	std::wstring uniqueId = Utility::StrTowstr(proto.unique_id());
	std::wcout.imbue(std::locale("kor"));
	std::wcout << uniqueId << std::endl;

	if (_serverState != Protocol::SERVER_STATE_LOGIN)
		return;



	_lobbyPlayers.clear();

	{
		DBConnector* dbConn = g_dbManager->Pop();

		DBBind<1, 1> dbBind(*dbConn, L"\
		SELECT AccountDbId\
			FROM Account\
			WHERE AccountName = (?)");

		const wchar_t* accountName = uniqueId.c_str();
		dbBind.BindParam(0, accountName);

		int32_t outAccountDbId = 0;



		dbBind.BindCol(0, OUT outAccountDbId);


		SJ_ASSERT(dbBind.Execute());


		if (dbConn->Fetch())
		{
			_accountDbId = outAccountDbId;

			Protocol::MS_LOGIN loginOk;
			loginOk.set_login_ok(1);

			DBConnector* dbConn = g_dbManager->Pop();

			DBBind<1, 8> dbBind(*dbConn, L"\
			SELECT PlayerDbId, PlayerName, Level, Hp, MaxHp, Attack, Speed, TotalExp \
				FROM Player\
				WHERE AccountDbId = (?) ");

			int32_t accountDbId = _accountDbId;
			dbBind.BindParam(0, accountDbId);

			int32_t outPlayerDbId = 0;
			wchar_t outPlayerName[100];
			int32_t outLevel = 0;
			int32_t outHp = 0;
			int32_t outMaxHp = 0;
			int32_t outAttack = 0;
			float outSpeed = 0;
			int32_t outTotalExp = 0;

			dbBind.BindCol(0, OUT outPlayerDbId);
			dbBind.BindCol(1, OUT outPlayerName);
			dbBind.BindCol(2, OUT outLevel);
			dbBind.BindCol(3, OUT outHp);
			dbBind.BindCol(4, OUT outMaxHp);
			dbBind.BindCol(5, OUT outAttack);
			dbBind.BindCol(6, OUT outSpeed);
			dbBind.BindCol(7, OUT outTotalExp);

			SJ_ASSERT(dbBind.Execute());

			while(dbConn->Fetch())
			{

				auto player = loginOk.add_players();
				player->set_player_db_id(outPlayerDbId);
				player->set_name(Utility::WstrTostr(std::wstring(outPlayerName)));
				player->mutable_stat_info()->set_level(outLevel);
				player->mutable_stat_info()->set_hp(outHp);
				player->mutable_stat_info()->set_max_hp(outMaxHp);
				player->mutable_stat_info()->set_attack(outAttack);
				player->mutable_stat_info()->set_speed(outSpeed);
				player->mutable_stat_info()->set_total_exp(outTotalExp);

				_lobbyPlayers.push_back(*player);
			}

			g_dbManager->Push(dbConn);

			auto sendBuffer = ClientProtocolManager::MakeSendBuffer(loginOk);
			Send(sendBuffer);
			// 로비로 이동
			_serverState = Protocol::SERVER_STATE_LOBBY;

		}
		else
		{
			DBConnector* dbConn2 = g_dbManager->Pop();

			DBBind<1, 1> dbBind2(*dbConn2, L"\
			INSERT INTO [dbo].[Account]([AccountName]) \
			OUTPUT inserted.AccountDbId\
			VALUES(?); \
			");
			dbBind2.BindParam(0, accountName);

			int32_t outAccountDbId = 0;
			dbBind2.BindCol(0, OUT outAccountDbId);

			SJ_ASSERT(dbBind2.Execute());

			SJ_ASSERT(dbBind2.Fetch());

			g_dbManager->Push(dbConn2);

			_accountDbId = outAccountDbId;

			Protocol::MS_LOGIN loginOk;
			loginOk.set_login_ok(1);

			auto sendBuffer = ClientProtocolManager::MakeSendBuffer(loginOk);
			Send(sendBuffer);

			_serverState = Protocol::SERVER_STATE_LOBBY;
		}

		g_dbManager->Push(dbConn);
	}
}

void ClientSession::ManageCreatePlayer(Protocol::UC_CREATE_PLAYER& proto)
{
	if (_serverState != Protocol::SERVER_STATE_LOBBY)
		return;
	DBConnector* dbConn = g_dbManager->Pop();

	DBBind<1, 1> dbBind(*dbConn, L"\
	SELECT TOP 1 PlayerDbId FROM Player AS p\
	WHERE p.PlayerName = (?);\
	");
	std::wstring findName = Utility::StrTowstr(proto.name());
	dbBind.BindParam(0, findName.c_str());

	int32_t outPlayerDbId = 0;
	dbBind.BindCol(0, OUT outPlayerDbId);

	SJ_ASSERT(dbBind.Execute());

	// 이름이 겹침
	if (dbConn->Fetch())
	{
		Protocol::MS_CREATE_PLAYER createPlayerProto;

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(createPlayerProto);
		Send(sendBuffer);
	}
	else
	{
		// 1렙
		Protocol::StatInfo stat = DataManager::StatMap[1];

		DBConnector* dbConn = g_dbManager->Pop();
		DBBind<8,1> dbBind(*dbConn,L"\
		INSERT INTO Player(PlayerName, Level, Hp,MaxHp,Attack,Speed,TotalExp,AccountDbId) \
		OUTPUT inserted.PlayerDbId\
		VALUES(?, ?, ?,?,?,?,?,?);\
		");

		std::wstring playerName = Utility::StrTowstr(proto.name());
		int32_t level = stat.level();
		int32_t	hp = stat.hp();
		int32_t	maxHp = stat.max_hp();
		int32_t	attack = stat.attack();
		float	speed = stat.speed();
		int32_t	totalExp = 0;
		int32_t	accountDbId = _accountDbId;
		dbBind.BindParam(0, playerName.c_str());
		dbBind.BindParam(1, level);
		dbBind.BindParam(2, hp);
		dbBind.BindParam(3, maxHp);
		dbBind.BindParam(4, attack);
		dbBind.BindParam(5, speed);
		dbBind.BindParam(6, totalExp);
		dbBind.BindParam(7, accountDbId);

		int32_t outPlayerDbId = 0;

		dbBind.BindCol(0, OUT outPlayerDbId);

		SJ_ASSERT(dbBind.Execute()); 
		SJ_ASSERT(dbBind.Fetch()); 

		g_dbManager->Push(dbConn);

		// 메모리에 추가
		Protocol::LobbyPlayerInfo lobbyPlayer;
		lobbyPlayer.set_player_db_id(outPlayerDbId);
		lobbyPlayer.set_name(proto.name());
		lobbyPlayer.mutable_stat_info()->CopyFrom(stat);
		lobbyPlayer.mutable_stat_info()->set_total_exp(0);

		// 메모리에도 들고있다
		_lobbyPlayers.push_back(lobbyPlayer);


		// 클라에 전송
		Protocol::MS_CREATE_PLAYER newPlayer;
		newPlayer.mutable_player()->CopyFrom(lobbyPlayer);

		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(newPlayer);
		Send(sendBuffer);
	}


	g_dbManager->Push(dbConn);

}

void ClientSession::ManageEnterGame(Protocol::UC_ENTER_GAME& proto)
{
	if (_serverState != Protocol::SERVER_STATE_LOBBY)
		return;

	auto it = std::find_if(_lobbyPlayers.begin(), _lobbyPlayers.end(), [&](const Protocol::LobbyPlayerInfo& p)
		{
			return p.name() == proto.name();
		});

	if (it == _lobbyPlayers.end())
		return;

	Protocol::LobbyPlayerInfo playerInfo = *it;

	auto player = g_objectManager.Add<Player>();
	if (player == nullptr)
		return;
	_player.store(player);
	{
		player->_playerDbId = playerInfo.player_db_id();
		player->_objectInfo->set_name(playerInfo.name());
		player->_posInfo->set_state(Protocol::CreatureState::IDLE);
		player->_posInfo->set_move_dir(Protocol::MoveDir::DOWN);
		player->_posInfo->set_pos_x(0);
		player->_posInfo->set_pos_y(0);

		player->_statInfo->CopyFrom(playerInfo.stat_info());

		player->_session = std::static_pointer_cast<ClientSession>(shared_from_this());
		player->_visualField = std::make_shared<VisualField>(player);
		Protocol::MS_ITEM_LIST itemListProto;
		// 아이템 목록을 가지고옴
		{
			DBConnector* dbConn = g_dbManager->Pop();

			DBBind<1, 6> dbBind(*dbConn, L"\
			SELECT * FROM Item\
			WHERE OwnerDbId = (?);\
			");
			int32_t outItemDbId = 0;
			int32_t outTemplateId = 0;
			int32_t outCount = 0;
			int32_t outSlot = 0;
			bool outEquipped = false;
			int32_t outOwnerDbId = 0;

			dbBind.BindParam(0, player->_playerDbId);
			
			dbBind.BindCol(0,OUT outItemDbId);
			dbBind.BindCol(1,OUT outTemplateId);
			dbBind.BindCol(2,OUT outCount);
			dbBind.BindCol(3,OUT outSlot);
			dbBind.BindCol(4,OUT outEquipped);
			dbBind.BindCol(5,OUT outOwnerDbId);

			SJ_ASSERT(dbBind.Execute());

			while (dbConn->Fetch())
			{
				ItemDB itemDb(outItemDbId,outTemplateId,outCount,outSlot,outEquipped,outOwnerDbId);
				std::shared_ptr<Item> item = item->MakeItem(itemDb);
				if (item != nullptr)
				{
					auto info = itemListProto.add_items();
					info->CopyFrom(*item->_itemInfo);
					// TODO : 인벤에 저장
					player->_inven->Add(item);
				}
			}
			g_dbManager->Push(dbConn);

		}
		// 클라이언트에게 아이템 목록 전달
		auto sendBuffer = ClientProtocolManager::MakeSendBuffer(itemListProto);
		Send(sendBuffer);

	}

	_serverState = Protocol::SERVER_STATE_GAME;

	std::shared_ptr<BaseObject> baseObject = std::static_pointer_cast<BaseObject>(player);
	g_roomManager->Find(ID)->PushTask(&Room::EnterGame, baseObject, true);
	
}

void ClientSession::Ping()
{
	if (_pingPongTick > 0)
	{
		uint64_t delta = (::GetTickCount() - _pingPongTick);
		if (delta > 30 * 1000)
		{
			std::cout << "PINGPONG DISCONNECT" << std::endl;
			Disconnect(L"Disconnected PingPong");
			return;
		}
	}

	Protocol::MS_PING pingProto;
	auto sendBuffer = ClientProtocolManager::MakeSendBuffer(pingProto);

	Send(sendBuffer);

	auto session = std::static_pointer_cast<ClientSession>(shared_from_this());

	g_roomManager->ReservePush(5000, [session]()
	{
		session->Ping();
	});
	
}

void ClientSession::ManagePong()
{
	_pingPongTick = ::GetTickCount();

}
