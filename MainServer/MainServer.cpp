#include "pch.h"

#include "ThreadManager.h"
#include "SocketManager.h"
#include "Service.h"
#include "ClientSession.h"
#include "ClientProtocolManager.h"
#include "ConfigManager.h"
#include "DataManager.h"
#include "Protocol.pb.h"
#include "Room.h"
#include "DBManager.h"

constexpr uint64_t WORK_TIME = 256;

void RunWork(std::shared_ptr<ServerService>& service)
{
	while (true)
	{
		t_endTime = ::GetTickCount64() + WORK_TIME;

		service->GetIocpBase()->Dispatch(10);

		ThreadManager::PushReservedTask();

		ThreadManager::ExecuteTask();
	}
}

int main()
{
	SJ_ASSERT(g_dbManager->Connect(42, L"Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\MSSQLLocalDB;Database=UnityDBTest;Trusted_Connection=Yes;"));


	ConfigManager::LoadConfig();
	DataManager::LoadData();

	ClientProtocolManager::Init();

	auto room_1 = g_roomManager->Add(1);
	auto room_2 = g_roomManager->Add(1);

	std::shared_ptr<ServerService> service = std::make_shared<ServerService>(
		SocketAddress(L"127.0.0.1", 9797),
		std::make_shared<IOCPBase>(),
		std::make_shared<ClientSession>, 
		1010);

	SJ_ASSERT(service->Start());

	std::cout << "Listening" << std::endl;

	for (int32_t i = 0; i < 20; i++)
	{
		g_threadManager->Run([&service]()
			{
				while (true)
				{
					RunWork(service);
				}
			});
	}


	RunWork(service);

	g_threadManager->Join();
}




