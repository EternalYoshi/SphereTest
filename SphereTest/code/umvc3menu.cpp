#include "umvc3menu.h"
#include <iostream>
//#include "proc.h"
//#include "gui.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <chrono>
//#include "..\utils\addr.h"
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include "umvc3utils.h"
#include "..\minhook\include\MinHook.h"
//#include "..\gui\imgui\imgui.h"
#include "sigscan.h"
#include "..\TheRenderer.h"
#include "..\Globals.h"
#include <bitset>

#define UMVC3HOOK_VERSION "0.1"

UMVC3Menu* TheMenu = new UMVC3Menu();
static int64 timer = GetTickCount64();

char* GetUMVC3HookVersion()
{
	static char buffer[512] = {};
	sprintf(buffer, "UMVC3Hook by ermaccer (%s)", UMVC3HOOK_VERSION);
	return buffer;
}

//Should speak for itself.
bool CheckTheMode()
{
	//Reads The Needed Pointer.
	sBattleSetting = *(uint64_t*)_addr(0x140d50e58);

	//Attempt to catch exceptions when closing game.
	try
	{
		if (sBattleSetting) {
			//Reads the match/menu flag.
			//GameMode = *(uint64_t*)_addr(sBattleSetting + 0x34C);
			GameMode = *(int*)(sBattleSetting + 0x34C);

			if (GameMode == 5)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	catch(int ExNUM)
	{
		return false;
	}
	return false;
}

void CheckIfInMatch()
{

	if (Player1CharNodeTree == 0 && Player2CharNodeTree == 0)
	{
		InMatch = false;
	}
	else
	{
		InMatch = true;
	}
}

//Gets the Pointers to the Tables needed for everything else.
void GetMainPointers()
{

	ThreeManager = ((sMvc3Manager * (__fastcall*)())_addr(0x140001af0))();
	if (GameMode == 5)
	{

		//Reads The Needed Pointers.
		gSound = *(uint64_t*)_addr(0x140d533e0);
		sAction = *(uint64_t*)_addr(0x140d47e68);
		sBattleSetting = *(uint64_t*)_addr(0x140d50e58);

		team1ptr = sAction + 0x350;
		team2ptr = sAction + 0x610;

		//Gets better Pointers.
		sCharacter = *(uint64_t*)_addr(0x140D44A70);
		if (sCharacter) {
			Player1CharNodeTree = *(uint64_t*)_addr(sCharacter + 0x58);
			Player2CharNodeTree = *(uint64_t*)_addr(sCharacter + 0x328);
		}

		//Gets Even More Pointers.
		if (sAction) {
			Player1TeamTable = *(uint64_t*)_addr(sAction + 0x350);
			Player2TeamTable = *(uint64_t*)_addr(sAction + 0x610);
			MatchFlag = *(int*)_addr(sAction + 0x118);
		}
		sGameConfig = *(uint64_t*)_addr(0x140D50F88);
		PointerToGameSpeedA = *(uint64_t*)_addr(0x140E177e8);

	}
}

//Gets the active characters based on the Table Pointers.
void GetCharacterIDs()
{
	if (GameMode == 5 && sBattleSetting)
	{
		P1Character1ID = *((int*)(sBattleSetting + 0x44));
		P1Character2ID = *((int*)(sBattleSetting + 0x44 + 0x58));
		P1Character3ID = *((int*)(sBattleSetting + 0x44 + 0x58 + 0x58));
		P2Character1ID = *((int*)(sBattleSetting + 0x44 + 0x58 + 0x58 + 0x58));
		P2Character2ID = *((int*)(sBattleSetting + 0x44 + 0x58 + 0x58 + 0x58 + 0x58));
		P2Character3ID = *((int*)(sBattleSetting + 0x44 + 0x58 + 0x58 + 0x58 + 0x58 + 0x58));

	}
}

//Gets the Active Player pointers.
void GetPlayerData()
{
	if (GameMode == 5)
	{
		if (Player1CharNodeTree && Player2CharNodeTree)
		{
			uint64_t TempA = *(uint64_t*)_addr(Player1CharNodeTree + 0x10);
			uint64_t TempB = *(uint64_t*)_addr(Player2CharNodeTree + 0x10);
			if (TempA && TempB) {
				P1Character1Data = *(uint64_t*)_addr(Player1CharNodeTree + 0x8);
				P1Character2Data = *(uint64_t*)_addr(TempA + 0x8);
				P1Character3Data = *(uint64_t*)_addr(P1Character2Data + 0x18);

				P2Character1Data = *(uint64_t*)_addr(Player2CharNodeTree + 0x8);
				P2Character2Data = *(uint64_t*)_addr(TempB + 0x8);
				P2Character3Data = *(uint64_t*)_addr(P2Character2Data + 0x18);

				Player1ActiveCharacter = *(uint64_t*)_addr(Player1TeamTable + 0x48);
				Player2ActiveCharacter = *(uint64_t*)_addr(Player2TeamTable + 0x48);
			}
		}
		if (P1Character1Data)
		{
			P1C1TagState = *(byte*)(P1Character1Data + 0x14);

		}
		if (P1Character2Data)
		{
			P1C2TagState = *(byte*)(P1Character2Data + 0x14);
		}
		if (P1Character3Data)
		{
			P1C3TagState = *(byte*)(P1Character3Data + 0x14);
		}
		if (P2Character1Data)
		{
			P2C1TagState = *(byte*)(P2Character1Data + 0x14);
		}
		if (P2Character2Data)
		{
			P2C2TagState = *(byte*)(P2Character2Data + 0x14);
		}
		if (P2Character3Data)
		{
			P2C3TagState = *(byte*)(P2Character3Data + 0x14);
		}



	}
}

void GetEvenMorePlayerData()
{
	//Short validation to prevent weird half frame crashes.
	if (P1Character1Data && P1Character2Data && P1Character3Data && P2Character1Data && P2Character2Data && P2Character3Data)
	{
		//Current Anmchr IDs.
		P1C1CurAnmchrID = *((int*)(P1Character1Data + 0x1310));
		P1C2CurAnmchrID = *((int*)(P1Character2Data + 0x1310));
		P1C3CurAnmchrID = *((int*)(P1Character3Data + 0x1310));
		P2C1CurAnmchrID = *((int*)(P2Character1Data + 0x1310));
		P2C2CurAnmchrID = *((int*)(P2Character2Data + 0x1310));
		P2C3CurAnmchrID = *((int*)(P2Character3Data + 0x1310));

		//Gets info from the Chunk of the Player Struct that houses current ATI info.

		P1C1cAtkCtrl.AtiID = *((int*)(P1Character1Data + 0x18A0));
		P1C1cAtkCtrl.FramesBeforeActive = *((float*)(P1Character1Data + 0x18AC));
		P1C1cAtkCtrl.RemainingActiveFrames = *((float*)(P1Character1Data + 0x18B0));
		P1C1cAtkCtrl.MaybeRecoveryFrames = *((float*)(P1Character1Data + 0x18B8));
		P1C1cAtkCtrl.ATIFrame = *((float*)(P1Character1Data + 0x18BC));
		P1C1cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P1Character1Data + 0x1898));
		//Validation.
		if(P1C1cAtkCtrl.rAtkInfoPtr && P1C1cAtkCtrl.AtiID != 0xffffffff)
		{
			P1C1cAtkCtrl.RawATIPointer = *((uint32_t*)(P1C1cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if(P1C1cAtkCtrl.RawATIPointer)
			{
				P1C1cAtkCtrl.ATIIdentifier = (char*)(_addr(P1C1cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P1C1cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P1C1cAtkCtrl.RawATIPointer + TempThing));

				P1C1cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P1C1cAtkCtrl.ATIIdentifier + 0x4)));
				P1C1cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P1C1cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}

		P1C2cAtkCtrl.AtiID = *((int*)(P1Character2Data + 0x18A0));
		P1C2cAtkCtrl.FramesBeforeActive = *((float*)(P1Character2Data + 0x18AC));
		P1C2cAtkCtrl.RemainingActiveFrames = *((float*)(P1Character2Data + 0x18B0));
		P1C2cAtkCtrl.MaybeRecoveryFrames = *((float*)(P1Character2Data + 0x18B8));
		P1C2cAtkCtrl.ATIFrame = *((float*)(P1Character2Data + 0x18BC));
		P1C2cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P1Character2Data + 0x1898));
		//Validation.
		if (P1C2cAtkCtrl.rAtkInfoPtr && P1C2cAtkCtrl.AtiID != 0xffffffff)
		{
			P1C2cAtkCtrl.RawATIPointer = *((uint32_t*)(P1C2cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if (P1C2cAtkCtrl.RawATIPointer)
			{
				P1C2cAtkCtrl.ATIIdentifier = (char*)(_addr(P1C2cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P1C2cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P1C2cAtkCtrl.RawATIPointer + TempThing));

				P1C2cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P1C2cAtkCtrl.ATIIdentifier + 0x4)));
				P1C2cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P1C2cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}

		P1C3cAtkCtrl.AtiID = *((int*)(P1Character3Data + 0x18A0));
		P1C3cAtkCtrl.FramesBeforeActive = *((float*)(P1Character3Data + 0x18AC));
		P1C3cAtkCtrl.RemainingActiveFrames = *((float*)(P1Character3Data + 0x18B0));
		P1C3cAtkCtrl.MaybeRecoveryFrames = *((float*)(P1Character3Data + 0x18B8));
		P1C3cAtkCtrl.ATIFrame = *((float*)(P1Character3Data + 0x18BC));
		P1C3cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P1Character3Data + 0x1898));
		//Validation.
		if (P1C3cAtkCtrl.rAtkInfoPtr && P1C3cAtkCtrl.AtiID != 0xffffffff)
		{
			P1C3cAtkCtrl.RawATIPointer = *((uint32_t*)(P1C3cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if (P1C3cAtkCtrl.RawATIPointer)
			{
				P1C3cAtkCtrl.ATIIdentifier = (char*)(_addr(P1C3cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P1C3cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P1C3cAtkCtrl.RawATIPointer + TempThing));

				P1C3cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P1C3cAtkCtrl.ATIIdentifier + 0x4)));
				P1C3cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P1C3cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}

		P2C1cAtkCtrl.AtiID = *((int*)(P2Character1Data + 0x18A0));
		P2C1cAtkCtrl.FramesBeforeActive = *((float*)(P2Character1Data + 0x18AC));
		P2C1cAtkCtrl.RemainingActiveFrames = *((float*)(P2Character1Data + 0x18B0));
		P2C1cAtkCtrl.MaybeRecoveryFrames = *((float*)(P2Character1Data + 0x18B8));
		P2C1cAtkCtrl.ATIFrame = *((float*)(P2Character1Data + 0x18BC));
		P2C1cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P2Character1Data + 0x1898));
		//Validation.
		if (P2C1cAtkCtrl.rAtkInfoPtr && P2C1cAtkCtrl.AtiID != 0xffffffff)
		{
			P2C1cAtkCtrl.RawATIPointer = *((uint32_t*)(P2C1cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if (P2C1cAtkCtrl.RawATIPointer)
			{
				P2C1cAtkCtrl.ATIIdentifier = (char*)(_addr(P2C1cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P2C1cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P2C1cAtkCtrl.RawATIPointer + TempThing));

				P2C1cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P2C1cAtkCtrl.ATIIdentifier + 0x4)));
				P2C1cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P2C1cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}

		P2C2cAtkCtrl.AtiID = *((int*)(P2Character2Data + 0x18A0));
		P2C2cAtkCtrl.FramesBeforeActive = *((float*)(P2Character2Data + 0x18AC));
		P2C2cAtkCtrl.RemainingActiveFrames = *((float*)(P2Character2Data + 0x18B0));
		P2C2cAtkCtrl.MaybeRecoveryFrames = *((float*)(P2Character2Data + 0x18B8));
		P2C2cAtkCtrl.ATIFrame = *((float*)(P2Character2Data + 0x18BC));
		P2C2cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P2Character2Data + 0x1898));
		//Validation.
		if (P2C2cAtkCtrl.rAtkInfoPtr && P2C2cAtkCtrl.AtiID != 0xffffffff)
		{
			P2C2cAtkCtrl.RawATIPointer = *((uint32_t*)(P2C2cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if (P2C2cAtkCtrl.RawATIPointer)
			{
				P2C2cAtkCtrl.ATIIdentifier = (char*)(_addr(P2C2cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P2C2cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P2C2cAtkCtrl.RawATIPointer + TempThing));

				P2C2cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P2C2cAtkCtrl.ATIIdentifier + 0x4)));
				P2C2cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P2C2cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}


		P2C3cAtkCtrl.AtiID = *((int*)(P2Character3Data + 0x18A0));
		P2C3cAtkCtrl.FramesBeforeActive = *((float*)(P2Character3Data + 0x18AC));
		P2C3cAtkCtrl.RemainingActiveFrames = *((float*)(P2Character3Data + 0x18B0));
		P2C3cAtkCtrl.MaybeRecoveryFrames = *((float*)(P2Character3Data + 0x18B8));
		P2C3cAtkCtrl.ATIFrame = *((float*)(P2Character3Data + 0x18BC));
		P2C3cAtkCtrl.rAtkInfoPtr = *((uint32_t*)(P2Character3Data + 0x1898));
		//Validation.
		if (P2C3cAtkCtrl.rAtkInfoPtr && P2C3cAtkCtrl.AtiID != 0xffffffff)
		{
			P2C3cAtkCtrl.RawATIPointer = *((uint32_t*)(P2C3cAtkCtrl.rAtkInfoPtr + 0x70));
			//Yet another validation check.
			if (P2C3cAtkCtrl.RawATIPointer)
			{
				P2C3cAtkCtrl.ATIIdentifier = (char*)(_addr(P2C3cAtkCtrl.RawATIPointer));
				TempThing = 16 + (8 * P2C3cAtkCtrl.AtiID) + 4;
				OffsetToATIData = *((uint32_t*)(P2C3cAtkCtrl.RawATIPointer + TempThing));

				P2C3cAtkCtrl.ATIStartup = *((int*)((OffsetToATIData + P2C3cAtkCtrl.ATIIdentifier + 0x4)));
				P2C3cAtkCtrl.ATIActiveFrames = *((int*)((OffsetToATIData + P2C3cAtkCtrl.ATIIdentifier + 0x8)));

			}
		}

	}


}

