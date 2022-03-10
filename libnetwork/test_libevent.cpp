

#include <event2/event.h>
#include <event2/thread.h>
#include <event2/listener.h>
#include <iostream>
#include <string.h>
#ifndef _MSC_VER
#include <signal.h>
#endif


#define SPORT 9101

void listener_cb(struct evconnlistener *ev, evutil_socket_t s, struct sockaddr *addr, int socklen, void *arg)
{
	std::cout << "listener_cb \n";
}


int main(int argc, const char** argv)
{
#ifdef _WIN32
    // 初始化socket库
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#else
    // 忽略管道信号，防止发送数据给关闭的socket出现宕机
    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "test libevent \n";
    event_config *conf = event_config_new();

    // window支持iocp
#ifdef _WIN32
    event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);;
    //初始化iocp的线程
    evthread_use_windows_threads();
    //设置cpu数量
    SYSTEM_INFO sinfo;
    GetSystemInfo(&sinfo);
    event_config_set_num_cpus_hint(conf, sinfo.dwNumberOfProcessors);
#endif

    event_base *base = event_base_new_with_config(conf);
    event_config_free(conf);

	if (base != NULL) {
		std::cout << "event base create successful \n";
	} else {
		std::cout << "event base create fail \n";
        event_base_free(base);
	}

	// 监听端口
    // socket, bind, listen
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SPORT);
    evconnlistener *ev = evconnlistener_new_bind(base, 
        listener_cb,
        base,
        LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, // 地址重用，关闭复用
        10,
        (sockaddr*)&serverAddr,
        sizeof(serverAddr)
    );

    // 事件分发
    event_base_dispatch(base);
    if(ev) {
        evconnlistener_free(ev);
    }

    if(base) {
        event_base_free(base);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}