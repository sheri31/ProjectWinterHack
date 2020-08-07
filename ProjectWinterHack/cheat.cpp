#include "pch.h"
#include "utils.hpp"
#include "offsets.hpp"
#include "classes.hpp"
#include <vector>
#include <ctime>

GameManager::GameManager__Update gameManagerUpdateOrig;
UnlocksEntryUI__Init unlocksEntryUIInitOrig;

PhotonNetwork* photonNetwork;
PlayerHandler* localPlayerHandler = nullptr;
bool acDisabled = false;
bool gameStarted = false;
bool refreshedNameplates = false;

bool infiniteHealth = false;
bool infiniteWarmth = false;
bool infiniteFood = false;
bool freeCrafting = false;
bool spBoost = false;

Color32 survivorColor = { 0x6A, 0xCE, 0xFF, 0xFF };
Color32 traitorColor = { 0xFF, 0x33, 0x33, 0xFF };
std::time_t lastLoop = 0;

const std::vector<ePlayerRole> availableRoles{
	ePlayerRole::Innocent,
	ePlayerRole::ClassicSurvivor,
	ePlayerRole::Defector,
	ePlayerRole::Medic_S,
	ePlayerRole::Soldier,
	ePlayerRole::Medium,
	ePlayerRole::Padre,
	ePlayerRole::Hunter,
	ePlayerRole::Hacker_S,
	ePlayerRole::Lumberjack,
	ePlayerRole::Scientist_S,	
	ePlayerRole::ClassicTraitor,
	ePlayerRole::Medic_T,
	ePlayerRole::Hacker_T,
	ePlayerRole::Demon,
	ePlayerRole::Whisperer,
	ePlayerRole::Sleeper,
	ePlayerRole::Scientist_T,
	ePlayerRole::Yeti,
	ePlayerRole::IdentityThief
};


void ShowCheatStatus(const char* message, bool isCheatOn)
{
	char msg[0x20];
	sprintf_s(msg, message, (isCheatOn ? "On" : "Off"));
	localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage(msg);
}