void EmptyTheChildLists()
{
	ChildHitboxes.clear();
	ChildHurtboxes.clear();
	ChildCharacterActiveATIChunks.clear();
	ChildCharacterCurAnmchrID.clear();
	P1C1ActiveChildData.clear();
	P1C2ActiveChildData.clear();
	P1C3ActiveChildData.clear();
	P2C1ActiveChildData.clear();
	P2C2ActiveChildData.clear();
	P2C3ActiveChildData.clear();
}

void EmptyShotLists()
{
	Player1ShotPointers.clear();
	Player2ShotPointers.clear();
	Player1Shots.clear();
	Player2Shots.clear();
	P1ShotHitCapsules.clear();
	P2ShotHitCapsules.clear();
	P1ShotHitSpheres.clear();
	P2ShotHitSpheres.clear();
}

//Intended to eliminate the spheres if not in Training Mode through emptying all the spheres proccessed previously.
void RemoveAllSpheres()
{
	EmptyTheChildLists();
	P1C1Hitboxes.clear();
	P1C2Hitboxes.clear();
	P1C3Hitboxes.clear();
	P2C1Hitboxes.clear();
	P2C2Hitboxes.clear();
	P2C3Hitboxes.clear();
	P1C1Hurtboxes.clear();
	P1C2Hurtboxes.clear();
	P1C3Hurtboxes.clear();
	P2C1Hurtboxes.clear();
	P2C2Hurtboxes.clear();
	P2C3Hurtboxes.clear();
	Emptytied = true;
}

