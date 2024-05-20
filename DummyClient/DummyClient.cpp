#include "pch.h"

#include "ThreadManager.h"
#include "SocketManager.h"
#include "Service.h"
#include "ServerProtocolManager.h"
#include "ServerSession.h"

int main()
{
	ServerProtocolManager::Init();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	std::shared_ptr<ClientService> service = std::make_shared<ClientService>(
		SocketAddress(L"127.0.0.1", 9797),
		std::make_shared<IOCPBase>(),
		std::make_shared<ServerSession>, 
		1000);

	SJ_ASSERT(service->Start());

	for (int32_t i = 0; i < 5; i++)
	{
		g_threadManager->Run([=]()
			{
				while (true)
				{
					service->GetIocpBase()->Dispatch();
				}
			});
	}




	service->GetIocpBase()->Dispatch();

	g_threadManager->Join();
}
