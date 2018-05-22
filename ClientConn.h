/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   文件名称：ClientConn.h
 *   创 建 者：Zhang Yuanhao
 *   邮    箱：bluefoxah@gmail.com
 *   创建日期：2014年12月30日
 *   描    述：
 *
 ================================================================*/

#ifndef CLIENTCONN_H_
#define CLIENTCONN_H_

#include "imconn.h"
#include "IM.BaseDefine.pb.h"
#include "IM.Login.pb.h"
#include "IM.Other.pb.h"
#include "IM.Server.pb.h"
#include "IM.Buddy.pb.h"
#include "IM.Message.pb.h"
#include "IM.Group.pb.h"
#include "IPacketCallback.h"
#include "ServInfo.h"
#include "public_define.h"

class CRouteServConn : public CImConn
{
public:
	CRouteServConn();
	virtual ~CRouteServConn();

	bool IsOpen() { return m_bOpen; }
	uint64_t GetConnectTime() { return m_connect_time; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);

	void PressureText(uint32_t num,uint32_t sleeptime,uint32_t TextNum);

private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
	uint64_t	m_connect_time;
};

void init_route_serv_conn(serv_info_t* server_list, uint32_t server_count);
bool is_route_server_available();
void send_to_all_route_server(CImPdu* pPdu);
CRouteServConn* get_route_serv_conn();

#endif /* CLIENTCONN_H_ */