ChildData GetChildCollisionData(uint64_t PlayerPtr, uint64_t ChildPtr)
{
	ChildData Child = {};
	uint64_t TempCA = 0;
	uint64_t TempCB = 0;
	uint64_t TempCC = 0;
	uint64_t TempCD = 0;
	uint64_t TempCE = 0;
	uint64_t TempCF = 0;
	uint64_t Capsule = 0;
	uint64_t v = 0;
	uint64_t t = 0;
	uint64_t tt = 0;
	uint64_t ttt = 0;
	uint64_t tttt = 0;
	uint64_t ttttt = 0;
	uint64_t tttttt = 0;
	int HitxCount = 0;
	int HurtXCount = 0;
	PlayercAtkCtrlStuff ChildATIChunk;

	TempCA = ChildPtr;
	TempCA = *(uint64_t*)_addr(TempCA + 0x4200);

	Child.ChildCharState = *(uint32_t*)(ChildPtr + 0x14FC);
	Child.ChildCharStateBinary = std::bitset<32>(Child.ChildCharState).to_string();

	Child.CurAnmchrID = *((int*)(ChildPtr + 0x1310));
	Child.SomeClassValue = *((int*)(ChildPtr + 0x1510));
	Child.WeirdFloat = *((float*)(ChildPtr + 0x22F8));
	Child.VTableAddress = *(uint64_t*)(ChildPtr);
	if (TempCA)
	{
		TempCB = *(uint64_t*)_addr(TempCA + 0x30);

		HitxCount = *((int*)(TempCA + 0x20));

		//ATI Chunks.

		Child.CurAnmchrID = (*((int*)(ChildPtr + 0x1310)));

		ChildATIChunk;
		ChildATIChunk.AtiID = *((int*)(ChildPtr + 0x18A0));
		ChildATIChunk.FramesBeforeActive = *((float*)(ChildPtr + 0x18AC));
		ChildATIChunk.RemainingActiveFrames = *((float*)(ChildPtr + 0x18B0));
		ChildATIChunk.MaybeRecoveryFrames = *((float*)(ChildPtr + 0x18B8));
		ChildATIChunk.ATIFrame = *((float*)(ChildPtr + 0x18BC));

		Child.ChildActiveATIChunk = ChildATIChunk;

		//For Hitboxes.

		for (int i = 0; i < HitxCount; i++)
		{
			TempCC = TempCB;
			TempCD = *(uint64_t*)_addr(TempCB);
			Hitbox Chits;
			Chits.ContainerPos.x = *(float*)(TempCD + 0x20);
			Chits.ContainerPos.y = *(float*)(TempCD + 0x24);
			Chits.ContainerPos.z = *(float*)(TempCD + 0x28);
			Chits.Radius = *(float*)(TempCD + 0x38);
			Chits.PointerToCapsuleData = *(uint64_t*)(TempCD + 0x10);

			//Checks if pointer is valid before going in there.
			if (Chits.PointerToCapsuleData)
			{

				//Then we use the pointer here to go to the capsule data.
				Capsule = *(uint64_t*)_addr(TempCD + 0x10);

				Chits.CapsulePrimaryPos.x = *((float*)(Capsule + 0x20));
				Chits.CapsulePrimaryPos.y = *((float*)(Capsule + 0x24));
				Chits.CapsulePrimaryPos.z = *((float*)(Capsule + 0x28));

				Chits.CapsuleSecondPos.x = *((float*)(Capsule + 0x30));
				Chits.CapsuleSecondPos.y = *((float*)(Capsule + 0x34));
				Chits.CapsuleSecondPos.z = *((float*)(Capsule + 0x38));

				Child.Child1ActiveHitSpheres.push_back(Chits);

			}
			TempCB = TempCB + 8;
		}

		//For Hurtboxes.
		t = *(uint64_t*)_addr(ChildPtr + 0x4E10);
		if (t && t < 0x140000000)
		{
			tt = *(uint64_t*)_addr(t + 0x30);
			tttt = tt;

			if (tt && tt < 0x140000000) {
				//This is for getting the hurtbox count.
				for (size_t i = 0; i < 100; i++)
				{
					ttt = *(uint64_t*)_addr(tt);
					if (ttt == 0) {
						break;
					}
					//tt++;;
					tt = tt + 8;
					HurtXCount++;
				}

				//uint64_t* HurtboxPointers = new uint64_t[HurtXCount];
				tt = tttt;


				if (HurtXCount && HurtXCount < 255)
					//Once More, but this time for the Hurtbox pointers.
				{
					for (int j = 0; j < HurtXCount; j++)
					{
						v = *(uint64_t*)_addr(tt);
						//HurtboxPointers[j] = v;
						//Stuff to get the relevant data.
						//v goes to a cHitPrimSphere node;
						if (v) {
							ttttt = *(uint64_t*)_addr(v);;
							Hurtbox HBx;

							HBx.PointerIdentifier = *(uint64_t*)_addr(v);
							HBx.PointerToMoreData = *(uint64_t*)_addr(v + 0x10);

							//For the defacto coordinates.
							HBx.DeFactoX = *(float*)(ttttt + 0x30);
							HBx.DeFactoY = *(float*)(ttttt + 0x34);
							HBx.DeFactoZ = *(float*)(ttttt + 0x38);

							if (HBx.PointerToMoreData) {

								HBx.SecondaryX = *((float*)(v + 0x20));
								HBx.SecondaryY = *((float*)(v + 0x24));
								HBx.SecondaryZ = *((float*)(v + 0x28));

								HBx.CollData.Coordinates.X = *((float*)(HBx.PointerToMoreData + 0x20));
								HBx.CollData.Coordinates.Y = *((float*)(HBx.PointerToMoreData + 0x24));
								HBx.CollData.Coordinates.Z = *((float*)(HBx.PointerToMoreData + 0x28));
								HBx.CollData.Radius = *((float*)(HBx.PointerToMoreData + 0x2C));



								Child.ChildActiveSpheres.push_back(HBx);

								tt = tt + 8;
							}
							else
							{
								//break;
							}
						}
					}
				}

			}
		}

	}

	return Child;

}

std::vector<ChildData> GetChildCharacterData(uint64_t PlayerPtr, int CharacterID, uint64_t NodeTree, std::vector<ChildData> ActiveChildData)
{
	uint64_t TempCA = 0;
	uint64_t TempCB = 0;
	uint64_t TempCC = 0;
	uint64_t TempCD = 0;
	uint64_t TempCE = 0;
	uint64_t TempCF = 0;
	uint64_t TempTreeJump = 0;
	uint64_t Capsule = 0;
	uint64_t v = 0;
	uint64_t t = 0;
	uint64_t tt = 0;
	uint64_t ttt = 0;
	uint64_t tttt = 0;
	uint64_t ttttt = 0;
	uint64_t tttttt = 0;
	uint64_t ATIChunkPtr = 0;
	uint64_t ChildPtr = 0;
	uint64_t Child2Ptr = 0;
	uint64_t Child3Ptr = 0;
	uint64_t Child4Ptr = 0;
	uint64_t Child5Ptr = 0;
	PlayercAtkCtrlStuff ChildATIChunk;
	int HitxCount = 0;
	int HurtXCount = 0;
	if (PlayerPtr)
	{
		TempCA = *(uint64_t*)_addr(NodeTree + 0x10);
		TempCA = *(uint64_t*)_addr(TempCA + 0x10);
		TempCA = *(uint64_t*)_addr(TempCA + 0x10);
		TempTreeJump = TempCA;
		if (TempCA)
		{
			TempCA = *(uint64_t*)_addr(TempCA + 0x8);
			ChildPtr = TempCA;
			if (ChildPtr)
			{
				ActiveChildData.push_back(GetChildCollisionData(PlayerPtr, ChildPtr));
				TempCA = *(uint64_t*)_addr(TempTreeJump + 0x10);

				//For the 2nd child.
				if (TempCA)
				{
					Child2Ptr = *(uint64_t*)_addr(TempCA + 0x8);
					ActiveChildData.push_back(GetChildCollisionData(PlayerPtr, Child2Ptr));
					TempCA = *(uint64_t*)_addr(TempTreeJump + 0x10);
					TempCA = *(uint64_t*)_addr(TempCA + 0x10);

					//For the 3rd child.
					if (TempCA)
					{
						Child3Ptr = *(uint64_t*)_addr(TempCA + 0x8);
						ActiveChildData.push_back(GetChildCollisionData(PlayerPtr, Child3Ptr));
						TempCA = *(uint64_t*)_addr(TempTreeJump + 0x10);
						TempCA = *(uint64_t*)_addr(TempCA + 0x10);
						TempCA = *(uint64_t*)_addr(TempCA + 0x10);

						//For the 4th child.
						if (TempCA)
						{
							Child4Ptr = *(uint64_t*)_addr(TempCA + 0x8);
							ActiveChildData.push_back(GetChildCollisionData(PlayerPtr, Child4Ptr));
							TempCA = *(uint64_t*)_addr(TempTreeJump + 0x10);
							TempCA = *(uint64_t*)_addr(TempCA + 0x10);
							TempCA = *(uint64_t*)_addr(TempCA + 0x10);
							TempCA = *(uint64_t*)_addr(TempCA + 0x10);

							//For the 5th child.
							if (TempCA)
							{
								Child5Ptr = *(uint64_t*)_addr(TempCA + 0x8);
								ActiveChildData.push_back(GetChildCollisionData(PlayerPtr, Child5Ptr));

							}
						}
					}
				}
			}
		}
	}

	return ActiveChildData;
}

void GetAnmchrIDs()
{
	if (P1Character1Data && P1Character2Data && P1Character3Data && P2Character1Data && P2Character2Data && P2Character3Data)
	{
		P1C1CurAnmchrID = *((int*)(P1Character1Data + 0x1310));
		P1C2CurAnmchrID = *((int*)(P1Character2Data + 0x1310));
		P1C3CurAnmchrID = *((int*)(P1Character3Data + 0x1310));
		P2C1CurAnmchrID = *((int*)(P2Character1Data + 0x1310));
		P2C2CurAnmchrID = *((int*)(P2Character2Data + 0x1310));
		P2C3CurAnmchrID = *((int*)(P2Character3Data + 0x1310));
	}


}

