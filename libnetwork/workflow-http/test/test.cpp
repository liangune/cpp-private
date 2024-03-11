#include "../engine.h"
#include "../context.h"
#include <iostream>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include "exceptionDump.h"

workflowhttp::HandlerFunc Access()  {
    return [&](workflowhttp::Context *ctx) {
        std::cout << "access start" << std::endl;
        ctx->Next();
        std::cout << "access endl" << std::endl;
	};
}

workflowhttp::HandlerFunc Recovery() {
    return [&](workflowhttp::Context *ctx) {
        std::cout << "Recovery start" << std::endl;
        ctx->Next();
        std::cout << "Recovery endl" << std::endl;
	};
}


void GET_TEST(workflowhttp::Context *ctx) {
    std::cout << "GET_TEST start" << std::endl;

	std::cout << "method: " << ctx->Method() << std::endl;
	std::cout << "header: " << ctx->GetHeader("Authorization") << std::endl;
	std::cout << "body: " << ctx->GetBody().ToString() << std::endl;
	std::cout << "param: " << ctx->GetParam("id") << std::endl;

	//ctx->SetHeader("Content-Length", "1");
    ctx->SetStatus(200);
    ctx->Write("test");

	//std::cin.get();
}

void process(WFHttpTask *server_task)
{
	protocol::HttpRequest *req = server_task->get_req();
	protocol::HttpResponse *resp = server_task->get_resp();
	long long seq = server_task->get_task_seq();

    protocol::HttpHeaderCursor cursor(req);
	std::string name;
	std::string value;
	char buf[8192];
	int len;

	/* Set response message body. */
	resp->append_output_body_nocopy("<html>", 6);
	len = snprintf(buf, 8192, "<p>%s %s %s</p>", req->get_method(),
				   req->get_request_uri(), req->get_http_version());
	resp->append_output_body(buf, len);

	while (cursor.next(name, value))
	{
		len = snprintf(buf, 8192, "<p>%s: %s</p>", name.c_str(), value.c_str());
		resp->append_output_body(buf, len);
	}

	resp->append_output_body_nocopy("</html>", 7);

	/* Set status line if you like. */
	resp->set_http_version("HTTP/1.1");
	resp->set_status_code("200");
	resp->set_reason_phrase("OK");

	resp->add_header_pair("Content-Type", "text/html");
	resp->add_header_pair("Server", "Sogou WFHttpServer");
	if (seq == 9) /* no more than 10 requests on the same connection. */
		resp->add_header_pair("Connection", "close");

	/* print some log */
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;

	server_task->get_peer_addr((struct sockaddr *)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
	else
		strcpy(addrstr, "Unknown");

	fprintf(stderr, "Peer address: %s:%d, seq: %lld.\n",
			addrstr, port, seq);

}

#define HTTPS

int main(int argc, char *argv[]) 
{
#ifdef _MSC_VER
    CExceptionDump dump;
#endif

#if 1
#ifdef HTTP
    workflowhttp::Engine engine(9002);
    engine.Use(Access()).Use(Recovery());
    engine.GET("/gettest", &GET_TEST);
	engine.POST("/gettest", &GET_TEST);
	engine.GET("/Z*", &GET_TEST);
	engine.GET("/gettest/:id", &GET_TEST);
    
    if(engine.Start())
#endif 

#ifdef HTTPS
    workflowhttp::Engine engine;
    engine.Use(Access()).Use(Recovery());
    engine.GET("/gettest", &GET_TEST);
	engine.POST("/gettest", &GET_TEST);
	engine.GET("/Z*", &GET_TEST);
	engine.GET("/gettest/:id", &GET_TEST);
    
    if(engine.StartTLS("0.0.0.0", 9002, "server.crt", "server.key"))
#endif
	{
#ifndef _WIN32
		pause();
#else
		getchar();
#endif
		engine.Stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}
#else if 0
    WFHttpServer server(process);
	if (server.start(9002) == 0)
	{
#ifndef _WIN32
		pause();
#else
		getchar();
#endif
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}
#endif

    return 0;
}