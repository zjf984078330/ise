/****************************************************************************\
*                                                                            *
*  ISE (Iris Server Engine) Project                                          *
*  http://github.com/haoxingeng/ise                                          *
*                                                                            *
*  Copyright 2013 HaoXinGeng (haoxingeng@gmail.com)                          *
*  All rights reserved.                                                      *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
\****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// ise_server_udp.h
///////////////////////////////////////////////////////////////////////////////

#ifndef _ISE_SERVER_UDP_H_
#define _ISE_SERVER_UDP_H_

#include "ise_options.h"
#include "ise_classes.h"
#include "ise_thread.h"
#include "ise_sysutils.h"
#include "ise_socket.h"
#include "ise_exceptions.h"

namespace ise
{

///////////////////////////////////////////////////////////////////////////////
// 提前声明

class ThreadTimeOutChecker;
class UdpPacket;
class UdpRequestQueue;
class UdpWorkerThread;
class UdpWorkerThreadPool;
class UdpRequestGroup;
class MainUdpServer;

///////////////////////////////////////////////////////////////////////////////
// class ThreadTimeOutChecker - 线程超时检测类
//
// 说明:
// 此类用于配合 UdpWorkerThread/CTcpWorkerThread，进行工作者线程的工作时间超时检测。
// 当工作者线程收到一个请求后，马上进入工作状态。一般而言，工作者线程为单个请求持续工作的
// 时间不宜太长，若太长则会导致服务器空闲工作者线程短缺，使得应付并发请求的能力下降。尤其
// 对于UDP服务来说情况更是如此。通常情况下，线程工作超时，很少是因为程序的流程和逻辑，而
// 是由于外部原因，比如数据库繁忙、资源死锁、网络拥堵等等。当线程工作超时后，应通知其退出，
// 若被通知退出后若干时间内仍未退出，则强行杀死。工作者线程调度中心再适时创建新的线程。

class ThreadTimeOutChecker : public AutoInvokable
{
private:
	Thread *thread_;          // 被检测的线程
	time_t startTime_;        // 开始计时时的时间戳
	bool started_;            // 是否已开始计时
	UINT timeoutSecs_;        // 超过多少秒认为超时 (为0表示不进行超时检测)
	CriticalSection lock_;

private:
	void start();
	void stop();

protected:
	virtual void invokeInitialize() { start(); }
	virtual void invokeFinalize() { stop(); }

public:
	explicit ThreadTimeOutChecker(Thread *thread);
	virtual ~ThreadTimeOutChecker() {}

	// 检测线程是否已超时，若超时则通知其退出
	bool check();

	// 设置超时时间，若为0则表示不进行超时检测
	void setTimeOutSecs(UINT value) { timeoutSecs_ = value; }
	// 返回是否已开始计时
	bool getStarted();
};

///////////////////////////////////////////////////////////////////////////////
// class UdpPacket - UDP数据包类

class UdpPacket
{
private:
	void *packetBuffer_;

public:
	UINT recvTimeStamp_;
	InetAddress peerAddr_;
	int packetSize_;

public:
	UdpPacket() :
		packetBuffer_(NULL),
		recvTimeStamp_(0),
		peerAddr_(0, 0),
		packetSize_(0)
	{}
	virtual ~UdpPacket()
		{ if (packetBuffer_) free(packetBuffer_); }

	void setPacketBuffer(void *pPakcetBuffer, int packetSize);
	inline void* getPacketBuffer() const { return packetBuffer_; }
};

///////////////////////////////////////////////////////////////////////////////
// class UdpRequestQueue - UDP请求队列类

class UdpRequestQueue
{
private:
	// 各容器优缺点:
	// deque  - 支持头尾快速增删，但增删中间元素很慢，支持下标访问。
	// vector - 支持尾部快速增删，头部和中间元素增删很慢，支持下标访问。
	// list   - 支持任何元素的快速增删，但不支持下标访问，不支持快速取当前长度(size())；
	typedef deque<UdpPacket*> PacketList;

	UdpRequestGroup *ownGroup_;    // 所属组别
	PacketList packetList_;        // 数据包列表
	int packetCount_;              // 队列中数据包的个数(为了快速访问)
	int capacity_;                 // 队列的最大容量
	int effWaitTime_;              // 数据包有效等待时间(秒)
	CriticalSection lock_;
	Semaphore semaphore_;

public:
	explicit UdpRequestQueue(UdpRequestGroup *ownGroup);
	virtual ~UdpRequestQueue() { clear(); }

	void addPacket(UdpPacket *pPacket);
	UdpPacket* extractPacket();
	void clear();
	void breakWaiting(int semCount);

	int getCount() { return packetCount_; }
};

///////////////////////////////////////////////////////////////////////////////
// class UdpWorkerThread - UDP工作者线程类
//
// 说明:
// 1. 缺省情况下，UDP工作者线程允许进行超时检测，若某些情况下需禁用超时检测，可以:
//    UdpWorkerThread::GetTimeOutChecker().SetTimeOutSecs(0);
//
// 名词解释:
// 1. 超时线程: 因某一请求进入工作状态但长久未完成的线程。
// 2. 僵死线程: 已被通知退出但长久不退出的线程。

class UdpWorkerThread : public Thread
{
private:
	UdpWorkerThreadPool *ownPool_;         // 所属线程池
	ThreadTimeOutChecker timeoutChecker_;  // 超时检测器
protected:
	virtual void execute();
	virtual void doTerminate();
	virtual void doKill();
public:
	explicit UdpWorkerThread(UdpWorkerThreadPool *threadPool);
	virtual ~UdpWorkerThread();

	// 返回超时检测器
	ThreadTimeOutChecker& getTimeoutChecker() { return timeoutChecker_; }
	// 返回该线程是否空闲状态(即在等待请求)
	bool isIdle() { return !timeoutChecker_.getStarted(); }
};

///////////////////////////////////////////////////////////////////////////////
// class UdpWorkerThreadPool - UDP工作者线程池类

class UdpWorkerThreadPool
{
public:
	enum
	{
		MAX_THREAD_TERM_SECS     = 60*3,    // 线程被通知退出后的最长寿命(秒)
		MAX_THREAD_WAIT_FOR_SECS = 2        // 线程池清空时最多等待时间(秒)
	};

private:
	UdpRequestGroup *ownGroup_;           // 所属组别
	ThreadList threadList_;               // 线程列表
private:
	void createThreads(int count);
	void terminateThreads(int count);
	void checkThreadTimeout();
	void killZombieThreads();
public:
	explicit UdpWorkerThreadPool(UdpRequestGroup *ownGroup);
	virtual ~UdpWorkerThreadPool();

	void registerThread(UdpWorkerThread *thread);
	void unregisterThread(UdpWorkerThread *thread);

	// 根据负载情况动态调整线程数量
	void AdjustThreadCount();
	// 通知所有线程退出
	void terminateAllThreads();
	// 等待所有线程退出
	void waitForAllThreads();

	// 取得当前线程数量
	int getThreadCount() { return threadList_.getCount(); }
	// 取得所属组别
	UdpRequestGroup& getRequestGroup() { return *ownGroup_; }
};

///////////////////////////////////////////////////////////////////////////////
// class UdpRequestGroup - UDP请求组别类

class UdpRequestGroup
{
private:
	MainUdpServer *ownMainUdpSvr_;         // 所属UDP服务器
	int groupIndex_;                       // 组别号(0-based)
	UdpRequestQueue requestQueue_;         // 请求队列
	UdpWorkerThreadPool threadPool_;       // 工作者线程池

public:
	UdpRequestGroup(MainUdpServer *ownMainUdpSvr, int groupIndex);
	virtual ~UdpRequestGroup() {}

	int getGroupIndex() { return groupIndex_; }
	UdpRequestQueue& getRequestQueue() { return requestQueue_; }
	UdpWorkerThreadPool& getThreadPool() { return threadPool_; }

	// 取得所属UDP服务器
	MainUdpServer& getMainUdpServer() { return *ownMainUdpSvr_; }
};

///////////////////////////////////////////////////////////////////////////////
// class MainUdpServer - UDP主服务器类

class MainUdpServer
{
private:
	UdpServer udpServer_;
	vector<UdpRequestGroup*> requestGroupList_;    // 请求组别列表
	int requestGroupCount_;                        // 请求组别总数
private:
	void initUdpServer();
	void initRequestGroupList();
	void clearRequestGroupList();
private:
	static void onRecvData(void *param, void *packetBuffer, int packetSize,
		const InetAddress& peerAddr);
public:
	explicit MainUdpServer();
	virtual ~MainUdpServer();

	void open();
	void close();

	void setLocalPort(int value) { udpServer_.setLocalPort(value); }
	void setListenerThreadCount(int value) { udpServer_.setListenerThreadCount(value); }

	// 根据负载情况动态调整工作者线程数量
	void adjustWorkerThreadCount();
	// 通知所有工作者线程退出
	void terminateAllWorkerThreads();
	// 等待所有工作者线程退出
	void waitForAllWorkerThreads();

	UdpServer& getUdpServer() { return udpServer_; }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace ise

#endif // _ISE_SERVER_UDP_H_