void GetChildCharacters()
{

	P1Character1Children.clear();

	if (P1Character1Data && Player1CharNodeTree)
	{
		uint64_t TempA = 0;
		uint64_t TempB = 0;
		uint64_t TempC = 0;
		uint64_t TempD = 0;
		uint64_t TempE = 0;
		uint64_t TempF = 0;
		uint64_t TempG = 0;
		uint64_t TempH = 0;
		uint64_t TempI = 0;
		uint64_t TempJ = 0;
		uint64_t TempK = 0;
		uint64_t TempL = 0;
		uint64_t TempM = 0;
		int Counter = 0;

		//For the first child.
		TempA = *(uint64_t*)_addr(Player1CharNodeTree + 0x10);
		if (TempA)
		{
			TempB = *(uint64_t*)_addr(TempA + 0x10);
			TempC = *(uint64_t*)_addr(TempB + 0x10);

			//If null the character in question has no children.
			if (TempC)
			{
				TempD = *(uint64_t*)_addr(TempC + 0x8);
				Counter++;

				TempE = *(uint64_t*)_addr(TempC + 0x10);
				if (TempE)
				{
					TempF = *(uint64_t*)_addr(TempE + 0x8);
					Counter++;

					TempG = *(uint64_t*)_addr(TempE + 0x10);
					if (TempG)
					{
						TempH = *(uint64_t*)_addr(TempG + 0x8);
						Counter++;

						TempI = *(uint64_t*)_addr(TempG + 0x10);
						if (TempI)
						{
							TempJ = *(uint64_t*)_addr(TempI + 0x8);
							Counter++;

							TempK = *(uint64_t*)_addr(TempI + 0x10);
							if (TempK)
							{
								TempL = *(uint64_t*)_addr(TempK + 0x8);
								Counter++;
							}
						}
					}
				}
			}
			if (Counter)
			{
				P1Character1Children.resize(Counter);

				switch (Counter)
				{
				case 1:
				default:
					P1Character1Children[0] = TempD;
					break;

				case 2:
					P1Character1Children[0] = TempD;
					P1Character1Children[1] = TempF;
					break;

				case 3:
					P1Character1Children[0] = TempD;
					P1Character1Children[1] = TempF;
					P1Character1Children[2] = TempH;
					break;

				case 4:
					P1Character1Children[0] = TempD;
					P1Character1Children[1] = TempF;
					P1Character1Children[2] = TempH;
					P1Character1Children[3] = TempJ;
					break;

				case 5:
					P1Character1Children[0] = TempD;
					P1Character1Children[1] = TempF;
					P1Character1Children[2] = TempH;
					P1Character1Children[3] = TempJ;
					P1Character1Children[4] = TempL;
					break;


				}

			}
			P1C1ChildrenCount = Counter;
		}


	}
}

void SetMeters()
{
	float P1MeterToFloat = P1Meter;
	float P2MeterToFloat = P2Meter;

	*((float*)(team1ptr + 0x78)) = P1MeterToFloat;
	*((float*)(team2ptr + 0x78)) = P2MeterToFloat;

}

void GetShots()
{
	uint64_t TempT;
	uint64_t TempU;
	uint64_t TempV;
	uint64_t TempW;
	uint64_t TempX;
	uint64_t TempY;
	uint64_t TempZ;
	uint64_t ContainerPtr;
	uint64_t CollisionPtr;
	uint64_t CollVtable;

	Player1ShotPointers.clear();
	Player2ShotPointers.clear();
	//Player1Shots.clear();
	//Player2Shots.clear();

	//P1ShotHitboxes.clear();
	//P2ShotHitboxes.clear();

	sShotList = *(uint64_t*)_addr(0x140D47F98);
	P1Shots = *(uint64_t*)_addr(0x140D47FA0);
	P2Shots = *(uint64_t*)_addr(0x140D47FC8);

	uint64_t CurrentPtr = *(uint64_t*)_addr(0x140D47FA0);
	uint64_t IPtr = CurrentPtr;
	//For Player 1. Thanks anotak.
	while (CurrentPtr && CurrentPtr != 0xFFFFFFFF && CurrentPtr != 0x140D47F98 && CurrentPtr != 0x140D47FC0)
	{
		IPtr = CurrentPtr;
		Player1ShotPointers.push_back(CurrentPtr);
		ShotData shotstuff;

		shotstuff.StopHitboxLoop = *(int*)_addr(IPtr + 0x48);
		float Somefloat = 0, SomeFloat2 = 0, SomeFloat3 = 0;
		Somefloat = *(float*)_addr(IPtr + 0x54);
		SomeFloat2 = *(float*)_addr(IPtr + 0x5C);
		SomeFloat3 = *(float*)_addr(IPtr + 0x64);

		TempT = CurrentPtr;
		//rAtkInfo..?
		shotstuff.rAtkInfoPtr = CurrentPtr + 0x40;

		//Shot file.
		TempU = *(uint64_t*)_addr(CurrentPtr + 0x18);
		if(TempU && TempU != 0xFFFFFFFF)
		{
			shotstuff.ShotVTAddress = *(uint64_t*)_addr(TempU);
			shotstuff.CurrentShotDuration = *(float*)_addr(TempU + 0x2078);
			shotstuff.FirstVector.X = *(float*)_addr(TempU + 0x50);
			shotstuff.FirstVector.Y = *(float*)_addr(TempU + 0x54);
			shotstuff.FirstVector.Z = *(float*)_addr(TempU + 0x58);

			shotstuff.SecondVector.X = *(float*)_addr(TempU + 0xB0);
			shotstuff.SecondVector.Y = *(float*)_addr(TempU + 0xB4);
			shotstuff.SecondVector.Z = *(float*)_addr(TempU + 0xB8);

			shotstuff.ThirdVector.X = *(float*)_addr(TempU + 0xF0);
			shotstuff.ThirdVector.Y = *(float*)_addr(TempU + 0xF4);
			shotstuff.ThirdVector.Z = *(float*)_addr(TempU + 0xF8);

			//cAtkCtrl for Shots: ShotPtr + 0x1470
			//CAtkCtrl for Characters: CharPtr + 0x1878

			shotstuff.FramesBeforeStartup = *(float*)_addr(TempU + 0x14A4);
			shotstuff.ATIID = *(float*)_addr(TempU + 0x1498);
			//I think this value controls whether or not the attack of the shot is active.
			shotstuff.WeirdValue = *(int*)_addr(TempU + 0x1F64);
			if (shotstuff.WeirdValue != 5 && shotstuff.FramesBeforeStartup < 1)
			{
				shotstuff.PeculiarCounter = *(int*)_addr(TempU + 0x2054);
				shotstuff.FirstCollPtr = *(uint64_t*)_addr(TempU + 0x2058);
				if (shotstuff.FirstCollPtr)
				{
					TempV = *(uint64_t*)_addr(shotstuff.FirstCollPtr + 0x30);
					if (TempV)
					{
						//This is where the while loop should be.
						while (*(uint64_t*)_addr(TempV) != 0x0 && *(uint64_t*)_addr(TempV) != 0xFFFFFFFF)
						{
							ContainerPtr = *(uint64_t*)_addr(TempV);
							if (ContainerPtr)
							{
								CollisionPtr = *(uint64_t*)_addr(ContainerPtr + 0x10);
								//Checks if the pointer actually points to somewhere.
								if (CollisionPtr)
								{

									////Checks if the duration is zero before considering adding those.
									//if (shotstuff.CurrentShotDuration != 0 && shotstuff.StopHitboxLoop == false)
									//{
										//Checks if Capsule or Sphere.
										CollVtable = *(uint64_t*)_addr(CollisionPtr);

										//If Capsule.
										if (CollVtable == 0x140a6b000)
										{
											Hitbox ShotBox;
											ShotBox.ContainerPos.x = *(float*)(ContainerPtr + 0x20);
											ShotBox.ContainerPos.y = *(float*)(ContainerPtr + 0x24);
											ShotBox.ContainerPos.z = *(float*)(ContainerPtr + 0x28);
											ShotBox.Radius = *(float*)_addr(ContainerPtr + 0x38);
											ShotBox.PointerToCapsuleData = CollisionPtr;

											ShotBox.CapsulePrimaryPos.x = *((float*)(CollisionPtr + 0x20));
											ShotBox.CapsulePrimaryPos.y = *((float*)(CollisionPtr + 0x24));
											ShotBox.CapsulePrimaryPos.z = *((float*)(CollisionPtr + 0x28));

											ShotBox.CapsuleSecondPos.x = *((float*)(CollisionPtr + 0x30));
											ShotBox.CapsuleSecondPos.y = *((float*)(CollisionPtr + 0x34));
											ShotBox.CapsuleSecondPos.z = *((float*)(CollisionPtr + 0x38));

											P1ShotHitCapsules.push_back(ShotBox);
										}
										//Else if a Sphere.
										else if (CollVtable == 0x140a6ae10)
										{
											Hurtbox ShotSphere;
											ShotSphere.CollData.Radius = *(float*)_addr(CollisionPtr + 0x2C);
											ShotSphere.PointerToMoreData = CollisionPtr;

											ShotSphere.CollData.Coordinates.X = *((float*)(CollisionPtr + 0x20));
											ShotSphere.CollData.Coordinates.Y = *((float*)(CollisionPtr + 0x24));
											ShotSphere.CollData.Coordinates.Z = *((float*)(CollisionPtr + 0x28));

											P1ShotHitSpheres.push_back(ShotSphere);
										}
									//}


								}



							}
							TempV = TempV + 0x08;
						}


					}

				}
			}

			//Ending of the loop.
			IPtr = IPtr + 0x8;
			CurrentPtr = *(uint64_t*)_addr(IPtr);
			
			Player1Shots.push_back(shotstuff);
		}


	}

	CurrentPtr = *(uint64_t*)_addr(0x140D47FC8);
	IPtr = 0x8;

	//For Player 2.
	while (CurrentPtr && CurrentPtr != 0xFFFFFFFF && CurrentPtr != 0x140D47F98 && CurrentPtr != 0x140D47FC0)
	{
		IPtr = CurrentPtr;
		Player2ShotPointers.push_back(CurrentPtr);
		ShotData shotstuff;

		TempT = CurrentPtr;
		//rAtkInfo..?
		shotstuff.rAtkInfoPtr = CurrentPtr + 0x40;

		//Shot file.
		TempU = *(uint64_t*)_addr(CurrentPtr + 0x18);
		if (TempU && TempU != 0xFFFFFFFF)
		{
			shotstuff.ShotVTAddress = *(uint64_t*)_addr(TempU);

			shotstuff.FirstVector.X = *(float*)_addr(TempU + 0x50);
			shotstuff.FirstVector.Y = *(float*)_addr(TempU + 0x54);
			shotstuff.FirstVector.Z = *(float*)_addr(TempU + 0x58);

			shotstuff.SecondVector.X = *(float*)_addr(TempU + 0xB0);
			shotstuff.SecondVector.Y = *(float*)_addr(TempU + 0xB4);
			shotstuff.SecondVector.Z = *(float*)_addr(TempU + 0xB8);

			shotstuff.ThirdVector.X = *(float*)_addr(TempU + 0xF0);
			shotstuff.ThirdVector.Y = *(float*)_addr(TempU + 0xF4);
			shotstuff.ThirdVector.Z = *(float*)_addr(TempU + 0xF8);

			shotstuff.FramesBeforeStartup = *(float*)_addr(TempU + 0x14A4);

			//I think this value controls whether or not the attack of the shot is active.
			shotstuff.WeirdValue = *(int*)_addr(TempU + 0x1F64);
			if(shotstuff.WeirdValue != 5 && shotstuff.FramesBeforeStartup < 1)
			{

				shotstuff.PeculiarCounter = *(int*)_addr(TempU + 0x2054);
				shotstuff.FirstCollPtr = *(uint64_t*)_addr(TempU + 0x2058);
				if (shotstuff.FirstCollPtr)
				{
					TempV = *(uint64_t*)_addr(shotstuff.FirstCollPtr + 0x30);
					if (TempV)
					{
						//This is where the while loop should be.
						while (*(uint64_t*)_addr(TempV) != 0x0 && *(uint64_t*)_addr(TempV) != 0xFFFFFFFF)
						{
							ContainerPtr = *(uint64_t*)_addr(TempV);
							if (ContainerPtr)
							{
								CollisionPtr = *(uint64_t*)_addr(ContainerPtr + 0x10);
								//Checks if the pointer actually points to somewhere.
								if (CollisionPtr)
								{

									////Checks if the duration is zero before considering adding those.
									//if (shotstuff.CurrentShotDuration != 0)
									//{
										//Checks if Capsule or Sphere.
										CollVtable = *(uint64_t*)_addr(CollisionPtr);

										//If Capsule.
										if (CollVtable == 0x140a6b000)
										{
											Hitbox ShotBox;
											ShotBox.ContainerPos.x = *(float*)(ContainerPtr + 0x20);
											ShotBox.ContainerPos.y = *(float*)(ContainerPtr + 0x24);
											ShotBox.ContainerPos.z = *(float*)(ContainerPtr + 0x28);
											ShotBox.Radius = *(float*)_addr(ContainerPtr + 0x38);
											ShotBox.PointerToCapsuleData = CollisionPtr;

											ShotBox.CapsulePrimaryPos.x = *((float*)(CollisionPtr + 0x20));
											ShotBox.CapsulePrimaryPos.y = *((float*)(CollisionPtr + 0x24));
											ShotBox.CapsulePrimaryPos.z = *((float*)(CollisionPtr + 0x28));

											ShotBox.CapsuleSecondPos.x = *((float*)(CollisionPtr + 0x30));
											ShotBox.CapsuleSecondPos.y = *((float*)(CollisionPtr + 0x34));
											ShotBox.CapsuleSecondPos.z = *((float*)(CollisionPtr + 0x38));

											P2ShotHitCapsules.push_back(ShotBox);
										}
										//Else if a Sphere.
										else if (CollVtable == 0x140a6ae10)
										{
											Hurtbox ShotSphere;
											ShotSphere.CollData.Radius = *(float*)_addr(CollisionPtr + 0x2C);
											ShotSphere.PointerToMoreData = CollisionPtr;

											ShotSphere.CollData.Coordinates.X = *((float*)(CollisionPtr + 0x20));
											ShotSphere.CollData.Coordinates.Y = *((float*)(CollisionPtr + 0x24));
											ShotSphere.CollData.Coordinates.Z = *((float*)(CollisionPtr + 0x28));

											P2ShotHitSpheres.push_back(ShotSphere);
										}
									//}


								}



							}
							TempV = TempV + 0x08;
						}


					}

				}

			}

			//Ending of the loop.
			IPtr = IPtr + 0x8;
			CurrentPtr = *(uint64_t*)_addr(IPtr);

			Player2Shots.push_back(shotstuff);
		}


	}


}

