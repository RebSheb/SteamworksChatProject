#include <C:\\Users\\robxp\\Desktop\\Visual Studio Projects\\C++\\SteamWorksWrapper\\SteamWorksWrapper\\SteamWorksWrapper\\include.h>

#include <modes.h>
#include <aes.h>
#include <filters.h>

SteamWrapper SteamWrap;
void steam_begincallback();

void main()
{
	SteamWrap.SteamWrapperInit();
	SteamWrap.DebugInfo();

	steam_begincallback();
}

void steam_begincallback()
{


	CallbackMsg_t callback;
	std::string ciphertext;
	std::string plaintext;

	while (1)
	{
		while (Steam_BGetCallback(SteamWrap.m_hSteamPipe, &callback))
		{
			switch (callback.m_iCallback)
			{

				case FriendChatMsg_t::k_iCallback:
				{
					FriendChatMsg_t *pMsg = (FriendChatMsg_t*)callback.m_pubParam;
					EChatEntryType eChat;
					char szMsg[k_cchFriendChatMsgMax];
					memset(szMsg, 0, k_cchFriendChatMsgMax);

					int nLength = SteamWrap.m_gSteamFriends->GetFriendMessage(pMsg->m_ulSenderID, pMsg->m_iChatID, szMsg, sizeof(szMsg), &eChat);

					if (eChat == k_EChatEntryTypeChatMsg)
					{
						if (strstr(pMsg->m_ulSenderID.Render(), "STEAM_0:1:76597776") != nullptr)
						{
						}
					}
				}

			}
		}
		Steam_FreeLastCallback(SteamWrap.m_hSteamPipe);
	}
	Sleep(100);
}