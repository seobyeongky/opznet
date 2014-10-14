#pragma once

#define PING_PONG_TEST_NUMBER		15

#define	PING						0u
#define CLIENT_INTRO				1u
#define CLIENT_GONE					2u
#define SERVERSIDE_USER_LEVEL_MSG	3u
#define HERE_IS_SERVER				4u

#define PONG						0u
#define JOIN_AS_MEMBER				1u
#define JOIN_AS_NONMEMBER			2u
#define CLIENTSIDE_USER_LEVEL_MSG	3u
#define GOOD_BYE					4u
#define ANY_SERVER_THERE			5u

//Server To Client
extern const unsigned short CONST_PING;
extern const unsigned short CONST_CLIENT_INTRO;
extern const unsigned short CONST_CLIENT_GONE;
extern const unsigned short CONST_SERVERSIDE_USER_LEVEL_MSG;
extern const unsigned short CONST_HERE_IS_SERVER;

//Client To Server
extern const unsigned short CONST_PONG;
extern const unsigned short CONST_JOIN_AS_MEMBER;
extern const unsigned short CONST_JOIN_AS_NONMEMBER;
extern const unsigned short CONST_CLIENTSIDE_USER_LEVEL_MSG;
extern const unsigned short CONST_GOOD_BYE;
extern const unsigned short CONST_ANY_SERVER_THERE;


#define GAME_NAME_BUF_LENGTH		32
#define SERVER_NAME_BUF_LENGTH		32
#define USER_NAME_BUF_LENGTH		32