void GetChildData()
{
	P1C1ActiveChildData.clear();

	P2C1ActiveChildData.clear();


	P1C1ActiveChildData = GetChildCharacterData(P1Character1Data, P1Character1ID, Player1CharNodeTree, P1C1ActiveChildData);

	P2C1ActiveChildData = GetChildCharacterData(P2Character1Data, P2Character1ID, Player2CharNodeTree, P2C1ActiveChildData);

}

//ShotData GetShotData()
//{
//
//
//
//}

void GetDebugData()
{

}

static std::string HashToTypeName(uint64_t Hash)
{

	switch (Hash)
	{

		//Ryu Shots.
	case 0x140ad4eb0:
		return "uShotLaserReflectSinkuHadouken";

	case 0x140ad5330:
		return "uShotLaserSinkuHadouken";

	case 0x140ad5780:
		return "uShotVulcanHadouken";

		//Chun-Li Shots.

	case 0x140a9ff60:
		return "uShotVulcanKikouken";

	case 0x140a9faf0:
		return "uShotShieldKikousyou";

		//Gouki A.K.A. Akuma Shots.
	case 0x140ab4640:
		return "uShotVulcanGouHadouken";

	case 0x140ab41b0:
		return "uShotLaserMessatsuGouHadou";

	case 0x140ab4a90:
		return "uShotVulcanTenmaGouZanku";

		//Chris Shots.
	case 0x140a9b050:
		return "uShotLaserSatelliteLaser";

	case 0x140a9b4c0:
		return "uShotMissileGrenadeLauncher";

	case 0x140a9b920:
		return "uShotShieldFlameGun";

	case 0x140a9bd70:
		return "uShotVulcanFireGrenadeBomb";

	case 0x140a9c210:
		return "uShotVulcanFireGrenadeExplode";

	case 0x140a9c670:
		return "uShotVulcanFireGrenadeGroundFlame";

	case 0x140a9cad0:
		return "uShotVulcanGrenadeLauncherExplode";

	case 0x140a9cf30:
		return "uShotVulcanHandGrenadeBomb";

	case 0x140a9d380:
		return "uShotVulcanHandGrenadeExplode";

	case 0x140a9d7e0:
		return "uShotVulcanHandGun";

	case 0x140a9dc30:
		return "uShotVulcanLandMine";

	case 0x140a9e0a0:
		return "uShotVulcanLandMineExplode";

	case 0x140a9e4f0:
		return "uShotVulcanMachineGun";

	case 0x140a9e940:
		return "uShotVulcanMagnumGun";

	case 0x140a9ed90:
		return "uShotVulcanRocketLauncher";

	case 0x140a9f210:
		return "uShotVulcanSatelliteLaserLock";

	case 0x140a9f6a0:
		return "uShotVulcanShotGun";

		//Wesker's Samurai Edge.
	case 0x140ae5b40:
		return "uShotVulcanWeskerHandGun";

		//Viewtiful Joe's Shots.
	case 0x140ae4990:
		return "uShotVulcanBoomerang";

	case 0x140ae4e10:
		return "uShotVulcanShockingPinkBomb";

	case 0x140ae5290:
		return "uShotVulcanShockingPinkExplode";

	case 0x140ae56f0:
		return "uShotVulcanSixCannon";

		//Dante's Shots.		
	case 0x140aa10d0:
		return "uShotLaserCrystal";

	case 0x140aa1540:
		return "uShotLaserMillionCarat";

	case 0x140aa19b0:
		return "uShotLaserThunderBolt";

	case 0x140aa1e30:
		return "uShotLaserTwister";

	case 0x140aa2280:
		return "uShotMissileHysteric";

	case 0x140aa26f0:
		return "uShotOptionGrapple";

	case 0x140aa2b60:
		return "uShotShieldJamSession";

	case 0x140aa2fd0:
		return "uShotShieldTempest";

	case 0x140aa3420:
		return "uShotVulcanAcidRain";

	case 0x140aa3890:
		return "uShotVulcanAirPlay";

	case 0x140aa3cf0:
		return "uShotVulcanBulletDanceFinish";

	case 0x140aa4150:
		return "uShotVulcanDrive";

	case 0x140aa45a0:
		return "uShotVulcanEbonyAndIvory";

	case 0x140aa49f0:
		return "uShotVulcanFireWorks";

	case 0x140aa4e40:
		return "uShotVulcanMultiLock";

	case 0x140aa5290:
		return "uShotVulcanDualShot";

		//Trish.

	case 0x140adfaa0:
		return "uShotMissileTrishBullet";

	case 0x140adff10:
		return "uShotOptionSpadaBoomerang";

	case 0x140ae03a0:
		return "uShotVulcanBarrier";

	case 0x140ae07f0:
		return "uShotVulcanBarrierSpread";

	case 0x140ae0c40:
		return "uShotVulcanEnergyBolt";

		//Frank.

	case 0x140ab06e0:
		return "uShotLaserLikeAMegaman";//Unused.

	case 0x140ab0b50:
		return "uShotShieldWeaponCartSpread";

	case 0x140ab0ff0:
		return "uShotShieldWildPitchHit";

	case 0x140ab1460:
		return "uShotVulcanThrowWeapon";

	case 0x140ab18b0:
		return "uShotVulcanThrowWeaponB";

	case 0x140ab1d20:
		return "uShotVulcanWeaponCart";

	case 0x140ab2330:
		return "uShotVulcanWeaponCartSpread";

	case 0x140ab2780:
		return "uShotVulcanWildPitch";

		//Spencer.
	case 0x140ad91f0:
		return "uShotOptionAttackWire";

	case 0x140ad96a0:
		return "uShotOptionSwingWire";

	case 0x140ad9b00:
		return "uShotVulcanFurisosoguShi";//Bionic Bomber...?

	case 0x140ad9f50:
		return "uShotVulcanSwingWire";

		//Arthur.

	case 0x140a97a90:
		return "uShotMissileArrow";

	case 0x140a97f20:
		return "uShotOptionSickleBoomerang";

	case 0x140a983c0:
		return "uShotShieldKnightShield";

	case 0x140a98840:
		return "uShotVulcanAxe";

	case 0x140a98ca0:
		return "uShotVulcanBraceletOfGoddess";

	case 0x140a99100:
		return "uShotVulcanCrossSword";

	case 0x140a995a0:
		return "uShotVulcanGreatSorcery";

	case 0x140a99a20:
		return "uShotVulcanJavelin";

	case 0x140a99e90:
		return "uShotVulcanKnife";

	case 0x140a9a2f0:
		return "uShotVulcanTorch";

	case 0x140a9a750:
		return "uShotVulcanTorchFirePillar";

		//Amaterasu.

	case 0x140a94a10:
		return "uShotVulcanBakuen";

	case 0x140a94e80:
		return "uShotVulcanExtentThunder";

	case 0x140a952e0:
		return "uShotVulcanHyoran";

	case 0x140a95760:
		return "uShotVulcanMagatama";

	case 0x140a95be0:
		return "uShotVulcanOneSpark";

	case 0x140a96070:
		return "uShotVulcanOneSparkZan";

	case 0x140a964c0:
		return "uShotVulcanRaikou";

	case 0x140a96910:
		return "uShotVulcanRaikouSub";

	case 0x140a96d60:
		return "uShotVulcanTekadama";

	case 0x140a971c0:
		return "uShotVulcanTekadamaExplode";

	case 0x140a97620:
		return "uShotVulcanTsutamaki";

		//Zero.

	case 0x140ae5fb0:
		return "uShotLaserRaikousen";

	case 0x140ae6420:
		return "uShotLaserRekkouha";

	case 0x140ae6870:
		return "uShotVulcanGenmuZero";

	case 0x140ae6cc0:
		return "uShotVulcanHadangeki";

	case 0x140ae7110:
		return "uShotVulcanZeroBuster";


		//Tron Bonne.

	case 0x140ae1090:
		return "uShotVulcanChushokuRash";

	case 0x140ae1530:
		return "uShotVulcanKingKobun";

	case 0x140ae19a0:
		return "uShotVulcanKobunLauncher";

	case 0x140ae1df0:
		return "uShotVulcanRockThrow";

	case 0x140ae2260:
		return "uShotVulcanShikiDan";

	case 0x140ae26b0:
		return "uShotVulcanShikiDanKobun";

		//Morrigan.

	case 0x140ac5860:
		return "uShotOptionSoulFistAbsorb";

	case 0x140ac5d10:
		return "uShotVulcanFinishingShower";

	case 0x140ac61b0:
		return "uShotVulcanShilhouetteBlade";

	case 0x140ac6600:
		return "uShotVulcanSoulFist";

		//Hsien-Ko/Lei-Lei.

	case 0x140abf2e0:
		return "uShotVulcanAnkihouBomb";

	case 0x140abf790:
		return "uShotVulcanAnkihouBombExplode";

	case 0x140abfbf0:
		return "uShotVulcanAnkihouKobun";

	case 0x140ac00c0:
		return "uShotVulcanAnkihouRolling";

	case 0x140ac0570:
		return "uShotVulcanHenkyouKi";

	case 0x140ac09c0:
		return "uShotVulcanTenraiha";

	case 0x140ac0e20:
		return "uShotVulcanZireitou";


		//Felicia.

	case 0x140aafdc0:
		return "uShotVulcanPleaseHelpMe";

	case 0x140ab0270:
		return "uShotVulcanSandSplash";

		//Crimson Viper.

	case 0x140aa03b0:
		return "uShotLaserOpticBlast";

	case 0x140aa0830:
		return "uShotVulcanBurningKick";

	case 0x140aa0c80:
		return "uShotVulcanSeismoHammer";

		//Haggar.

	case 0x140ab4ee0:
		return "uShotVulcanBodyPressFinish";

	case 0x140ab5350:
		return "uShotVulcanIronPipeRolling";

		//Jill.
	case 0x140abee70:
		return "uShotVulcanJillMachineGun";


		//Strider Hiryu.

	case 0x140ab9110:
		return "uShotShieldFormationD";

	case 0x140ab95c0:
		return "uShotShieldOuroboros";

	case 0x140ab9a70:
		return "uShotShieldOuroborosExit";

	case 0x140ab9ec0:
		return "uShotVulcanFormationATiger";

	case 0x140aba350:
		return "uShotVulcanFormationBHawk";

	case 0x140aba7c0:
		return "uShotVulcanFormationCBom";

	case 0x140abac50:
		return "uShotVulcanFormationCExplode";

	case 0x140abb0c0:
		return "uShotVulcanFormationCSpread";

	case 0x140abb550:
		return "uShotVulcanFormationDSpread";

	case 0x140abb9c0:
		return "uShotVulcanLegionHawk";

	case 0x140abbe10:
		return "uShotVulcanLegionTiger";

	case 0x140abc260:
		return "uShotVulcanOuroborosShot";

		//Vergil Shots.
	case 0x140ae33f0:
		return "uShotShieldSpiralSword";

	case 0x140ae3840:
		return "uShotShieldStormSword";

	case 0x140ae2f90:
		return "uShotShieldRaidSword";

	case 0x140ae4540:
		return "uShotVulcanStormSwordSpread";

	case 0x140ae2b00:
		return "uShotOptionRoundTrip";

	case 0x140ae3ca0:
		return "uShotVulcanJigenZan";//AKA Judgment Cut.

	case 0x140ae40f0:
		return "uShotVulcanRapidSlash";


		//Mr. Wright.


		//Red Arremer AKA Firebrand.


		//Nemesis.


		//Spider-Man.


		//Captain America's Shield Slash.
	case 0x140a9aba0:
		return "uShotOptionShieldSlash";

		//Wolverine does not have any shot files.


		//Magneto Shots.
	case 0x140ac1b30:
		return "uShotMissileGravitySqueeze";

	case 0x140ac1f90:
		return "uShotVulcanHyperGravitation";

	case 0x140ac23f0:
		return "uShotVulcanMagneticBlast";

	case 0x140ac2840:
		return "uShotVulcanMagneticTempst";

	case 0x140ac2ca0:
		return "uShotVulcanMagnetoAtkSitM";



		//Taskmaster Shots.
	case 0x140add7c0:
		return "uShotVulcanArrowRain";

	case 0x140addc30:
		return "uShotVulcanArrowShot";

	case 0x140ade080:
		return "uShotVulcanBullet";








		//case "":
		//	return "";


	default:
		return "?????";
	}

}

