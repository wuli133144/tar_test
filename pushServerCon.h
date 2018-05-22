

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

class CPushServConn_test: public CImConn
{
public:
	CPushServConn_test();
	virtual ~CPushServConn_test();

	bool IsOpen() { return m_bOpen; }
	uint64_t GetConnectTime() { return m_connect_time; }

	void Connect(const char* server_ip, uint16_t server_port, uint32_t serv_idx);
	virtual void Close();

	virtual void OnConfirm();
	virtual void OnClose();
	virtual void OnTimer(uint64_t curr_tick);
	
	//void PressureText(uint32_t num,uint32_t sleeptime,uint32_t TextNum);
    

private:
	bool 		m_bOpen;
	uint32_t	m_serv_idx;
	uint64_t	m_connect_time;
};

void init_CPushServConn_test_serv_conn(serv_info_t* server_list, uint32_t &server_count);
bool is_CPushServConn_test_server_available();
void send_to_all_push_server(CImPdu* pPdu);
CPushServConn_test* get_push_serv_conn_test();

#endif /* CLIENTCONN_H_ */

