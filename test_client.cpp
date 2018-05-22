/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：test_client.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#include <vector>
#include <iostream>
#include "pushServerCon.h"
#include "netlib.h"
#include "TokenValidator.h"
#include "Thread.h"
#include "IM.Buddy.pb.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Message.pb.h"
#include "IM.Consult.pb.h"
#include "IM.Other.pb.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "IM.File.pb.h"
#include "Common.h"
#include "ConfigFileReader.h"
#include "../base/FileMonitor.h"
using namespace std;
using namespace IM::BaseDefine;

#define MAX_LINE_LEN	1024
string g_cmd_string[10];
int g_cmd_num;
void split_cmd(char* buf)
{
	int len = strlen(buf);
	string element;

	g_cmd_num = 0;
	for (int i = 0; i < len; i++) {
		if (buf[i] == ' ' || buf[i] == '\t') {
			if (!element.empty()) {
				g_cmd_string[g_cmd_num++] = element;
				element.clear();
			}
		} else {
			element += buf[i];
		}
	}

	// put the last one
	if (!element.empty()) {
		g_cmd_string[g_cmd_num++] = element;
	}
}

void print_help()
{
	printf("Usage:\n");
    printf("login user_name user_pass\n");
    /*
	printf("connect serv_ip serv_port user_name user_pass\n");
    printf("getuserinfo\n");
    printf("send toId msg\n");
    printf("unreadcnt\n");
     */
	printf("close\n");
	printf("quit\n");
}
void exec_cmd()
{
        //std::cout<<"ok"<<std::endl;
		CImPdu pdu;
        IM::Server::IMPushToUserReq msg3;
		msg3.set_notify_type(2000);//old version 
        UserTokenInfo *user_token_tmp=msg3.add_user_token_list();
		user_token_tmp->set_user_id(12);
        user_token_tmp->set_user_type(17);
        user_token_tmp->set_token("fasdfzsfcvxfasdffasdfasfdasfasdfasdfasdfasdfdasfasdfasdfasdfasddfaserrfqwertfqwev rqewrxcrxqwerqwcreqwxrqwexrqwerqwecrweqcrqwerxsqwrqwcerdxqwxdrqwerqwdedrxqwxdrqwexra");
        user_token_tmp->set_push_count(1);
        user_token_tmp->set_push_type(1);
        user_token_tmp->set_allow_disturb(23);
        msg3.set_attach_data("jackwu is xx");
		pdu.SetPBMsg(&msg3); 
		pdu.SetServiceId(SID_OTHER);
		pdu.SetCommandId(CID_OTHER_PUSH_TO_USER_REQ);

		
        
		CPushServConn_test *pCon=get_push_serv_conn_test();
		if(pCon){
			 pCon->SendPdu(&pdu);
		}
	
}


class CmdThread : public CThread
{
public:
	void OnThreadRun()
	{
		while (true)
		{
			fprintf(stderr, "%s", PROMPT);	// print to error will not buffer the printed message

			if (fgets(m_buf, MAX_LINE_LEN - 1, stdin) == NULL)
			{
				fprintf(stderr, "fgets failed: %d\n", errno);
				continue;
			}		
				
				//std::cout<<"jackwu is "<<std::endl;
				//fprintf(stdout,"%s","jacwuis nu");
               
				exec_cmd();
                                fflush(stdin);
		}
	}
private:
	char	m_buf[MAX_LINE_LEN];
};

CmdThread g_cmd_thread;

int main(int argc, char* argv[])
{
//    play("message.wav");
	g_cmd_thread.StartThread();

	signal(SIGPIPE, SIG_IGN);
	CConfigFileReader config_file("client.conf");
	//std::cout<<config_file.GetConfigName("pushserver")<<std::endl;
	//std::cout<<config_file.GetConfigName("pushserverport")<<std::endl;
	fprintf(stdout,"pushserver ip=%s:pushserver port=%s\n",config_file.GetConfigName("pushserver1"),config_file.GetConfigName("pushserverport1"));
	uint32_t push_server_count = 0;
	serv_info_t* push_server_list = read_server_config(&config_file, "pushserver","pushserverport", push_server_count);
	
	    printf("push_server_count:%d\n",push_server_count);
		int ret = netlib_init();

	if (ret == NETLIB_ERROR)
		return ret;

    init_CPushServConn_test_serv_conn(push_server_list, push_server_count);
    writePid();
	netlib_eventloop();

	return 0;
}