static void GetCharacterHurtboxData(std::vector<Hurtbox> PlayerHurtboxes, int HurtboxCount, uint64_t PlayerPtr)
{

	uint64_t t = 0;
	uint64_t tt = 0;
	uint64_t ttt = 0;
	uint64_t tttt = 0;
	uint64_t ttttt = 0;
	uint64_t tttttt = 0;
	uint64_t v = 0;
	uint64_t w = 0;
	uint64_t ww = 0;
	uint64_t g = 0;
	uint64_t gg = 0;
	uint64_t pp = 0;
	uint64_t ppp = 0;
	uint64_t qq = 0;
	HurtboxCount = 0;
	//P1ProjectileCount = 0;
	//P2ProjectileCount = 0;

	t = *(uint64_t*)_addr(PlayerPtr + 0x4E10);
	if (t && t < 0x140000000 && t != 0xffffffff && t != 0x10000000f)
	{
		tt = *(uint64_t*)_addr(t + 0x30);
		tttt = tt;
		/*
		ReadProcessMemory(hProcess, (LPVOID*)(P1Character1Data + 0x4E10), &t, sizeof(uint64_t), 0);
		ReadProcessMemory(hProcess, (LPVOID*)(t + 0x30), &tt, sizeof(uint64_t), 0);
		tttt = tt;
		*/

		if (tt && tt < 0x140000000) {
			//This is for getting the hurtbox count.
			for (size_t i = 0; i < 100; i++)
			{
				ttt = *(uint64_t*)_addr(tt);
				if (ttt == 0) {
					break;
				}
				//tt++;;
				tt = tt + 8;
				HurtboxCount++;
			}

			PlayerHurtboxes.clear();
			PlayerHurtboxes.resize(HurtboxCount);

			uint64_t* HurtboxPointers = new uint64_t[HurtboxCount];
			tt = tttt;


			if (HurtboxCount && HurtboxCount < 255)
				//Once More, but this time for the Hurtbox pointers.
			{
				for (int j = 0; j < HurtboxCount; j++)
				{
					v = *(uint64_t*)_addr(tt);
					HurtboxPointers[j] = v;
					//Stuff to get the relevant data.
					//v goes to a cHitPrimSphere node;
					if (v) {
						PlayerHurtboxes[j].PointerIdentifier = *(uint64_t*)_addr(v);
						PlayerHurtboxes[j].PointerToMoreData = *(uint64_t*)_addr(v + 0x10);

						if (PlayerHurtboxes[j].PointerToMoreData) {

							PlayerHurtboxes[j].SecondaryX = *((float*)(v + 0x20));
							PlayerHurtboxes[j].SecondaryY = *((float*)(v + 0x24));
							PlayerHurtboxes[j].SecondaryZ = *((float*)(v + 0x28));

							PlayerHurtboxes[j].CollData.Coordinates.X = *((float*)(PlayerHurtboxes[j].PointerToMoreData + 0x20));
							PlayerHurtboxes[j].CollData.Coordinates.Y = *((float*)(PlayerHurtboxes[j].PointerToMoreData + 0x24));
							PlayerHurtboxes[j].CollData.Coordinates.Z = *((float*)(PlayerHurtboxes[j].PointerToMoreData + 0x28));
							PlayerHurtboxes[j].CollData.Radius = *((float*)(PlayerHurtboxes[j].PointerToMoreData + 0x2C));

							tt = tt + 8;
						}
						else
						{
							//break;
						}
					}
				}

				tt = tttt;
				uint64_t backupt = tttt;
				//And again for the defacto coordinates.
				for (int k = 0; k < HurtboxCount; k++)
				{
					tt = backupt;
					tt += k * 8;
					ttttt = *(uint64_t*)_addr(tt);
					if (ttttt)
					{
						tttttt = *(uint64_t*)(ttttt);
						PlayerHurtboxes[k].DeFactoX = *(float*)(ttttt + 0x30);
						PlayerHurtboxes[k].DeFactoY = *(float*)(ttttt + 0x34);
						PlayerHurtboxes[k].DeFactoZ = *(float*)(ttttt + 0x38);
					}
				}
			}

		}

		if (PlayerPtr == P1Character1Data)
		{
			P1C1Hurtboxes = PlayerHurtboxes;
			P1C1HurtboxCount = HurtboxCount;
		}
		else if (PlayerPtr == P1Character2Data)
		{
			P1C2Hurtboxes = PlayerHurtboxes;
			P1C2HurtboxCount = HurtboxCount;
		}
		else if (PlayerPtr == P1Character3Data)
		{
			P1C3Hurtboxes = PlayerHurtboxes;
			P1C3HurtboxCount = HurtboxCount;
		}
		else if (PlayerPtr == P2Character1Data)
		{
			P2C1Hurtboxes = PlayerHurtboxes;
			P2C1HurtboxCount = HurtboxCount;
		}
		else if (PlayerPtr == P2Character2Data)
		{
			P2C2Hurtboxes = PlayerHurtboxes;
			P2C2HurtboxCount = HurtboxCount;

		}
		else if (PlayerPtr == P2Character3Data)
		{
			P2C3Hurtboxes = PlayerHurtboxes;
			P2C3HurtboxCount = HurtboxCount;
		}
	}


}

