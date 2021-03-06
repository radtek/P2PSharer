///////////////////////////////////////////////////
//文件下载类，负责：
//1.发送分片下载请求
//2.接收分片，重新组装文件
//3.根据MD5值信息自动搜索可下载节点
#pragma once
#include "acl_cpp\stdlib\thread.hpp"
#include "CommonDefine.h"
#include "ReqSender.h"
#include "FileClient.h"

#define MAX_CACHE_BLOCKS  2048
#define BLOCK_REQUEST_TIME_OUT          500    //分片下载超时时间（默认5分钟）
#define UPDATE_SERVICE_PROVIDER_TIME    60000     //更新服务下载节点的时间
class CDownloader : public acl::thread
{
public:
	CDownloader();
	~CDownloader();

	//初始化
	bool Init(T_LOCAL_FILE_INFO &fileinfo, acl::socket_stream &sock, CRedisClient *redis, HWND hNotifyWnd);

	//处理收到的分块数据,转发给FileReciver
	void Recieve(MSGDef::TMSG_FILEBLOCKDATA *data);

	//移除连接失败的发送对象(只移除但不释放内存，CReqSender本身会自删除)
	void RemoveFailConnSender(CReqSender *);

	//控制分片及请求
	void *run();

	//停止
	void Stop();

private:
	//对文件进行分片
	bool SplitFileSizeIntoBlockMap();

	//对超过5分钟未响应的分片重新压入分片列表
	void DealTimeoutBlockRequests();

	//获取一批分块 0：分片缓冲区为空 1：成功 2：无未下发分片
	int GetBlocks(std::vector<DWORD> &blockNums);

	//每隔1分钟重新搜索一次服务节点
	bool UpdateServiceProvider(void);

	//增加可用下载节点
	void AddProvider(acl::string &addr, acl::string &md5);


	DWORD m_dwLastBlockPos;                     //最近的分片位置
	std::vector<acl::string> m_vProviderMACs;       //所有可用下载节点
// 	std::vector<DWORD> m_vBlocksCache;          //当前缓存的需要下载的分片
	std::map<DWORD, DWORD> m_mapFileBlocks;     //已下发的分片 key:分片位置 value:分片下发的时间
	acl::locker m_lockFileBlocks;               //保护分片列表
	//acl::socket_stream *m_sock;
    CRedisClient *m_redis;
	DWORD m_dwProviderLastUpdateTime;

	T_LOCAL_FILE_INFO m_fileInfo;
	acl::ofstream m_fstream;
	HWND m_hWndRecieveNotify;                    //接收停止下载消息的窗口句柄
	acl::locker m_lockSenderObject;

	CFileClient *m_objReciever;
	std::vector<CReqSender *> m_vObjSender;
	bool m_bExit;
};