void __fastcall GameManager_UpdateHook(GameManager* gameManager)
{
	gameManagerUpdateOrig(gameManager);

	if (!acDisabled)
	{
		ObscuredCheatingDetector__Dispose* DetectorDispose = Utilities::FindFunction<ObscuredCheatingDetector__Dispose>(Offsets::Methods::ObscuredCheatingDetector_Dispose);
		DetectorDispose();

		acDisabled = true;
	}

	if (GetAsyncKeyState(VK_F11) & 1)
	{
		gameManager->socialRatingManager->UpdateSocialRatingOnPlayFab(125, gameManager->playFabId);
	}

	if (gameManager->lobbyHandler->lobbyState != eQuickMatchLobbyState::LS_Loading)
	{
		if (gameStarted)
		{
			gameStarted = false;

			if (spBoost)
			{
				gameManager->SetAwardedPoints((rand() % 5000) + 2000, true, false);
			}
		}
		return;
	}

	if (!gameManager->levelManager || !gameManager->levelManager->hasSessionStarted)
		return;

	gameStarted = true;

	for (int i = 0; i < photonNetwork->networkingPeer->playerList->length; i++)
	{
		PhotonPlayer* player = photonNetwork->networkingPeer->playerList->GetItem(i);

		if (player)
		{
			PlayerHandler* playerHandler = gameManager->GetPlayerHandler(player->actorID, false);

			if (playerHandler)
			{
				if (player->isLocal)
				{
					if (infiniteHealth)
					{
						playerHandler->healthScript->currentValue.currentCryptoKey = 0;
						playerHandler->healthScript->currentValue.hiddenValue = 10000.0f;
					}

					if (infiniteWarmth)
					{
						playerHandler->warmthScript->currentValue.currentCryptoKey = 0;
						playerHandler->warmthScript->currentValue.hiddenValue = 10000.0f;
					}

					if (infiniteFood)
					{
						playerHandler->hungerScript->currentValue.currentCryptoKey = 0;
						playerHandler->hungerScript->currentValue.hiddenValue = 10000.0f;
					}

					if (freeCrafting &&
						playerHandler->hudManager->craftingUI &&
						playerHandler->hudManager->craftingUI->craftingEntryUI)
					{
						playerHandler->hudManager->craftingUI->craftingEntryUI->canCraft = true;
					}
					localPlayerHandler = playerHandler;
				}

				if (playerHandler->playerHoverUIHandler && playerHandler->playerHoverUIHandler->nameField)
				{
					if (std::time(nullptr) - lastLoop >= 5)
					{
                        Il2CppString* roleStr = playerHandler->playerRoleHandler->playerRoleData->GetPlayerRoleString();
                        Il2CppString* updatedName = Il2CppString::Concat(player->nameField, roleStr);
                        playerHandler->playerHoverUIHandler->nameField->SetText(updatedName);

                        if (!playerHandler->playerRoleHandler->playerRoleData->isTraitorRole)
                        {
                            playerHandler->playerHoverUIHandler->nameField->SetFaceColor(survivorColor);
                        }
                        else
                        {
                            playerHandler->playerHoverUIHandler->nameField->SetFaceColor(traitorColor);
                        }
                        
                        /*if (!playerHandler->isConvertedTraitor)
						{
							Il2CppString* roleStr = playerHandler->playerRoleHandler->playerRoleData->GetPlayerRoleString();
							Il2CppString* updatedName = Il2CppString::Concat(player->nameField, roleStr);
							playerHandler->playerHoverUIHandler->nameField->SetText(updatedName);

							if (!playerHandler->playerRoleHandler->playerRoleData->isTraitorRole)
							{
								playerHandler->playerHoverUIHandler->nameField->SetFaceColor(survivorColor);
							}
							else
							{
								playerHandler->playerHoverUIHandler->nameField->SetFaceColor(traitorColor);
							}
						}
						else
						{
							Il2CppString* updatedName = Il2CppString::Concat(player->nameField, Il2CppString::New(" [Converted]"));
							playerHandler->playerHoverUIHandler->nameField->SetText(updatedName);
							playerHandler->playerHoverUIHandler->nameField->SetFaceColor(traitorColor);
						}*/
						playerHandler->playerHoverUIHandler->nameField->SetAllDirty();
						refreshedNameplates = true;
					}
				}
			}
		}
	}

	if (refreshedNameplates)
	{
		lastLoop = std::time(nullptr);
		refreshedNameplates = false;
	}

	if (GetAsyncKeyState(VK_F1) & 1)
	{
		infiniteHealth = !infiniteHealth;
		ShowCheatStatus("Infinite Health: %s", infiniteHealth);
	}
	if (GetAsyncKeyState(VK_F2) & 1)
	{
		infiniteWarmth = !infiniteWarmth;
		ShowCheatStatus("Infinite Warmth: %s", infiniteWarmth);
	}
	if (GetAsyncKeyState(VK_F3) & 1)
	{
		infiniteFood = !infiniteFood;
		ShowCheatStatus("Infinite Food: %s", infiniteFood);
	}
	if (GetAsyncKeyState(VK_F4) & 1)
	{
		freeCrafting = !freeCrafting;
		ShowCheatStatus("Free Crafting: %s", freeCrafting);
	}
	if (GetAsyncKeyState(VK_F5) & 1)
	{
		if (localPlayerHandler->playerRoleHandler->playerRoleData->playerRole != ePlayerRole::IdentityThief)
		{
			std::vector<ePlayerRole>::const_iterator it = std::find(availableRoles.begin(), availableRoles.end(), localPlayerHandler->playerRoleHandler->playerRoleData->playerRole);
			ePlayerRole newRole = *(++it);

			localPlayerHandler->SwapPlayerRole(newRole);
		}
		else
		{
			localPlayerHandler->SwapPlayerRole(ePlayerRole::Innocent);
		}
		lastLoop = 0;
	}
	if (GetAsyncKeyState(VK_F12) & 1)
	{
		spBoost = !spBoost;
	}
	if (GetAsyncKeyState(VK_H) & 1)
	{
		localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage("Infinite Health: F1");
		localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage("Infinite Warmth: F2");
		localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage("Infinite Food: F3");
		localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage("Free Crafting: F4");
		localPlayerHandler->hudManager->textChatBox->DisplayLocalMessage("Swap Role: F5");
	}
}


void __fastcall UnlocksEntryUI_InitHook(__int64 a1, __int64 a2, int a3, unsigned int a4, __int64 a5, __int64 a6, UnlockProgress* unlockProgress, __int64 a8)
{
	if (unlockProgress)
	{
		for (int i = 0; i < unlockProgress->treeNodeInfo->length; i++)
		{
			UnlockProgressNodeInfo* nodeInfo = unlockProgress->treeNodeInfo->GetItem(i);

			nodeInfo->progressMade = 1;
			nodeInfo->branchStarted = true;
			nodeInfo->branchDone = true;
		}
	}
	unlocksEntryUIInitOrig(a1, a2, a3, a4, a5, a6, unlockProgress, a8);
}

void HijackGameLoop()
{
    //DebugBreak();
#if _DEBUG
	Utilities::AttachDebugConsole();
#endif
	Offsets::Initialize();

	auto gameManagerUpdatePtr = Utilities::FindFunction<GameManager::GameManager__Update>(Offsets::Methods::GameManager_Update);
    printf("gameManagerUpdatePtr %p\r\n", gameManagerUpdatePtr);
    gameManagerUpdateOrig = (GameManager::GameManager__Update)Utilities::Hook::DetourFunc64((BYTE*)gameManagerUpdatePtr, (BYTE*)GameManager_UpdateHook, 17);

	auto unlocksEntryUIInitPtr = Utilities::FindFunction<UnlocksEntryUI__Init>(Offsets::Methods::UnlocksEntryUI_Init);
    printf("unlocksEntryUIInitPtr %p\r\n", unlocksEntryUIInitPtr);
    unlocksEntryUIInitOrig = (UnlocksEntryUI__Init)Utilities::Hook::DetourFunc64((BYTE*)unlocksEntryUIInitPtr, (BYTE*)UnlocksEntryUI_InitHook, 19);

	photonNetwork = PhotonNetwork::Instance();
}