void GetHurtboxData()
{
	//The Hurtboxes aren't really stored in the same way as the character data stuff I'm used to working with normally.
	//Have to use this funky looking method to get the characters' hurtboxes during runtime.
	uint64_t t;
	uint64_t tt;
	uint64_t ttt;
	uint64_t tttt;
	uint64_t v;
	uint64_t w = 0;
	uint64_t ww;
	uint64_t g = 0;
	uint64_t gg;
	uint64_t pp = 0;
	uint64_t ppp;
	uint64_t qq;
	P1C1HurtboxCount = 0;
	P1C2HurtboxCount = 0;
	P1C3HurtboxCount = 0;
	P2C1HurtboxCount = 0;
	P2C2HurtboxCount = 0;
	P2C3HurtboxCount = 0;
	P1ProjectileCount = 0;
	P2ProjectileCount = 0;

	if (P1Character1Data)
	{
		GetCharacterHurtboxData(P1C1Hurtboxes, P1C1HurtboxCount, P1Character1Data);
	}
	if (P1Character2Data)
	{
		GetCharacterHurtboxData(P1C2Hurtboxes, P1C2HurtboxCount, P1Character2Data);
	}
	if (P1Character3Data)
	{
		GetCharacterHurtboxData(P1C3Hurtboxes, P1C3HurtboxCount, P1Character3Data);
	}
	if (P2Character1Data)
	{
		GetCharacterHurtboxData(P2C1Hurtboxes, P2C1HurtboxCount, P2Character1Data);
	}
	if (P2Character2Data)
	{
		GetCharacterHurtboxData(P2C2Hurtboxes, P2C2HurtboxCount, P2Character2Data);
	}
	if (P2Character3Data)
	{
		GetCharacterHurtboxData(P2C3Hurtboxes, P2C3HurtboxCount, P2Character3Data);
	}

	w = 0;
	g = 0;


	//ww = 0x140D47F98;
	//while (w != -1 && w != 0x140D47F98)
	//{
	//	w = *(uint64_t*)_addr(ww + 0x8);
	//	//ReadProcessMemory(hProcess, (LPVOID*)((ww + 0x8)), &w, sizeof(uint64_t), 0);
	//	if (w != -1)
	//	{
	//		ww = w;
	//	}
	//	if (w != -1 && w != 0x140D47F98)
	//	{
	//		P1ProjectileCount++;
	//	}

	//}

	////Player 2 Shots. Let's try to iterate through this properly.
	//gg = 0x140D47F98;
	//while (g != -1 && g != 0x140d47fc0)
	//{
	//	g = *(uint64_t*)_addr(gg + 0x30);
	//	//ReadProcessMemory(hProcess, (LPVOID*)((gg + 0x30)), &g, sizeof(uint64_t), 0);
	//	if (g != -1)
	//	{
	//		gg = g;
	//	}
	//	if (g != -1 && g != 0x140d47fc0)
	//	{
	//		P2ProjectileCount++;
	//	}

	//}

	//pp = 0;

	//ActiveShots.clear();
	////ActiveShots.resize((P1ProjectileCount + P2ProjectileCount));
	//ActiveShots.resize((static_cast<std::vector<ProjectileData, std::allocator<ProjectileData>>::size_type>(P1ProjectileCount) + P2ProjectileCount));


	//ppp = 0x140D47F98;
	//for (int j = 0; j < P1ProjectileCount; j++)
	//{
	//	if (pp != -1 && pp != 0x140D47F98)
	//	{
	//		pp = *(uint64_t*)_addr(ppp + 0x8);
	//		//ReadProcessMemory(hProcess, (LPVOID*)((ppp + 0x8)), &pp, sizeof(uint64_t), 0);
	//		if (pp != -1)
	//		{
	//			ppp = pp;
	//		}
	//		qq = (pp - 0x1450);
	//		ActiveShots[j].ShotVTAddress = *(uint64_t*)_addr(qq);
	//		//ReadProcessMemory(hProcess, (LPVOID*)(qq), &ActiveShots[j].ShotVTAddress, sizeof(uint64_t), 0);
	//		ActiveShots[j].ProjName = HashToTypeName(ActiveShots[j].ShotVTAddress);

	//		ActiveShots[j].PossibleAbsPos.X = *((float*)(qq + 0x50));
	//		ActiveShots[j].PossibleAbsPos.Y = *((float*)(qq + 0x54));
	//		ActiveShots[j].PossibleAbsPos.Z = *((float*)(qq + 0x58));

	//		ActiveShots[j].PossibleQuat.X = *((float*)(qq + 0x60));
	//		ActiveShots[j].PossibleQuat.Y = *((float*)(qq + 0x64));
	//		ActiveShots[j].PossibleQuat.Z = *((float*)(qq + 0x68));
	//		ActiveShots[j].PossibleQuat.W = *((float*)(qq + 0x6C));

	//		ActiveShots[j].PossibleScale.X = *((float*)(qq + 0x70));
	//		ActiveShots[j].PossibleScale.Y = *((float*)(qq + 0x74));
	//		ActiveShots[j].PossibleScale.Z = *((float*)(qq + 0x78));

	//	}
	//}


	//for (int j = 0; j < P2ProjectileCount; j++)
	//{

	//}

	//Acquires the Character states.
	if (P1Character1Data)
	{
		P1C1CharState = *(uint32_t*)(P1Character1Data + 0x14FC);
		P1C1VulnState = *(byte*)(P1Character1Data + 0x14FC);
		P1C1ChrStateBinary = std::bitset<32>(P1C1CharState).to_string();
	}

	if (P1Character2Data)
	{
		P1C2CharState = *(uint32_t*)(P1Character2Data + 0x14FC);
		P1C2VulnState = *(byte*)(P1Character2Data + 0x14FC);
		P1C2ChrStateBinary = std::bitset<32>(P1C2CharState).to_string();
	}

	if (P1Character3Data)
	{
		P1C3CharState = *(uint32_t*)(P1Character3Data + 0x14FC);
		P1C3VulnState = *(byte*)(P1Character3Data + 0x14FC);
		P1C3ChrStateBinary = std::bitset<32>(P1C3CharState).to_string();
	}

	if (P2Character1Data)
	{
		P2C1CharState = *(uint32_t*)(P2Character1Data + 0x14FC);
		P2C1VulnState = *(byte*)(P2Character1Data + 0x14FC);
		P2C1ChrStateBinary = std::bitset<32>(P2C1CharState).to_string();
	}

	if (P2Character2Data)
	{
		P2C2CharState = *(uint32_t*)(P2Character2Data + 0x14FC);
		P2C2VulnState = *(byte*)(P2Character2Data + 0x14FC);
		P2C2ChrStateBinary = std::bitset<32>(P2C2CharState).to_string();
	}

	if (P2Character3Data)
	{
		P2C3CharState = *(uint32_t*)(P2Character3Data + 0x14FC);
		P2C3VulnState = *(byte*)(P2Character3Data + 0x14FC);
		P2C3ChrStateBinary = std::bitset<32>(P2C3CharState).to_string();
	}



}

