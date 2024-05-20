#include "pch.h"
#include "GlobaVariable.h"
#include "ThreadManager.h"
#include "SocketManager.h"
#include "TaskQueueManager.h"
#include "TaskScheduler.h"
#include "DBManager.h"

ThreadManager* g_threadManager = nullptr;
TaskQueueManager* g_taskQueueManager = nullptr;
TaskScheduler* g_taskScheduler = nullptr;
DBManager* g_dbManager = nullptr;

class GlobaVariable
{
public:
	GlobaVariable()
	{
		g_threadManager = new ThreadManager();
		g_taskQueueManager = new TaskQueueManager();
		g_taskScheduler = new TaskScheduler();
		g_dbManager = new DBManager();
		SocketManager::Init();
	}
	~GlobaVariable()
	{
		delete g_threadManager;
		delete g_taskQueueManager;
		delete g_taskScheduler;
		delete g_dbManager;
		SocketManager::Cleanup();
	}
} g_GlobalVariable;
