#pragma once

#ifdef OPZNET_SERVER_DLL
#define	OPZNET_SERVER_IMPL	__declspec( dllexport )
#else
#define	OPZNET_SERVER_IMPL	__declspec( dllimport )
#endif

#include <opznet/shared_structs.h>

#include <vector>

namespace opznet
{
	enum server_msg_type_t
	{
		SVMSG_NEW_CLIENT,
		SVMSG_CLIENT_GONE,
		SVMSG_PACKET_RECV,
		SVMSG_CLIENT_DELAY_TIME_UPDATED,
		SVMSG_BIND_FAILED,
		SVMSG_UNEXPECTED_ERROR
	};

	struct server_msg_t
	{
		server_msg_type_t			type;
		client_t					client_info;
		Packet						packet;
		int							delay_time;
	};

	// �������� ���� ���񽺸� �ʱ�ȭ��Ű�� ���۽�Ű�� �Լ��Դϴ�.
	OPZNET_SERVER_IMPL	bool	BeginServer(const std::wstring &server_name);

	// �������� ���� ���񽺸� �����Ű�� �Լ��Դϴ�.
	OPZNET_SERVER_IMPL	void	EndServer();

	// ���� ���񽺷κ��� �޽����� �޽��ϴ�.
	// �ƹ��� �޽����� ������ false�� ��ȯ�մϴ�.
	OPZNET_SERVER_IMPL	bool	GetServerMsg(server_msg_t *msg);
	
	// TCP ���������� ����ؼ� ��Ŷ�� �����մϴ�.
	OPZNET_SERVER_IMPL	void	SafeSend(ID client_id, const Packet &packet);

	// UDP ���������� ����ؼ� ��Ŷ�� �����մϴ�.
	// Ư�� Ŭ���̾�Ʈ���� ���� �� �ֽ��ϴ�. (�����ɽ�Ʈ)
	OPZNET_SERVER_IMPL	void	UnsafeSend(ID client_id, const Packet &packet);

	// UDP ���������� ����ؼ� ��Ŷ�� �����մϴ�.
	// ���� Ŭ���̾�Ʈ���� �Ѳ����� ���� �� �ֽ��ϴ�. (��Ƽ�ɽ�Ʈ)
	// ���� ���� ����
	// OPZNET_SERVER_IMPL	void	UnsafeSendMultiTarget(std::vector<ID> &client_id_vector, const char * data, size_t byte_size);
}