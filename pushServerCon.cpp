

#include "pushServerCon.h"
#include "Common.h"
#include "AttachData.h"
#include "IM.Buddy.pb.h"
#include "IM.Message.pb.h"
#include "IM.Consult.pb.h"
#include "IM.Other.pb.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"
#include "IM.BaseDefine.pb.h"
#include "IM.SwitchService.pb.h"
#include "IM.File.pb.h"
#include "util.h"
#include "unistd.h"

using namespace IM::BaseDefine;

//static ConnMap_t g_route_server_conn_map;
//static serv_info_t* g_route_server_list;
//static uint32_t g_route_server_count;
//static CRouteServConn* g_master_rs_conn = NULL;



static ConnMap_t g_pushserver_server_conn_map;
static serv_info_t* g_push_server_list;
static uint32_t g_push_server_count;
static CPushServConn_test* g_master_rs_conn = NULL;



extern map<int, string> g_consult_script_map;

void push_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CPushServConn_test* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_pushserver_server_conn_map.begin(); it != g_pushserver_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CPushServConn_test*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	// reconnect RouteServer
	serv_check_reconnect<CPushServConn_test>(g_push_server_list, g_push_server_count);
}

void init_CPushServConn_test_serv_conn(serv_info_t* server_list, uint32_t&server_count)
{
	g_push_server_list = server_list;
	g_push_server_count = server_count;

	serv_init<CPushServConn_test>(g_push_server_list, g_push_server_count);
	printf("g_push_server_list[i].ip=%s\n",g_push_server_list[0].server_ip.c_str());

	netlib_register_timer(push_server_conn_timer_callback, NULL, 1000);
        printf("server_count:%d\n",server_count);
	printf("server_count=%d  \n",server_count);
}

bool is_CPushServConn_test_server_available()
{
	CPushServConn_test* pConn = NULL;

	for (uint32_t i = 0; i < g_push_server_count; i++) {
		pConn = (CPushServConn_test*)g_push_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_push_server(CImPdu* pPdu)
{
	CPushServConn_test* pConn = NULL;

	for (uint32_t i = 0; i < g_push_server_count; i++) {
		pConn = (CPushServConn_test*)g_push_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			pConn->SendPdu(pPdu);
		}
	}
}

// get the oldest route server connection
CPushServConn_test* get_push_serv_conn_test()
{
   return g_master_rs_conn;
}

void update_master_push_serv_conn()
{
	uint64_t oldest_connect_time = (uint64_t)-1;
	CPushServConn_test* pOldestConn = NULL;

	CPushServConn_test* pConn = NULL;

	for (uint32_t i = 0; i < g_push_server_count; i++) {
		pConn = (CPushServConn_test*)g_push_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time) ){
			pOldestConn = pConn;
			oldest_connect_time = pConn->GetConnectTime();
		}
	}

	g_master_rs_conn =  pOldestConn;

	if (g_master_rs_conn) {
		/*IM::Server::IMRoleSet msg;
        msg.set_master(1);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_ROLE_SET);
		g_master_rs_conn->SendPdu(&pdu);
		*/
	       /*CImPdu pdu;
			   IM::Server::IMPushToUserReq msg3;
			   msg3.set_notify_type(2000);//old version 
			   UserTokenInfo *user_token_tmp=msg3.add_user_token_list();
			   user_token_tmp->set_user_id(12);
			   user_token_tmp->set_user_type((IM::BaseDefine::ClientType)1);
			   user_token_tmp->set_token("fasdfzsfcvxfasdfa");
			   user_token_tmp->set_push_count(1);
			   user_token_tmp->set_push_type(1);
			   user_token_tmp->set_allow_disturb(23);
			   
			   msg3.set_attach_data("jackwu is xx");
			   pdu.SetPBMsg(&msg3); 
			   pdu.SetServiceId(SID_OTHER);
			   pdu.SetCommandId(CID_OTHER_PUSH_TO_USER_REQ);
		       g_master_rs_conn->SendPdu(&pdu);
			   */
	}
}


CPushServConn_test::CPushServConn_test()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CPushServConn_test::~CPushServConn_test()
{

}

void CPushServConn_test::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	INFO("Connecting to RouteServer %s:%d ", server_ip, server_port);

	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_pushserver_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_pushserver_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CPushServConn_test::Close()
{
	serv_reset<CPushServConn_test>(g_push_server_list, g_push_server_count, m_serv_idx);

	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_pushserver_server_conn_map.erase(m_handle);
	}

	ReleaseRef();

	if (g_master_rs_conn == this) {
		update_master_push_serv_conn();
	}
}

void CPushServConn_test::OnConfirm()
{
	INFO("connect to route server success %u  %s:%d", m_serv_idx, g_push_server_list[m_serv_idx].server_ip.c_str(), g_push_server_list[m_serv_idx].server_port);
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_push_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

/*	if (g_master_rs_conn == NULL) {
		//update_master_push_serv_conn();
	   for(int i=0;i<g_push_server_count;i++)
	   {
	       g_master_rs_conn=(CPushServConn_test *)g_push_server_list[i].serv_conn;
		   if(g_master_rs_conn&&g_master_rs_conn->IsOpen())
		   	{
		   	   break;
	   }
*/
	update_master_push_serv_conn();
	}

void CPushServConn_test::OnClose()
{
	WARN("onclose from route server handle=%d ", m_handle);
	Close();
}

void CPushServConn_test::OnTimer(uint64_t curr_tick)
{
	if (curr_tick > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {
        //DEBUG("Send heartbeat to route  server %u  %s:%d, curr_tick:%ld, m_last_send_tick:%ld,  %ld", m_serv_idx, g_route_server_list[m_serv_idx].server_ip.c_str(), g_route_server_list[m_serv_idx].server_port, curr_tick, m_last_send_tick, curr_tick-m_last_send_tick);
        IM::Other::IMHeartBeat msg;
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_HEARTBEAT);
		SendPdu(&pdu);
	}

	if (curr_tick > m_last_recv_tick + SERVER_TIMEOUT) {
	//	ERROR("connect to route  server %u timeout  %s:%d, curr_tick:%ld, m_last_recv_tick:%ld,  %ld.", m_serv_idx, g_route_server_list[m_serv_idx].server_ip.c_str(), g_route_server_list[m_serv_idx].server_port, curr_tick, m_last_recv_tick, curr_tick-m_last_recv_tick);
		Close();
	}
}


//
//    IM::Server::IMPushToUserReq msg3;
//
//    CImPdu pdu;
//pdu.SetPBMsg(&msg3);
// pdu.SetServiceId(SID_OTHER);
//pdu.SetCommandId(CID_OTHER_PUSH_TO_USER_REQ);

//  CPushServConn* PushConn = get_push_serv_conn();
//  if (PushConn) {
//        PushConn->SendPdu(&pdu);
//    }
//

