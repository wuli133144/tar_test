/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.cpp
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/
#include "ClientConn.h"
#include "Common.h"
#include "AttachData.h"
#include "IM.Buddy.pb.h"
#include "IM.Message.pb.h"
#include "IM.Consult.pb.h"
#include "IM.Other.pb.h"
#include "IM.Group.pb.h"
#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
#include "IM.File.pb.h"
#include "util.h"
#include "unistd.h"
using namespace IM::BaseDefine;

static ConnMap_t g_route_server_conn_map;

static serv_info_t* g_route_server_list;
static uint32_t g_route_server_count;
static CRouteServConn* g_master_rs_conn = NULL;

extern map<int, string> g_consult_script_map;

void route_server_conn_timer_callback(void* callback_data, uint8_t msg, uint32_t handle, void* pParam)
{
	ConnMap_t::iterator it_old;
	CRouteServConn* pConn = NULL;
	uint64_t cur_time = get_tick_count();

	for (ConnMap_t::iterator it = g_route_server_conn_map.begin(); it != g_route_server_conn_map.end(); ) {
		it_old = it;
		it++;

		pConn = (CRouteServConn*)it_old->second;
		pConn->OnTimer(cur_time);
	}

	// reconnect RouteServer
	serv_check_reconnect<CRouteServConn>(g_route_server_list, g_route_server_count);
}

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count)
{
	g_route_server_list = server_list;
	g_route_server_count = server_count;

	serv_init<CRouteServConn>(g_route_server_list, g_route_server_count);

	netlib_register_timer(route_server_conn_timer_callback, NULL, 1000);
        printf("server_count:%d\n",server_count);
}

bool is_route_server_available()
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			return true;
		}
	}

	return false;
}

void send_to_all_route_server(CImPdu* pPdu)
{
	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen()) {
			pConn->SendPdu(pPdu);
		}
	}
}

// get the oldest route server connection
CRouteServConn* get_route_serv_conn()
{
	return g_master_rs_conn;
}

void update_master_route_serv_conn()
{
	uint64_t oldest_connect_time = (uint64_t)-1;
	CRouteServConn* pOldestConn = NULL;

	CRouteServConn* pConn = NULL;

	for (uint32_t i = 0; i < g_route_server_count; i++) {
		pConn = (CRouteServConn*)g_route_server_list[i].serv_conn;
		if (pConn && pConn->IsOpen() && (pConn->GetConnectTime() < oldest_connect_time) ){
			pOldestConn = pConn;
			oldest_connect_time = pConn->GetConnectTime();
		}
	}

	g_master_rs_conn =  pOldestConn;

	if (g_master_rs_conn) {
        IM::Server::IMRoleSet msg;
        msg.set_master(1);
        CImPdu pdu;
        pdu.SetPBMsg(&msg);
        pdu.SetServiceId(SID_OTHER);
        pdu.SetCommandId(CID_OTHER_ROLE_SET);
		g_master_rs_conn->SendPdu(&pdu);
	}
}


CRouteServConn::CRouteServConn()
{
	m_bOpen = false;
	m_serv_idx = 0;
}

CRouteServConn::~CRouteServConn()
{

}

void CRouteServConn::Connect(const char* server_ip, uint16_t server_port, uint32_t idx)
{
	INFO("Connecting to RouteServer %s:%d ", server_ip, server_port);

	m_serv_idx = idx;
	m_handle = netlib_connect(server_ip, server_port, imconn_callback, (void*)&g_route_server_conn_map);

	if (m_handle != NETLIB_INVALID_HANDLE) {
		g_route_server_conn_map.insert(make_pair(m_handle, this));
	}
}

void CRouteServConn::Close()
{
	serv_reset<CRouteServConn>(g_route_server_list, g_route_server_count, m_serv_idx);

	m_bOpen = false;
	if (m_handle != NETLIB_INVALID_HANDLE) {
		netlib_close(m_handle);
		g_route_server_conn_map.erase(m_handle);
	}

	ReleaseRef();

	if (g_master_rs_conn == this) {
		update_master_route_serv_conn();
	}
}

void CRouteServConn::OnConfirm()
{
	INFO("connect to route server success %u  %s:%d", m_serv_idx, g_route_server_list[m_serv_idx].server_ip.c_str(), g_route_server_list[m_serv_idx].server_port);
	m_bOpen = true;
	m_connect_time = get_tick_count();
	g_route_server_list[m_serv_idx].reconnect_cnt = MIN_RECONNECT_CNT / 2;

	if (g_master_rs_conn == NULL) {
		update_master_route_serv_conn();
	}
}
void CRouteServConn::OnClose()
{
	WARN("onclose from route server handle=%d ", m_handle);
	Close();
}

void CRouteServConn::OnTimer(uint64_t curr_tick)
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
		ERROR("connect to route  server %u timeout  %s:%d, curr_tick:%ld, m_last_recv_tick:%ld,  %ld.", m_serv_idx, g_route_server_list[m_serv_idx].server_ip.c_str(), g_route_server_list[m_serv_idx].server_port, curr_tick, m_last_recv_tick, curr_tick-m_last_recv_tick);
		Close();
	}
}

void CRouteServConn::PressureText(uint32_t num,uint32_t sleeptime,uint32_t TextNum)
{
    	for(int j=0;j<num;j++)
	{
    		for(int i=0;i<TextNum;i++)
    		{
        	IM::Server::IMUserStatusUpdate msg;
        	msg.set_user_status(::IM::BaseDefine::USER_STATUS_ONLINE);
        	msg.set_user_id(6);
        	msg.set_client_type((::IM::BaseDefine::ClientType)18);
        	msg.set_identity(2);
        	CImPdu pdu;
        	pdu.SetPBMsg(&msg);
        	pdu.SetServiceId(SID_OTHER);
        	pdu.SetCommandId(CID_OTHER_USER_STATUS_UPDATE);
        	send_to_all_route_server(&pdu);

                CImPdu pdu2;
                msg.set_user_status(::IM::BaseDefine::USER_STATUS_OFFLINE);
                pdu2.SetPBMsg(&msg);
                pdu2.SetServiceId(SID_OTHER);
                pdu2.SetCommandId(CID_OTHER_USER_STATUS_UPDATE);
                send_to_all_route_server(&pdu2);
    		}
             sleep(sleeptime);
	}
}