static void GetCharacterHitboxData(std::vector<Hitbox> PlayerHitboxes, int HitboxCount, uint64_t PlayerPtr)
{


	uint64_t u;
	uint64_t uu;
	uint64_t uuu = 0;
	uint64_t uuuu = 0;
	uint64_t w = 0;
	uint64_t pp = 0;
	uint64_t ppp = 0;
	uint64_t pppp = 0;
	uint64_t ppppp = 0;
	//uint64_t CapsulePtr = 0;
	uint64_t Capsule = 0;

	//Fix this later.
	if (PlayerPtr)
	{
		u = *(uint64_t*)_addr(PlayerPtr + 0x4200);
		if (u && u < 0x140000000)
		{
			uu = *(uint64_t*)_addr(u + 0x30);
			uuuu = uu;

			//Active Hitbox Count.
			HitboxCount = *((int*)(u + 0x20));

			PlayerHitboxes.clear();
			PlayerHitboxes.resize(HitboxCount);

			uint64_t* HurtboxPointers = new uint64_t[HitboxCount];
			uu = uuuu;

			for (int j = 0; j < HitboxCount; j++)
			{
				uuuu = uu;
				w = *(uint64_t*)_addr(uu);
				//First the container data.
				PlayerHitboxes[j].ContainerPos.x = *(float*)(w + 0x20);
				PlayerHitboxes[j].ContainerPos.y = *(float*)(w + 0x24);
				PlayerHitboxes[j].ContainerPos.z = *(float*)(w + 0x28);
				PlayerHitboxes[j].Radius = *(float*)(w + 0x38);
				PlayerHitboxes[j].PointerToCapsuleData = *(uint64_t*)(w + 0x10);

				//Checks if the pointer actually points to somewhere.
				if (PlayerHitboxes[j].PointerToCapsuleData)
				{
					//Then we use the pointer here to go to the capsule data.
					Capsule = *(uint64_t*)_addr(w + 0x10);

					PlayerHitboxes[j].CapsulePrimaryPos.x = *((float*)(Capsule + 0x20));
					PlayerHitboxes[j].CapsulePrimaryPos.y = *((float*)(Capsule + 0x24));
					PlayerHitboxes[j].CapsulePrimaryPos.z = *((float*)(Capsule + 0x28));

					PlayerHitboxes[j].CapsuleSecondPos.x = *((float*)(Capsule + 0x30));
					PlayerHitboxes[j].CapsuleSecondPos.y = *((float*)(Capsule + 0x34));
					PlayerHitboxes[j].CapsuleSecondPos.z = *((float*)(Capsule + 0x38));
				}

				uu = uu + 8;
			}

		}


		if (PlayerPtr == P1Character1Data)
		{
			P1C1Hitboxes = PlayerHitboxes;
			P1C1HitboxCount = HitboxCount;
		}
		else if (PlayerPtr == P1Character2Data)
		{
			P1C2Hitboxes = PlayerHitboxes;
			P1C2HitboxCount = HitboxCount;
		}
		else if (PlayerPtr == P1Character3Data)
		{
			P1C3Hitboxes = PlayerHitboxes;
			P1C3HitboxCount = HitboxCount;
		}
		else if (PlayerPtr == P2Character1Data)
		{
			P2C1Hitboxes = PlayerHitboxes;
			P2C1HitboxCount = HitboxCount;
		}
		else if (PlayerPtr == P2Character2Data)
		{
			P2C2Hitboxes = PlayerHitboxes;
			P2C2HitboxCount = HitboxCount;

		}
		else if (PlayerPtr == P2Character3Data)
		{
			P2C3Hitboxes = PlayerHitboxes;
			P2C3HitboxCount = HitboxCount;
		}









	}



}

void GetHitboxDataPart1()
{

	uint64_t u;
	uint64_t uu;
	uint64_t uuu;
	uint64_t uuuu;
	uint64_t w;
	uint64_t pp = 0;
	uint64_t ppp;
	P1C1HitboxCount = 0;
	P1C2HitboxCount = 0;
	P1C3HitboxCount = 0;
	P2C1HitboxCount = 0;
	P2C2HitboxCount = 0;
	P2C3HitboxCount = 0;


	GetCharacterHitboxData(P1C1Hitboxes, P1C1HitboxCount, P1Character1Data);
	GetCharacterHitboxData(P1C2Hitboxes, P1C2HitboxCount, P1Character2Data);
	GetCharacterHitboxData(P1C3Hitboxes, P1C3HitboxCount, P1Character3Data);
	GetCharacterHitboxData(P2C1Hitboxes, P2C1HitboxCount, P2Character1Data);
	GetCharacterHitboxData(P2C2Hitboxes, P2C2HitboxCount, P2Character2Data);
	GetCharacterHitboxData(P2C3Hitboxes, P2C3HitboxCount, P2Character3Data);

}

//Carries over changes To restart.
void RestartWithChanges()
{
}

void GetCameraStuff()
{

	GetMainPointers();

	auto TempA = *(uint64_t*)_addr(0x140E17930);
	//auto TempB = *(uint64_t*)_addr(TempA + 0x58);
	uActionSideCamera = *(uint64_t*)_addr(TempA + 0x58);  //*(uint64_t*)_addr(0x140E17930);

	if (uActionSideCamera)
	{
		FarPlane = *(float*)(uActionSideCamera + 0x40);
		NearPlane = *(float*)(uActionSideCamera + 0x44);
		mFOV = *(float*)(uActionSideCamera + 0x4C);
		CameraPos.x = *(float*)(uActionSideCamera + 0x50);
		CameraPos.y = *(float*)(uActionSideCamera + 0x54);
		CameraPos.z = *(float*)(uActionSideCamera + 0x58);
		CameraUp.x = *(float*)(uActionSideCamera + 0x60);
		CameraUp.y = *(float*)(uActionSideCamera + 0x64);
		CameraUp.z = *(float*)(uActionSideCamera + 0x68);
		TargetPos.x = *(float*)(uActionSideCamera + 0x70);
		TargetPos.y = *(float*)(uActionSideCamera + 0x74);
		TargetPos.z = *(float*)(uActionSideCamera + 0x78);
	}


}

//Updates things every frame.
void TickUpdates()
{
	GetCameraStuff();

}
