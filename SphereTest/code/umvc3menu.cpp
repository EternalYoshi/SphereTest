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
#include "..\gui\imgui\imgui.h"
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

//Forces the X Factor Timer to stay at a stupid high time to be unlimited.
void EndlessXFactorUpdate()
{
	GetMainPointers();
	GetPlayerData();
	//float fli = 7000.0;

	//Team1.
	if (EndlessXFactorP1) {
		*(float*)(team1ptr + 0xC0) = 7000.0;

	}
	//Team2.
	if (EndlessXFactorP2)
	{
		*(float*)(team2ptr + 0xC0) = 7000.0;

	}
}

//Checks for valid vtable address at start of struct to hopefully stop crashing/running when input display is either off or not present.
bool InputDisplayValidator()
{

	long long vftaddress = *((long long*)(uBtKeyDisp));
	/*ReadProcessMemory(hProcess, (LPVOID*)(uBtKeyDisp), &vftaddress, sizeof(vftaddress), 0);*/

	if (vftaddress == 5380359088)
	{
		return true;
	}
	else
	{
		return false;
	}
	return false;
}

void LeftSideInputDisplay()
{
	if (MoveInputDisplay == true)
	{
		uint64_t TempA = *(uint64_t*)_addr(0x140E1BC98);
		uint64_t TempB = *(uint64_t*)_addr(TempA + 0x70);
		if (TempA && TempB)
		{
			uint64_t TempC = *(uint64_t*)_addr(TempB + 0x20);
			uint64_t TempD = *(uint64_t*)_addr(TempC + 0x70);
			uint64_t TempE = *(uint64_t*)_addr(TempD);
			uBtKeyDisp = TempD;

			float TempFloat = 0.0;
			float TempFloatTwo = 0.7;
			if (InputDisplayValidator())
			{
				if (TempA == 0 || TempB == 0 || TempC == 0 || uBtKeyDisp == 0)
				{
				}
				else
				{
					//InputDisplayDataPointer stuff.
					TempFloat = 15;

					*(float*)(uBtKeyDisp + 0x70) = TempFloat;

					TempFloat = 15;

					*(float*)(uBtKeyDisp + 0x74) = TempFloat;

					TempFloat = 0;

					*(float*)(uBtKeyDisp + 0x78) = TempFloat;

					*(float*)(uBtKeyDisp + 0x7C) = TempFloat;

					TempFloat = 0.00146484;

					*(float*)(uBtKeyDisp + 0x80) = TempFloat;

					TempFloat = 0.7;

					*(float*)(uBtKeyDisp + 0x84) = TempFloat;

					TempFloat = 0;

					*(float*)(uBtKeyDisp + 0x88) = TempFloat;

					*(float*)(uBtKeyDisp + 0x8C) = TempFloat;

					TempFloat = 0.7;

					*(float*)(uBtKeyDisp + 0x90) = TempFloat;

					TempFloat = 0.00146484;

					*(float*)(uBtKeyDisp + 0x94) = TempFloat;

					TempFloat = 0;

					*(float*)(uBtKeyDisp + 0x98) = TempFloat;

					*(float*)(uBtKeyDisp + 0x9C) = TempFloat;

					//Part Two. This part gets the uBtKeyDispBtn table for scaling purposes. I wonder if I can do this without
					//affecting perforamnce...

					ButtonCount = *((int*)(uBtKeyDisp + 0x50));

					uBtKeyDispBtnTable = *(uint64_t*)_addr(uBtKeyDisp + 0x60);

					uint64_t TempF = uBtKeyDispBtnTable;
					uint64_t TempG = 0;
					for (int v = 0; v < ButtonCount; v++)
					{
						TempG = *(uint64_t*)_addr(TempF);

						*(float*)(TempG + 0x60) = TempFloatTwo;
						*(float*)(TempG + 0x64) = TempFloatTwo;

						TempF = TempF + 0x08;
					}

				}

			}
		}
	}





}

//Sets Individual Character HP on Training Mode restarts.
void SetIndividualCharacterHealth()
{

	//Gets the Max HP Value for the active characters. They are ints instead of floats.
	P1C1MaxHP = *(uint64_t*)_addr((P1Character1Data + 0x154C));
	P1C2MaxHP = *(uint64_t*)_addr((P1Character2Data + 0x154C));
	P1C3MaxHP = *(uint64_t*)_addr((P1Character3Data + 0x154C));
	P2C1MaxHP = *(uint64_t*)_addr((P2Character1Data + 0x154C));
	P2C2MaxHP = *(uint64_t*)_addr((P2Character2Data + 0x154C));
	P2C3MaxHP = *(uint64_t*)_addr((P2Character3Data + 0x154C));


	if (SetIndividualHP == true)
	{



		//P1 Character 1.
		*(float*)(P1Character1Data + 0x1550) = P1C1MaxHP * P1Char1Health;

		//P1 Character 2.
		*(float*)(P1Character2Data + 0x1550) = P1C2MaxHP * P1Char3Health;

		//P1 Character 3.
		*(float*)(P1Character3Data + 0x1550) = P1C3MaxHP * P1Char3Health;

		//P2 Character 1.
		*(float*)(P2Character1Data + 0x1550) = P2C1MaxHP * P2Char1Health;

		//P2 Character 2.
		*(float*)(P2Character2Data + 0x1550) = P2C2MaxHP * P2Char2Health;

		//P2 Character 3.
		*(float*)(P2Character3Data + 0x1550) = P2C3MaxHP * P2Char3Health;

		//Applies the requested values to the characters' red health as well.
		if (AlsoSetRedHealthP1 == true)
		{

			//P1 Character 1.
			*(float*)(P1Character1Data + 0x1558) = P1C1MaxHP * P1Char1Health;

			//P1 Character 2.
			*(float*)(P1Character2Data + 0x1558) = P1C2MaxHP * P1Char3Health;

			//P1 Character 3.
			*(float*)(P1Character3Data + 0x1558) = P1C3MaxHP * P1Char3Health;

		}

		if (AlsoSetRedHealthP2 == true)
		{

			//P2 Character 1.
			*(float*)(P2Character1Data + 0x1558) = P2C1MaxHP * P2Char1Health;

			//P2 Character 2.
			*(float*)(P2Character2Data + 0x1558) = P2C2MaxHP * P2Char2Health;

			//P2 Character 3.
			*(float*)(P2Character3Data + 0x1558) = P2C3MaxHP * P2Char3Health;
		}

	}



}

//Adjusts Frank's level based on the Slider's value.
void ChangeFrankLevel(int FrankLevel)
{
	GetPlayerData();
	GetCharacterIDs();
	//Sets Prestige Points based on level chosen.
	switch (FrankLevel)
	{
	case 1:
		PrestigePoints = 0.0;
		break;
	case 2:
		PrestigePoints = 5.0;
		break;
	case 3:
		PrestigePoints = 20.0;
		break;
	case 4:
		PrestigePoints = 50.0;
		break;
	case 5:
		PrestigePoints = 100.0;
		break;
	default:
		PrestigePoints = 0.0;
		break;

	}

	if (P1Character1ID != 0)
	{
		if (P1Character1ID == 9)
		{
			*(int*)_addr(P1Character1Data + 0x69C4) = FrankLevel;
			*(float*)(P1Character1Data + 0x69DC) = PrestigePoints;

		}

		if (P1Character2ID == 9)
		{
			*(int*)_addr(P1Character2Data + 0x69C4) = FrankLevel;
			*(float*)(P1Character2Data + 0x69DC) = PrestigePoints;

		}

		if (P1Character3ID == 9)
		{
			*(int*)_addr(P1Character3Data + 0x69C4) = FrankLevel;
			*(float*)(P1Character3Data + 0x69DC) = PrestigePoints;

		}

		if (P2Character1ID == 9)
		{
			*(int*)_addr(P2Character1Data + 0x69C4) = FrankLevel;
			*(float*)(P2Character1Data + 0x69DC) = PrestigePoints;

		}

		if (P2Character2ID == 9)
		{
			*(int*)_addr(P2Character2Data + 0x69C4) = FrankLevel;
			*(float*)(P2Character2Data + 0x69DC) = PrestigePoints;

		}

		if (P2Character3ID == 9)
		{
			*(int*)_addr(P2Character3Data + 0x69C4) = FrankLevel;
			*(float*)(P2Character3Data + 0x69DC) = PrestigePoints;

		}
	}




}

//Sets Evidence based on selection.
void ChangeWrightEvidence()
{
	GetPlayerData();
	GetCharacterIDs();

	//Checks Character ID.
	if (P1Character1ID == 23)
	{
		*(int*)_addr(P1Character1Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P1Character1Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P1Character1Data + 0x6920) = WrightEvidenceC;
	}
	if (P1Character2ID == 23)
	{
		*(int*)_addr(P1Character2Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P1Character2Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P1Character2Data + 0x6920) = WrightEvidenceC;
	}
	if (P1Character3ID == 23)
	{
		*(int*)_addr(P1Character3Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P1Character3Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P1Character3Data + 0x6920) = WrightEvidenceC;
	}
	if (P2Character1ID == 23)
	{
		*(int*)_addr(P2Character1Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P2Character1Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P2Character1Data + 0x6920) = WrightEvidenceC;
	}
	if (P2Character2ID == 23)
	{
		*(int*)_addr(P2Character2Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P2Character2Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P2Character2Data + 0x6920) = WrightEvidenceC;
	}
	if (P2Character3ID == 23)
	{
		*(int*)_addr(P2Character3Data + 0x6918) = WrightEvidenceA;
		*(int*)_addr(P2Character3Data + 0x691C) = WrightEvidenceB;
		*(int*)_addr(P2Character3Data + 0x6920) = WrightEvidenceC;
	}

}

int EvidenceNameToID(const char* name)
{

	if (name == "Nothing")
	{
		return -1;
	}
	else if (name == "Photo")
	{
		return 0;
	}
	else if (name == "Watch")
	{
		return 1;
	}
	else if (name == "Vase")
	{
		return 2;
	}
	else if (name == "Folder")
	{
		return 3;
	}
	else if (name == "Phone")
	{
		return 4;
	}
	else if (name == "Knife")
	{
		return 5;
	}
	else if (name == "Sunglasses")
	{
		return 6;
	}
	else if (name == "Beer")
	{
		return 7;
	}
	else if (name == "Bonsai")
	{
		return 8;
	}
	else if (name == "Doll")
	{
		return 9;
	}
	else if (name == "Figurine")
	{
		return 10;
	}
	else if (name == "Plunger")
	{
		return 11;
	}
	else if (name == "Chicken")
	{
		return 12;
	}

	return -1;

}

//Rough version of Turnabout Mode.
void Objection()
{
	int TBOn = 2;
	int TBOff = 0;
	GetPlayerData();
	GetCharacterIDs();

	if (P1Character1ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P1Character1Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P1Character1Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P1Character1Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P1Character1Data + 0x1548) = TBOff;
		}
	}
	if (P1Character2ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P1Character2Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P1Character2Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P1Character2Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P1Character2Data + 0x1548) = TBOff;
		}
	}
	if (P1Character3ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P1Character3Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P1Character3Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P1Character3Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P1Character3Data + 0x1548) = TBOff;
		}
	}
	if (P2Character1ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P2Character1Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P2Character1Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P2Character1Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P2Character1Data + 0x1548) = TBOff;
		}
	}
	if (P2Character2ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P2Character2Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P2Character2Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P2Character2Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P2Character2Data + 0x1548) = TBOff;
		}
	}
	if (P2Character3ID == 23)
	{
		if (Turnabout == true)
		{
			*(FighterInstall*)_addr(P2Character3Data + 0x15F0) = TurnaboutMode;
			*(int*)_addr(P2Character3Data + 0x1548) = TBOn;
		}
		else
		{
			*(FighterInstall*)_addr(P2Character3Data + 0x15F0) = InvestigationMode;
			*(int*)_addr(P2Character3Data + 0x1548) = TBOff;
		}
	}

}

void PopTheBird()
{
	int DPOnA = 1;
	int DPOnB = 0;
	int DPOffA = 0;
	int DPOffB = 16;

	GetPlayerData();
	GetCharacterIDs();

	if (P1Character1ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P1Character1Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P1Character1Data + 0x6930) = DPOnA;
			*(int*)(P1Character1Data + 0x1548) = DPOnA;
			*(int*)(P1Character1Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P1Character1Data + 0x15F0) = EmptyInstall;
			*(int*)(P1Character1Data + 0x6930) = DPOffA;
			*(int*)(P1Character1Data + 0x1548) = DPOffA;
			*(int*)(P1Character1Data + 0x1588) = DPOffB;
		}
	}

	if (P1Character2ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P1Character2Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P1Character2Data + 0x6930) = DPOnA;
			*(int*)(P1Character2Data + 0x1548) = DPOnA;
			*(int*)(P1Character2Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P1Character2Data + 0x15F0) = EmptyInstall;
			*(int*)(P1Character2Data + 0x6930) = DPOffA;
			*(int*)(P1Character2Data + 0x1548) = DPOffA;
			*(int*)(P1Character2Data + 0x1588) = DPOffB;
		}
	}

	if (P1Character3ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P1Character3Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P1Character3Data + 0x6930) = DPOnA;
			*(int*)(P1Character3Data + 0x1548) = DPOnA;
			*(int*)(P1Character3Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P1Character3Data + 0x15F0) = EmptyInstall;
			*(int*)(P1Character3Data + 0x6930) = DPOffA;
			*(int*)(P1Character3Data + 0x1548) = DPOffA;
			*(int*)(P1Character3Data + 0x1588) = DPOffB;
		}
	}

	if (P2Character1ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P2Character1Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P2Character1Data + 0x6930) = DPOnA;
			*(int*)(P2Character1Data + 0x1548) = DPOnA;
			*(int*)(P2Character1Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P2Character1Data + 0x15F0) = EmptyInstall;
			*(int*)(P2Character1Data + 0x6930) = DPOffA;
			*(int*)(P2Character1Data + 0x1548) = DPOffA;
			*(int*)(P2Character1Data + 0x1588) = DPOffB;
		}
	}

	if (P2Character2ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P2Character2Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P2Character2Data + 0x6930) = DPOnA;
			*(int*)(P2Character2Data + 0x1548) = DPOnA;
			*(int*)(P2Character2Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P2Character2Data + 0x15F0) = EmptyInstall;
			*(int*)(P2Character2Data + 0x6930) = DPOffA;
			*(int*)(P2Character2Data + 0x1548) = DPOffA;
			*(int*)(P2Character2Data + 0x1588) = DPOffB;
		}
	}

	if (P2Character3ID == 36)
	{
		if (DarkPhoenix == true)
		{
			*(FighterInstall*)(P2Character3Data + 0x15F0) = DarkPhoenixInstall;
			*(int*)(P2Character3Data + 0x6930) = DPOnA;
			*(int*)(P2Character3Data + 0x1548) = DPOnA;
			*(int*)(P2Character3Data + 0x1588) = DPOnB;

		}
		else
		{
			*(FighterInstall*)(P2Character3Data + 0x15F0) = EmptyInstall;
			*(int*)(P2Character3Data + 0x6930) = DPOffA;
			*(int*)(P2Character3Data + 0x1548) = DPOffA;
			*(int*)(P2Character3Data + 0x1588) = DPOffB;
		}
	}

}

//Adjusts MODOK's level of Understanding.
void ChangeMODOKUnderstanding(int MODOKLOU)
{
	GetPlayerData();
	GetCharacterIDs();

	if (P1Character1ID == 38)
	{

		*((int*)(P1Character1Data + 0x68F0)) = MODOKLOU;
		*((int*)(P1Character1Data + 0x68F4)) = MODOKLOU;
	}

	if (P1Character2ID == 38)
	{
		*((int*)(P1Character2Data + 0x68F0)) = MODOKLOU;
		*((int*)(P1Character2Data + 0x68F4)) = MODOKLOU;
	}

	if (P1Character3ID == 38)
	{
		*((int*)(P1Character3Data + 0x68F0)) = MODOKLOU;
		*((int*)(P1Character3Data + 0x68F4)) = MODOKLOU;
	}

	if (P2Character1ID == 38)
	{
		*((int*)(P2Character1Data + 0x68F0)) = MODOKLOU;
		*((int*)(P2Character1Data + 0x68F4)) = MODOKLOU;
	}

	if (P2Character2ID == 38)
	{
		*((int*)(P2Character2Data + 0x68F0)) = MODOKLOU;
		*((int*)(P2Character2Data + 0x68F4)) = MODOKLOU;
	}

	if (P2Character3ID == 38)
	{
		*((int*)(P2Character3Data + 0x68F0)) = MODOKLOU;
		*((int*)(P2Character3Data + 0x68F4)) = MODOKLOU;
	}

}

//Sets Dormammu Spell Charges.
void SetDormSpellLevels()
{
	GetPlayerData();
	GetCharacterIDs();
	//Sets Both Spell Values.
	if (P1Character1ID == 39)
	{
		*((int*)(P1Character1Data + 0x68F0)) = DormRed;
		*((int*)(P1Character1Data + 0x68F4)) = DormBlue;
	}

	if (P1Character2ID == 39)
	{
		*((int*)(P1Character2Data + 0x68F0)) = DormRed;
		*((int*)(P1Character2Data + 0x68F4)) = DormBlue;
	}

	if (P1Character3ID == 39)
	{
		*((int*)(P1Character3Data + 0x68F0)) = DormRed;
		*((int*)(P1Character3Data + 0x68F4)) = DormBlue;
	}

	if (P2Character1ID == 39)
	{
		*((int*)(P2Character1Data + 0x68F0)) = DormRed;
		*((int*)(P2Character1Data + 0x68F4)) = DormBlue;
	}

	if (P2Character2ID == 39)
	{
		*((int*)(P2Character2Data + 0x68F0)) = DormRed;
		*((int*)(P2Character2Data + 0x68F4)) = DormBlue;
	}

	if (P2Character3ID == 39)
	{
		*((int*)(P2Character3Data + 0x68F0)) = DormRed;
		*((int*)(P2Character3Data + 0x68F4)) = DormBlue;
	}

}

//Sets Infnite Install Time for Several Characters.
void EndlessInstallBoolUpdate()
{
	float flo = 21600.0;
	char FlagA = 0;
	GetMainPointers();
	GetPlayerData();
	GetCharacterIDs();
	//Checks for these characters as these are the ones compatible with this method.
	if (P1Character1ID == 1 || P1Character1ID == 7 || P1Character1ID == 11 || P1Character1ID == 13 || P1Character1ID == 15 || P1Character1ID == 16
		|| P1Character1ID == 17 || P1Character1ID == 20 || P1Character1ID == 21 || P1Character1ID == 22 || P1Character1ID == 23 || P1Character1ID == 24 || P1Character1ID == 28
		|| P1Character1ID == 37 || P1Character1ID == 44)
	{
		FlagA = *((char*)(P1Character1Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P1Character1Data + 0x1600)) = flo;
		}
	}

	//Checks for these characters as these are the ones compatible with this method.
	if (P1Character2ID == 1 || P1Character2ID == 7 || P1Character2ID == 11 || P1Character2ID == 13 || P1Character2ID == 15 || P1Character2ID == 16
		|| P1Character2ID == 17 || P1Character2ID == 20 || P1Character2ID == 21 || P1Character2ID == 22 || P1Character2ID == 23 || P1Character2ID == 24 || P1Character2ID == 28
		|| P1Character2ID == 37 || P1Character2ID == 44)
	{
		FlagA = *((char*)(P1Character2Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P1Character2Data + 0x1600)) = flo;
		}
	}

	//Checks for these characters as these are the ones compatible with this method.
	if (P1Character3ID == 1 || P1Character3ID == 7 || P1Character3ID == 11 || P1Character3ID == 13 || P1Character3ID == 15 || P1Character3ID == 16
		|| P1Character3ID == 17 || P1Character3ID == 20 || P1Character3ID == 21 || P1Character3ID == 22 || P1Character3ID == 23 || P1Character3ID == 24 || P1Character3ID == 28
		|| P1Character3ID == 37 || P1Character3ID == 44)
	{
		FlagA = *((char*)(P1Character3Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P1Character3Data + 0x1600)) = flo;
		}
	}

	//Checks for these characters as these are the ones compatible with this method.
	if (P2Character1ID == 1 || P2Character1ID == 7 || P2Character1ID == 11 || P2Character1ID == 13 || P2Character1ID == 15 || P2Character1ID == 16
		|| P2Character1ID == 17 || P2Character1ID == 20 || P2Character1ID == 21 || P2Character1ID == 22 || P2Character1ID == 23 || P2Character1ID == 24 || P2Character1ID == 28
		|| P2Character1ID == 37 || P2Character1ID == 44)
	{
		FlagA = *((char*)(P2Character1Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P2Character1Data + 0x1600)) = flo;
		}
	}

	//Checks for these characters as these are the ones compatible with this method.
	if (P2Character2ID == 1 || P2Character2ID == 7 || P2Character2ID == 11 || P2Character2ID == 13 || P2Character2ID == 15 || P2Character2ID == 16
		|| P2Character2ID == 17 || P2Character2ID == 20 || P2Character2ID == 21 || P2Character2ID == 22 || P2Character2ID == 23 || P2Character2ID == 24 || P2Character2ID == 28
		|| P2Character2ID == 37 || P2Character2ID == 44)
	{

		FlagA = *((char*)(P2Character2Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P2Character2Data + 0x1600)) = flo;
		}
	}

	//Checks for these characters as these are the ones compatible with this method.
	if (P2Character3ID == 1 || P2Character3ID == 7 || P2Character3ID == 11 || P2Character3ID == 13 || P2Character3ID == 15 || P2Character3ID == 16
		|| P2Character3ID == 17 || P2Character3ID == 20 || P2Character3ID == 21 || P2Character3ID == 22 || P2Character3ID == 23 || P2Character3ID == 24 || P2Character3ID == 28
		|| P2Character3ID == 37 || P2Character3ID == 44)
	{
		FlagA = *((char*)(P2Character3Data + 0x15F8));
		if (FlagA == 17 || FlagA == 16)
		{
			*((float*)(P2Character3Data + 0x1600)) = flo;
		}
	}

}

void SetDeadpoolTeleport()
{
	GetPlayerData();
	GetCharacterIDs();
	if (P1Character1ID == 40)
	{
		*((int*)(P1Character1Data + 0x68F0)) = DeadpoolTeleportCount;

	}

	if (P1Character2ID == 40)
	{
		*((int*)(P1Character2Data + 0x68F0)) = DeadpoolTeleportCount;

	}

	if (P1Character3ID == 40)
	{
		*((int*)(P1Character3Data + 0x68F0)) = DeadpoolTeleportCount;

	}

	if (P2Character1ID == 40)
	{
		*((int*)(P2Character1Data + 0x68F0)) = DeadpoolTeleportCount;

	}

	if (P2Character2ID == 40)
	{
		*((int*)(P2Character2Data + 0x68F0)) = DeadpoolTeleportCount;

	}

	if (P2Character3ID == 40)
	{
		*((int*)(P2Character3Data + 0x68F0)) = DeadpoolTeleportCount;

	}


}

void BreakWeskerGlasses()
{
	GetPlayerData();
	GetCharacterIDs();

	if (P1Character1ID == 5)
	{
		*((float*)(P1Character1Data + 0x6908)) = ZeroHP;

	}
	if (P1Character2ID == 5)
	{

		*((float*)(P1Character2Data + 0x6908)) = ZeroHP;

	}
	if (P1Character3ID == 5)
	{

		*((float*)(P1Character3Data + 0x6908)) = ZeroHP;

	}

	if (P2Character1ID == 5)
	{

		*((float*)(P2Character1Data + 0x6908)) = ZeroHP;

	}
	if (P2Character2ID == 5)
	{

		*((float*)(P2Character2Data + 0x6908)) = ZeroHP;

	}
	if (P2Character3ID == 5)
	{

		*((float*)(P2Character3Data + 0x6908)) = ZeroHP;

	}

}

int CheckInstallSlotA(int InstallSlot, int CharID, FighterInstall install, uint64_t PlayerDataPointer)
{

	if (CharID == 22)
	{


		CheckedID = *((int*)(PlayerDataPointer + 0x15F8));
		CheckedMType = *((int*)(PlayerDataPointer + 0x15FC));

		if ((CheckedID == 0 && CheckedMType == 0) || (CheckedID == 0 && CheckedMType == 34))
		{
			return 1;
		}



		CheckedID = *((int*)(PlayerDataPointer + 0x1678));
		CheckedMType = *((int*)(PlayerDataPointer + 0x167C));

		if ((CheckedID == 0 && CheckedMType == 0) || (CheckedID == 0 && CheckedMType == 34))
		{
			return 2;
		}


		CheckedID = *((int*)(PlayerDataPointer + 0x16F8));
		CheckedMType = *((int*)(PlayerDataPointer + 0x16FC));

		if ((CheckedID == 0 && CheckedMType == 0) || (CheckedID == 0 && CheckedMType == 34))
		{
			return 3;
		}


		CheckedID = *((int*)(PlayerDataPointer + 0x1778));
		CheckedMType = *((int*)(PlayerDataPointer + 0x177C));

		if ((CheckedID == 0 && CheckedMType == 0) || (CheckedID == 0 && CheckedMType == 34))
		{
			return 4;
		}


		CheckedID = *((int*)(PlayerDataPointer + 0x17F8));
		CheckedMType = *((int*)(PlayerDataPointer + 0x17FC));

		if ((CheckedID == 0 && CheckedMType == 0) || (CheckedID == 0 && CheckedMType == 34))
		{
			return 5;
		}
	}

	return -1;


}

void VergilSpiralSwords()
{
	int8_t SSOn = 1;
	int8_t SSOff = 0;
	GetPlayerData();
	GetCharacterIDs();



	if (P1Character1ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P1Character1Data))

			{
			case 1:

				*((FighterInstall*)(P1Character1Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P1Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P1Character1Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P1Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P1Character1Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P1Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P1Character1Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P1Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P1Character1Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P1Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character1Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P1Character1Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P1Character1Data + 0x68f0)) = SSOff;
			*((int8_t*)(P1Character1Data + 0x68f1)) = SSOff;
			*((int8_t*)(P1Character1Data + 0x68f2)) = SSOff;
			*((int8_t*)(P1Character1Data + 0x68f3)) = SSOff;
			*((int8_t*)(P1Character1Data + 0x68f4)) = SSOff;
			*((int8_t*)(P1Character1Data + 0x68f5)) = SSOff;

		}
	}

	if (P1Character2ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P1Character2Data))

			{
			case 1:

				*((FighterInstall*)(P1Character2Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P1Character2Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P1Character2Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P1Character2Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P1Character2Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P1Character2Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P1Character2Data + 0x68f0)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f1)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f2)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f3)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f4)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f5)) = SSOff;

		}
	}

	if (P1Character3ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P1Character3Data))

			{
			case 1:

				*((FighterInstall*)(P1Character3Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P1Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P1Character3Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P1Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P1Character3Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P1Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P1Character3Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P1Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P1Character3Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P1Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character3Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P1Character3Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P1Character3Data + 0x68f0)) = SSOff;
			*((int8_t*)(P1Character3Data + 0x68f1)) = SSOff;
			*((int8_t*)(P1Character3Data + 0x68f2)) = SSOff;
			*((int8_t*)(P1Character3Data + 0x68f3)) = SSOff;
			*((int8_t*)(P1Character3Data + 0x68f4)) = SSOff;
			*((int8_t*)(P1Character3Data + 0x68f5)) = SSOff;

		}
	}

	if (P2Character1ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P2Character1Data))

			{
			case 1:

				*((FighterInstall*)(P2Character1Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P2Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P2Character1Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P2Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P2Character1Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P2Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P2Character1Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P2Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P2Character1Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P2Character1Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character1Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P2Character1Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P2Character1Data + 0x68f0)) = SSOff;
			*((int8_t*)(P2Character1Data + 0x68f1)) = SSOff;
			*((int8_t*)(P2Character1Data + 0x68f2)) = SSOff;
			*((int8_t*)(P2Character1Data + 0x68f3)) = SSOff;
			*((int8_t*)(P2Character1Data + 0x68f4)) = SSOff;
			*((int8_t*)(P2Character1Data + 0x68f5)) = SSOff;

		}
	}

	if (P2Character2ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P1Character2Data))

			{
			case 1:

				*((FighterInstall*)(P1Character2Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P1Character2Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P1Character2Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P1Character2Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P1Character2Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P1Character2Data + 0x68f0)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f1)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f2)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f3)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f4)) = SSOn;
				*((int8_t*)(P1Character2Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P1Character2Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P1Character2Data + 0x68f0)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f1)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f2)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f3)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f4)) = SSOff;
			*((int8_t*)(P1Character2Data + 0x68f5)) = SSOff;

		}
	}

	if (P2Character3ID == 22)
	{
		if (VergilSpiralSwordsForever == true)
		{
			//Check if Install slot has Swords already.
			switch (CheckInstallSlotA(1, 22, VergilSwords, P2Character3Data))

			{
			case 1:

				*((FighterInstall*)(P2Character3Data + 0x15F0)) = VergilSwords;
				*((int8_t*)(P2Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f5)) = SSOn;

				break;

			case 2:

				*((FighterInstall*)(P2Character3Data + 0x1670)) = VergilSwords;
				*((int8_t*)(P2Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f5)) = SSOn;

				break;

			case 3:
				*((FighterInstall*)(P2Character3Data + 0x16F0)) = VergilSwords;
				*((int8_t*)(P2Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f5)) = SSOn;
				break;

			case 4:
				*((FighterInstall*)(P2Character3Data + 0x1770)) = VergilSwords;
				*((int8_t*)(P2Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f5)) = SSOn;
				break;

			case 5:
				*((FighterInstall*)(P2Character3Data + 0x17F0)) = VergilSwords;
				*((int8_t*)(P2Character3Data + 0x68f0)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f1)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f2)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f3)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f4)) = SSOn;
				*((int8_t*)(P2Character3Data + 0x68f5)) = SSOn;
				break;

			default:
				break;

			}
		}


		else
		{
			*((FighterInstall*)(P2Character3Data + 0x1670)) = EmptyInstall;
			*((int8_t*)(P2Character3Data + 0x68f0)) = SSOff;
			*((int8_t*)(P2Character3Data + 0x68f1)) = SSOff;
			*((int8_t*)(P2Character3Data + 0x68f2)) = SSOff;
			*((int8_t*)(P2Character3Data + 0x68f3)) = SSOff;
			*((int8_t*)(P2Character3Data + 0x68f4)) = SSOff;
			*((int8_t*)(P2Character3Data + 0x68f5)) = SSOff;

		}
	}



}

void TurnOnTheSwords() {}

//Sets whether or not the specified characters are hit by Jamming Bomb.
void JammingToggle()
{
	GetPlayerData();

	if (P1C1Jammed == true)
	{

		if (P1C1Slot2Free == true && !P1C1JammedAlready)
		{
			P1C1JammedSlot = 2;
			P1C1JammedAlready = true;
			*((FighterInstall*)(P1Character1Data + 0x15F0)) = Jammed;
		}
		else if (P1C1Slot3Free == true && !P1C1JammedAlready)
		{
			P1C1JammedSlot = 3;
			P1C1JammedAlready = true;
			*((FighterInstall*)(P1Character1Data + 0x1670)) = Jammed;
		}
		else if (P1C1Slot4Free == true && !P1C1JammedAlready)
		{
			P1C1JammedAlready = true;
			P1C1JammedSlot = 4;
			*((FighterInstall*)(P1Character1Data + 0x16F0)) = Jammed;
		}
		else if (P1C1Slot5Free == true && !P1C1JammedAlready)
		{
			P1C1JammedAlready = true;
			P1C1JammedSlot = 5;
			*((FighterInstall*)(P1Character1Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P1C1JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P1Character1Data + 0x1770)) = EmptyInstall;
			P1C1JammedSlot = 0;
			P1C1JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P1Character1Data + 0x16F0)) = EmptyInstall;
			P1C1JammedSlot = 0;
			P1C1JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P1Character1Data + 0x1670)) = EmptyInstall;
			P1C1JammedSlot = 0;
			P1C1JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P1Character1Data + 0x15F0)) = EmptyInstall;
			P1C1JammedSlot = 0;
			P1C1JammedAlready = false;
			break;
		default:
			P1C1JammedSlot = 0;
			P1C1JammedAlready = false;
			break;
		}
	}

	if (P1C2Jammed == true)
	{

		if (P1C2Slot2Free == true && !P1C2JammedAlready)
		{
			P1C2JammedSlot = 2;
			P1C2JammedAlready = true;
			*((FighterInstall*)(P1Character2Data + 0x15F0)) = Jammed;
		}
		else if (P1C2Slot3Free == true && !P1C2JammedAlready)
		{
			P1C2JammedSlot = 3;
			P1C2JammedAlready = true;
			*((FighterInstall*)(P1Character2Data + 0x1670)) = Jammed;
		}
		else if (P1C2Slot4Free == true && !P1C2JammedAlready)
		{
			P1C2JammedAlready = true;
			P1C2JammedSlot = 4;
			*((FighterInstall*)(P1Character2Data + 0x16F0)) = Jammed;
		}
		else if (P1C2Slot5Free == true && !P1C2JammedAlready)
		{
			P1C2JammedAlready = true;
			P1C2JammedSlot = 5;
			*((FighterInstall*)(P1Character2Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P1C2JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P1Character2Data + 0x1770)) = EmptyInstall;
			P1C2JammedSlot = 0;
			P1C2JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P1Character2Data + 0x16F0)) = EmptyInstall;
			P1C2JammedSlot = 0;
			P1C2JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P1Character2Data + 0x1670)) = EmptyInstall;
			P1C2JammedSlot = 0;
			P1C2JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P1Character2Data + 0x15F0)) = EmptyInstall;
			P1C2JammedSlot = 0;
			P1C2JammedAlready = false;
			break;
		default:
			P1C2JammedSlot = 0;
			P1C2JammedAlready = false;
			break;
		}
	}

	if (P1C3Jammed == true)
	{

		if (P1C3Slot2Free == true && !P1C3JammedAlready)
		{
			P1C3JammedSlot = 2;
			P1C3JammedAlready = true;
			*((FighterInstall*)(P1Character3Data + 0x15F0)) = Jammed;
		}
		else if (P1C3Slot3Free == true && !P1C3JammedAlready)
		{
			P1C3JammedSlot = 3;
			P1C3JammedAlready = true;
			*((FighterInstall*)(P1Character3Data + 0x1670)) = Jammed;
		}
		else if (P1C3Slot4Free == true && !P1C3JammedAlready)
		{
			P1C3JammedAlready = true;
			P1C3JammedSlot = 4;
			*((FighterInstall*)(P1Character3Data + 0x16F0)) = Jammed;
		}
		else if (P1C3Slot5Free == true && !P1C3JammedAlready)
		{
			P1C3JammedAlready = true;
			P1C3JammedSlot = 5;
			*((FighterInstall*)(P1Character3Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P1C3JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P1Character3Data + 0x1770)) = EmptyInstall;
			P1C3JammedSlot = 0;
			P1C3JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P1Character3Data + 0x16F0)) = EmptyInstall;
			P1C3JammedSlot = 0;
			P1C3JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P1Character3Data + 0x1670)) = EmptyInstall;
			P1C3JammedSlot = 0;
			P1C3JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P1Character3Data + 0x15F0)) = EmptyInstall;
			P1C3JammedSlot = 0;
			P1C3JammedAlready = false;
			break;
		default:
			P1C3JammedSlot = 0;
			P1C3JammedAlready = false;
			break;
		}
	}

	if (P2C1Jammed == true)
	{

		if (P2C1Slot2Free == true && !P2C1JammedAlready)
		{
			P2C1JammedSlot = 2;
			P2C1JammedAlready = true;
			*((FighterInstall*)(P2Character1Data + 0x15F0)) = Jammed;
		}
		else if (P2C1Slot3Free == true && !P2C1JammedAlready)
		{
			P2C1JammedSlot = 3;
			P2C1JammedAlready = true;
			*((FighterInstall*)(P2Character1Data + 0x1670)) = Jammed;
		}
		else if (P2C1Slot4Free == true && !P2C1JammedAlready)
		{
			P2C1JammedAlready = true;
			P2C1JammedSlot = 4;
			*((FighterInstall*)(P2Character1Data + 0x16F0)) = Jammed;
		}
		else if (P2C1Slot5Free == true && !P2C1JammedAlready)
		{
			P2C1JammedAlready = true;
			P2C1JammedSlot = 5;
			*((FighterInstall*)(P2Character1Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P2C1JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P2Character1Data + 0x1770)) = EmptyInstall;
			P2C1JammedSlot = 0;
			P2C1JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P2Character1Data + 0x16F0)) = EmptyInstall;
			P2C1JammedSlot = 0;
			P2C1JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P2Character1Data + 0x1670)) = EmptyInstall;
			P2C1JammedSlot = 0;
			P2C1JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P2Character1Data + 0x15F0)) = EmptyInstall;
			P2C1JammedSlot = 0;
			P2C1JammedAlready = false;
			break;
		default:
			P2C1JammedSlot = 0;
			P2C1JammedAlready = false;
			break;
		}
	}

	if (P2C2Jammed == true)
	{

		if (P2C2Slot2Free == true && !P2C2JammedAlready)
		{
			P2C2JammedSlot = 2;
			P2C2JammedAlready = true;
			*((FighterInstall*)(P2Character2Data + 0x15F0)) = Jammed;
		}
		else if (P2C2Slot3Free == true && !P2C2JammedAlready)
		{
			P2C2JammedSlot = 3;
			P2C2JammedAlready = true;
			*((FighterInstall*)(P2Character2Data + 0x1670)) = Jammed;
		}
		else if (P2C2Slot4Free == true && !P2C2JammedAlready)
		{
			P2C2JammedAlready = true;
			P2C2JammedSlot = 4;
			*((FighterInstall*)(P2Character2Data + 0x16F0)) = Jammed;
		}
		else if (P2C2Slot5Free == true && !P2C2JammedAlready)
		{
			P2C2JammedAlready = true;
			P2C2JammedSlot = 5;
			*((FighterInstall*)(P2Character2Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P2C2JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P2Character2Data + 0x1770)) = EmptyInstall;
			P2C2JammedSlot = 0;
			P2C2JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P2Character2Data + 0x16F0)) = EmptyInstall;
			P2C2JammedSlot = 0;
			P2C2JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P2Character2Data + 0x1670)) = EmptyInstall;
			P2C2JammedSlot = 0;
			P2C2JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P2Character2Data + 0x15F0)) = EmptyInstall;
			P2C2JammedSlot = 0;
			P2C2JammedAlready = false;
			break;
		default:
			P2C2JammedSlot = 0;
			P2C2JammedAlready = false;
			break;
		}
	}

	if (P2C3Jammed == true)
	{

		if (P2C3Slot2Free == true && !P2C3JammedAlready)
		{
			P2C3JammedSlot = 2;
			P2C3JammedAlready = true;
			*((FighterInstall*)(P2Character3Data + 0x15F0)) = Jammed;
		}
		else if (P2C3Slot3Free == true && !P2C3JammedAlready)
		{
			P2C3JammedSlot = 3;
			P2C3JammedAlready = true;
			*((FighterInstall*)(P2Character3Data + 0x1670)) = Jammed;
		}
		else if (P2C3Slot4Free == true && !P2C3JammedAlready)
		{
			P2C3JammedAlready = true;
			P2C3JammedSlot = 4;
			*((FighterInstall*)(P2Character3Data + 0x16F0)) = Jammed;
		}
		else if (P2C3Slot5Free == true && !P2C3JammedAlready)
		{
			P2C3JammedAlready = true;
			P2C3JammedSlot = 5;
			*((FighterInstall*)(P2Character3Data + 0x1770)) = Jammed;
		}

	}
	else
	{
		switch (P2C3JammedSlot)
		{
		case 5:
			*((FighterInstall*)(P2Character3Data + 0x1770)) = EmptyInstall;
			P2C3JammedSlot = 0;
			P2C3JammedAlready = false;
			break;
		case 4:
			*((FighterInstall*)(P2Character3Data + 0x16F0)) = EmptyInstall;
			P2C3JammedSlot = 0;
			P2C3JammedAlready = false;
			break;
		case 3:
			*((FighterInstall*)(P2Character3Data + 0x1670)) = EmptyInstall;
			P2C3JammedSlot = 0;
			P2C3JammedAlready = false;
			break;
		case 2:
			*((FighterInstall*)(P2Character3Data + 0x15F0)) = EmptyInstall;
			P2C3JammedSlot = 0;
			P2C3JammedAlready = false;
			break;
		default:
			P2C3JammedSlot = 0;
			P2C3JammedAlready = false;
			break;
		}
	}

}

void JammingToggleRestart()
{

	if (P1C1Jammed == true)
	{
		switch (P1C1JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P1Character1Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P1Character1Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P1Character1Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P1Character1Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

	if (P1C2Jammed == true)
	{
		switch (P1C2JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P1Character2Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P1Character2Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P1Character2Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P1Character2Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

	if (P1C3Jammed == true)
	{
		switch (P1C3JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P1Character3Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P1Character3Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P1Character3Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P1Character3Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

	if (P2C1Jammed == true)
	{
		switch (P2C1JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P2Character1Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P2Character1Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P2Character1Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P2Character1Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

	if (P2C2Jammed == true)
	{
		switch (P2C2JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P2Character2Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P2Character2Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P2Character2Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P2Character2Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

	if (P2C3Jammed == true)
	{
		switch (P2C3JammedSlot)
		{
		case 2:
			*((FighterInstall*)(P2Character3Data + 0x15F0)) = Jammed;
			break;
		case 3:
			*((FighterInstall*)(P2Character3Data + 0x1670)) = Jammed;
			break;
		case 4:
			*((FighterInstall*)(P2Character3Data + 0x16F0)) = Jammed;
			break;
		case 5:
			*((FighterInstall*)(P2Character3Data + 0x1770)) = Jammed;
			break;
		default:
			break;
		}
	}

}

void SetGlobalPlayerSpeed(float GlobalSpeed)
{

	GetPlayerData();
	if (P1Character1Data && P2Character1Data) {
		*((float*)(P1Character1Data + 0x15B4)) = P1Char1Speed;
		*((float*)(P1Character1Data + 0x15B4)) = P1Char1Speed;
		*((float*)(P1Character1Data + 0x15B4)) = P1Char1Speed;

		*((float*)(P2Character1Data + 0x15B4)) = P2Char1Speed;
		*((float*)(P2Character1Data + 0x15B4)) = P2Char1Speed;
		*((float*)(P2Character1Data + 0x15B4)) = P2Char1Speed;
	}

}

void SetGameSpeed(float GameSpeed)
{
	GetMainPointers();
	*((float*)(PointerToGameSpeedA + 0x8c)) = GameSpeed;
}

void ResetGameSpeed()
{


	GameSpeed = 1.0;
	GetMainPointers();
	if (PointerToGameSpeedA)
	{
		*((float*)(PointerToGameSpeedA + 0x8c)) = GameSpeed;
	}
}

void GetActiveInstallData()
{
	if (P1Character1Data) {
		P1C1InstallID1 = *((int*)((P1Character1Data + 0x1578)));
		P1C1InstallID2 = *((int*)(P1Character1Data + 0x15F8));
		P1C1InstallID3 = *((int*)(P1Character1Data + 0x1678));
		P1C1InstallID4 = *((int*)(P1Character1Data + 0x16F8));
		P1C1InstallID5 = *((int*)(P1Character1Data + 0x1778));
		P1C1InstallType1 = *((int*)(P1Character1Data + 0x157C));
		P1C1InstallType2 = *((int*)(P1Character1Data + 0x15FC));
		P1C1InstallType3 = *((int*)(P1Character1Data + 0x167C));
		P1C1InstallType4 = *((int*)(P1Character1Data + 0x16FC));
		P1C1InstallType5 = *((int*)(P1Character1Data + 0x177C));

		P1C2InstallID1 = *((int*)(P1Character2Data + 0x1578));
		P1C2InstallID2 = *((int*)(P1Character2Data + 0x15F8));
		P1C2InstallID3 = *((int*)(P1Character2Data + 0x1678));
		P1C2InstallID4 = *((int*)(P1Character2Data + 0x16F8));
		P1C2InstallID5 = *((int*)(P1Character2Data + 0x1778));
		P1C2InstallType1 = *((int*)(P1Character2Data + 0x157C));
		P1C2InstallType2 = *((int*)(P1Character2Data + 0x15FC));
		P1C2InstallType3 = *((int*)(P1Character2Data + 0x167C));
		P1C2InstallType4 = *((int*)(P1Character2Data + 0x16FC));
		P1C2InstallType5 = *((int*)(P1Character2Data + 0x177C));

		P1C3InstallID1 = *((int*)(P1Character3Data + 0x1578));
		P1C3InstallID2 = *((int*)(P1Character3Data + 0x15F8));
		P1C3InstallID3 = *((int*)(P1Character3Data + 0x1678));
		P1C3InstallID4 = *((int*)(P1Character3Data + 0x16F8));
		P1C3InstallID5 = *((int*)(P1Character3Data + 0x1778));
		P1C3InstallType1 = *((int*)(P1Character3Data + 0x157C));
		P1C3InstallType2 = *((int*)(P1Character3Data + 0x15FC));
		P1C3InstallType3 = *((int*)(P1Character3Data + 0x167C));
		P1C3InstallType4 = *((int*)(P1Character3Data + 0x16FC));
		P1C3InstallType5 = *((int*)(P1Character3Data + 0x177C));

		P2C1InstallID1 = *((int*)(P2Character1Data + 0x1578));
		P2C1InstallID2 = *((int*)(P2Character1Data + 0x15F8));
		P2C1InstallID3 = *((int*)(P2Character1Data + 0x1678));
		P2C1InstallID4 = *((int*)(P2Character1Data + 0x16F8));
		P2C1InstallID5 = *((int*)(P2Character1Data + 0x1778));
		P2C1InstallType1 = *((int*)(P2Character1Data + 0x157C));
		P2C1InstallType2 = *((int*)(P2Character1Data + 0x15FC));
		P2C1InstallType3 = *((int*)(P2Character1Data + 0x167C));
		P2C1InstallType4 = *((int*)(P2Character1Data + 0x16FC));
		P2C1InstallType5 = *((int*)(P2Character1Data + 0x177C));

		P2C2InstallID1 = *((int*)(P2Character2Data + 0x1578));
		P2C2InstallID2 = *((int*)(P2Character2Data + 0x15F8));
		P2C2InstallID3 = *((int*)(P2Character2Data + 0x1678));
		P2C2InstallID4 = *((int*)(P2Character2Data + 0x16F8));
		P2C2InstallID5 = *((int*)(P2Character2Data + 0x1778));
		P2C2InstallType1 = *((int*)(P2Character2Data + 0x157C));
		P2C2InstallType2 = *((int*)(P2Character2Data + 0x15FC));
		P2C2InstallType3 = *((int*)(P2Character2Data + 0x167C));
		P2C2InstallType4 = *((int*)(P2Character2Data + 0x16FC));
		P2C2InstallType5 = *((int*)(P2Character2Data + 0x177C));

		P2C3InstallID1 = *((int*)(P2Character3Data + 0x1578));
		P2C3InstallID2 = *((int*)(P2Character3Data + 0x15F8));
		P2C3InstallID3 = *((int*)(P2Character3Data + 0x1678));
		P2C3InstallID4 = *((int*)(P2Character3Data + 0x16F8));
		P2C3InstallID5 = *((int*)(P2Character3Data + 0x1778));
		P2C3InstallType1 = *((int*)(P2Character3Data + 0x157C));
		P2C3InstallType2 = *((int*)(P2Character3Data + 0x15FC));
		P2C3InstallType3 = *((int*)(P2Character3Data + 0x167C));
		P2C3InstallType4 = *((int*)(P2Character3Data + 0x16FC));
		P2C3InstallType5 = *((int*)(P2Character3Data + 0x177C));

		if (P1C1InstallID2 == 0 && P1C1InstallType2 == 0)
		{
			P1C1Slot2Free = true;
		}
		else
		{
			P1C1Slot2Free = false;
		}

		if (P1C1InstallID3 == 0 && P1C1InstallType3 == 0)
		{
			P1C1Slot3Free = true;
		}
		else
		{
			P1C1Slot3Free = false;
		}

		if (P1C1InstallID4 == 0 && P1C1InstallType4 == 0)
		{
			P1C1Slot4Free = true;
		}
		else
		{
			P1C1Slot4Free = false;
		}

		if (P1C1InstallID5 == 0 && P1C1InstallType5 == 0)
		{
			P1C1Slot5Free = true;
		}
		else
		{
			P1C1Slot5Free = false;
		}

		if (P1C2InstallID2 == 0 && P1C2InstallType2 == 0)
		{
			P1C2Slot2Free = true;
		}
		else
		{
			P1C2Slot2Free = false;
		}

		if (P1C2InstallID3 == 0 && P1C2InstallType3 == 0)
		{
			P1C2Slot3Free = true;
		}
		else
		{
			P1C2Slot3Free = false;
		}

		if (P1C2InstallID4 == 0 && P1C2InstallType4 == 0)
		{
			P1C2Slot4Free = true;
		}
		else
		{
			P1C2Slot4Free = false;
		}

		if (P1C2InstallID5 == 0 && P1C2InstallType5 == 0)
		{
			P1C2Slot5Free = true;
		}
		else
		{
			P1C2Slot5Free = false;
		}

		if (P1C3InstallID2 == 0 && P1C3InstallType2 == 0)
		{
			P1C3Slot2Free = true;
		}
		else
		{
			P1C3Slot2Free = false;
		}

		if (P1C3InstallID3 == 0 && P1C3InstallType3 == 0)
		{
			P1C3Slot3Free = true;
		}
		else
		{
			P1C3Slot3Free = false;
		}

		if (P1C3InstallID4 == 0 && P1C3InstallType4 == 0)
		{
			P1C3Slot4Free = true;
		}
		else
		{
			P1C3Slot4Free = false;
		}

		if (P1C3InstallID5 == 0 && P1C3InstallType5 == 0)
		{
			P1C3Slot5Free = true;
		}
		else
		{
			P1C3Slot5Free = false;
		}

		if (P2C1InstallID2 == 0 && P2C1InstallType2 == 0)
		{
			P2C1Slot2Free = true;
		}
		else
		{
			P2C1Slot2Free = false;
		}

		if (P2C1InstallID3 == 0 && P2C1InstallType3 == 0)
		{
			P2C1Slot3Free = true;
		}
		else
		{
			P2C1Slot3Free = false;
		}

		if (P2C1InstallID4 == 0 && P2C1InstallType4 == 0)
		{
			P2C1Slot4Free = true;
		}
		else
		{
			P2C1Slot4Free = false;
		}

		if (P2C1InstallID5 == 0 && P2C1InstallType5 == 0)
		{
			P2C1Slot5Free = true;
		}
		else
		{
			P2C1Slot5Free = false;
		}

		if (P2C2InstallID2 == 0 && P2C2InstallType2 == 0)
		{
			P2C2Slot2Free = true;
		}
		else
		{
			P2C2Slot2Free = false;
		}

		if (P2C2InstallID3 == 0 && P2C2InstallType3 == 0)
		{
			P2C2Slot3Free = true;
		}
		else
		{
			P2C2Slot3Free = false;
		}

		if (P2C2InstallID4 == 0 && P2C2InstallType4 == 0)
		{
			P2C2Slot4Free = true;
		}
		else
		{
			P2C2Slot4Free = false;
		}

		if (P2C2InstallID5 == 0 && P2C2InstallType5 == 0)
		{
			P2C2Slot5Free = true;
		}
		else
		{
			P2C2Slot5Free = false;
		}

		if (P2C3InstallID2 == 0 && P2C3InstallType2 == 0)
		{
			P2C3Slot2Free = true;
		}
		else
		{
			P2C3Slot2Free = false;
		}

		if (P2C3InstallID3 == 0 && P2C3InstallType3 == 0)
		{
			P2C3Slot3Free = true;
		}
		else
		{
			P2C3Slot3Free = false;
		}

		if (P2C3InstallID4 == 0 && P2C3InstallType4 == 0)
		{
			P2C3Slot4Free = true;
		}
		else
		{
			P2C3Slot4Free = false;
		}

		if (P2C3InstallID5 == 0 && P2C3InstallType5 == 0)
		{
			P2C3Slot5Free = true;
		}
		else
		{
			P2C3Slot5Free = false;
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

			shotstuff.PeculiarCounter = *(int*)_addr(TempU + 0x2054);
			shotstuff.FirstCollPtr = *(uint64_t*)_addr(TempU + 0x2058);
			if (shotstuff.FirstCollPtr)
			{
				TempV = *(uint64_t*)_addr(shotstuff.FirstCollPtr + 0x30);
				if(TempV)
				{
					//This is where the while loop should be.
					while(*(uint64_t*)_addr(TempV) != 0x0 && *(uint64_t*)_addr(TempV) != 0xFFFFFFFF)
					{						
						ContainerPtr = *(uint64_t*)_addr(TempV);
						if (ContainerPtr)
						{
							CollisionPtr = *(uint64_t*)_addr(ContainerPtr + 0x10);
							//Checks if the pointer actually points to somewhere.
							if (CollisionPtr)
							{

								//Checks if the duration is zero before considering adding those.
								if(shotstuff.CurrentShotDuration != 0)								
								{
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
								}


							}



						}
						TempV = TempV + 0x08;
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


							}



						}
						TempV = TempV + 0x08;
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
		return "uShotVulcanJigenZan";// AKA Judgment Cut.

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

void RemoveAllInstalls()
{

	//Player 1 Character 1.
	*((FighterInstall*)(P1Character1Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character1Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P1Character1Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character1Data + 0x1770)) = EmptyInstall;

	*((FighterInstall*)(P1Character2Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character2Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P1Character2Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character2Data + 0x1770)) = EmptyInstall;

	*((FighterInstall*)(P1Character3Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character3Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P1Character3Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P1Character3Data + 0x1770)) = EmptyInstall;

	*((FighterInstall*)(P2Character1Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character1Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P2Character1Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character1Data + 0x1770)) = EmptyInstall;

	*((FighterInstall*)(P2Character2Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character2Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P2Character2Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character2Data + 0x1770)) = EmptyInstall;

	*((FighterInstall*)(P2Character3Data + 0x15F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character3Data + 0x1670)) = EmptyInstall;
	*((FighterInstall*)(P2Character3Data + 0x16F0)) = EmptyInstall;
	*((FighterInstall*)(P2Character3Data + 0x1770)) = EmptyInstall;


}

bool CheckIfDead(uint64_t PlayerPtr)
{

	uint64_t YHP = 0;
	uint64_t RHP = 0;

	YHP = *(float*)(PlayerPtr + 0x1550);
	RHP = *(float*)(PlayerPtr + 0x1558);

	if (YHP == 0 && RHP == 0) { return true; }
	else { return false; }
}

static void ForJean(uint64_t PlayerPtr)
{
	int DPOnA = 1;
	int DPOnB = 0;

	*(FighterInstall*)(P1Character1Data + 0x15F0) = DarkPhoenixInstall;
	*(int*)(P1Character1Data + 0x6930) = DPOnA;
	*(int*)(P1Character1Data + 0x1548) = DPOnA;
	*(int*)(P1Character1Data + 0x1588) = DPOnB;

	Sleep(16.67);
}

void TheReapersScythe(uint64_t PlayerPtr)
{
	int FlagA = 89;
	int FlagB = 0;
	int FlagC = 1050672;
	int FlagD = 85;
	int FlagDGr = 0x50;
	int FlagE = 87;
	int FlagEGr = 0x42;
	int FlagF = 131104;//0x20020 AIR
	int FlagFGr = 131073;//0x20020 Ground
	int FlagG = 73728;
	int FlagGGr = 65536;
	int FlagH = 525313;
	int FlagHGr = 81922;
	int FlagI = 131104;
	int FlagJ = 73404422;
	int FlagJGr = 100663298;

	char PrevAGState = 0;
	char PrevTagState = 0;
	//Air/Ground State.

	PrevAGState = *(char*)(PlayerPtr + 0x14F4);
	PrevTagState = *(char*)(PlayerPtr + 0x14F0);

	if (PrevAGState == 20)
	{

		*(int*)(PlayerPtr + 0x14F4) = FlagF;

		//Something Part 1.
		*(int*)(PlayerPtr + 0x1504) = FlagG;

		//Something Part 2.
		*(int*)(PlayerPtr + 0x1508) = FlagH;

		Sleep(15);

		//Sets Health and Red Health to 0.
		*(int*)(PlayerPtr + 0x1550) = FlagB;
		*(int*)(PlayerPtr + 0x1558) = FlagB;

		//Action/Anm thing.
		*(int*)(PlayerPtr + 0x1310) = FlagD;

		//if (!WriteProcessMemory(hProcess, (LPVOID*)(PlayerPtr + 0x1314), &FlagE, sizeof(int), NULL))
		*(int*)(PlayerPtr + 0x1314) = FlagE;

		Sleep(15);

		//TagState Flag Thing.
		*(int*)(PlayerPtr + 0x14F0) = FlagC;

		//I dunno what this is.
		*(int*)(PlayerPtr + 0x14F8) = FlagJ;
	}
	else
	{
		*(int*)(PlayerPtr + 0x14F4) = FlagFGr;

		//Something Part 1.
		*(int*)(PlayerPtr + 0x1504) = FlagGGr;

		//Something Part 2.
		*(int*)(PlayerPtr + 0x1508) = FlagHGr;


		Sleep(15);

		//Sets Health and Red Health to 0.
		*(int*)(PlayerPtr + 0x1550) = FlagB;
		*(int*)(PlayerPtr + 0x1558) = FlagB;

		//Action/Anm thing.
		*(int*)(PlayerPtr + 0x1310) = FlagDGr;
		*(int*)(PlayerPtr + 0x1314) = FlagEGr;

		Sleep(15);

		if (PrevTagState == 01) {
			//TagState Flag Thing.
			*(int*)(PlayerPtr + 0x14F0) = FlagC;

			//I dunno what this is.
			*(int*)(PlayerPtr + 0x14F8) = FlagJGr;
		}
		else
		{
			//TagState Flag Thing.
			*(int*)(PlayerPtr + 0x14F0) = FlagC;

			//I dunno what this is.
			*(int*)(PlayerPtr + 0x14F8) = FlagJ;

		}
	}

}

void TheKillingFloor()
{
	GetPlayerData();
	int FlagA = 89;
	int FlagB = 0;
	int FlagC = 1050672;
	int FlagD = 85;
	int FlagE = 87;
	int FlagF = 131104;
	int FlagG = 73728;
	int FlagH = 525313;
	int FlagI = 131104;
	int FlagJ = 73404422;
	//char FlagK = 131104;

	if (DeathSiteY > 0)
	{
		if (KOActiveCharacterAtStart)
		{
			//Checks for Jean and activates Dark Phoenix before KOing.
			GetCharacterIDs();
			if (P1Character1ID == 36)
			{

				int DPOnA = 1;
				int DPOnB = 0;



				*(FighterInstall*)(P1Character1Data + 0x15F0) = DarkPhoenixInstall;
				*(int*)(P1Character1Data + 0x6930) = DPOnA;
				*(int*)(P1Character1Data + 0x1548) = DPOnA;
				*(int*)(P1Character1Data + 0x1588) = DPOnB;

				Sleep(16.67);
			}


			*(float*)(P1Character1Data + 0x50) = DeathSiteX;
			*(float*)(P1Character1Data + 0x54) = DeathSiteY;

			//Air/Ground State.


			//Something Part 1.


			//Something Part 2.


			*(int*)(P1Character1Data + 0x14F4) = FlagF;
			*(int*)(P1Character1Data + 0x1504) = FlagG;
			*(int*)(P1Character1Data + 0x1508) = FlagH;

			Sleep(15);
			//Sets Health and Red Health to 0.


			*(int*)(P1Character1Data + 0x1550) = FlagB;
			*(int*)(P1Character1Data + 0x1558) = FlagB;

			Sleep(15);

			//Action/Anm thing.

			*(int*)(P1Character1Data + 0x1310) = FlagD;
			*(int*)(P1Character1Data + 0x1314) = FlagE;


			//TagState Flag Thing.

			//I dunno what this is.


			*(int*)(P1Character1Data + 0x14F0) = FlagC;
			*(int*)(P1Character1Data + 0x14F8) = FlagJ;

		}
		if (KOActiveOpponentAtStart)
		{
			//Checks for Jean and activates Dark Phoenix before KOing.
			GetCharacterIDs();
			if (P2Character1ID == 36)
			{

				int DPOnA = 1;
				int DPOnB = 0;


				*(FighterInstall*)(P2Character1Data + 0x15F0) = DarkPhoenixInstall;
				*(int*)(P2Character1Data + 0x6930) = DPOnA;
				*(int*)(P2Character1Data + 0x1548) = DPOnA;
				*(int*)(P2Character1Data + 0x1588) = DPOnB;

				Sleep(16.67);
			}

			//X Coordinate.

			//Y Coordinate.

			*(float*)(P2Character1Data + 0x50) = DeathSiteX;
			*(float*)(P2Character1Data + 0x54) = DeathSiteY;

			//Air/Ground State.


			//Something Part 1.


			//Something Part 2.


			*(int*)(P2Character1Data + 0x14F4) = FlagF;
			*(int*)(P2Character1Data + 0x1504) = FlagG;
			*(int*)(P2Character1Data + 0x1508) = FlagH;

			Sleep(15);
			//Sets Health and Red Health to 0.


			*(int*)(P2Character1Data + 0x1550) = FlagB;
			*(int*)(P2Character1Data + 0x1558) = FlagB;

			Sleep(15);

			//Action/Anm thing.

			*(int*)(P2Character1Data + 0x1310) = FlagD;
			*(int*)(P2Character1Data + 0x1314) = FlagE;


			//TagState Flag Thing.


			//I dunno what this is.


			*(int*)(P2Character1Data + 0x14F0) = FlagC;
			*(int*)(P2Character1Data + 0x14F8) = FlagJ;

		}
	}
	else
	{
		if (KOActiveCharacterAtStart)
		{

			//Checks for Jean and activates Dark Phoenix before KOing.
			GetCharacterIDs();
			if (P1Character1ID == 36)
			{

				int DPOnA = 1;
				int DPOnB = 0;


				*(FighterInstall*)(P1Character1Data + 0x15F0) = DarkPhoenixInstall;
				*(int*)(P1Character1Data + 0x6930) = DPOnA;
				*(int*)(P1Character1Data + 0x1548) = DPOnA;
				*(int*)(P1Character1Data + 0x1588) = DPOnB;
				Sleep(16.67);
			}


			//Sets Health and Red Health to 0.


			*(int*)(P1Character1Data + 0x1550) = FlagB;
			*(int*)(P1Character1Data + 0x1558) = FlagB;

			Sleep(16.67);

			//X Coordinate.


			*(float*)(P1Character1Data + 0x50) = DeathSiteX;

			//Action/Anm thing.


			*(int*)(P1Character1Data + 0x1310) = FlagA;
			*(int*)(P1Character1Data + 0x1314) = FlagA;


			//TagState Flag Thing.


			*(int*)(P1Character1Data + 0x14F0) = FlagC;

		}
		if (KOActiveOpponentAtStart)
		{

			//Checks for Jean and activates Dark Phoenix before KOing.
			GetCharacterIDs();
			if (P2Character1ID == 36)
			{

				int DPOnA = 1;
				int DPOnB = 0;


				*(FighterInstall*)(P2Character1Data + 0x15F0) = DarkPhoenixInstall;
				*(int*)(P2Character1Data + 0x6930) = DPOnA;
				*(int*)(P2Character1Data + 0x1548) = DPOnA;
				*(int*)(P2Character1Data + 0x1588) = DPOnB;
				Sleep(16.67);
			}


			//Sets Health and Red Health to 0.


			*(int*)(P2Character1Data + 0x1550) = FlagB;
			*(int*)(P2Character1Data + 0x1558) = FlagB;

			Sleep(16.67);

			//X Coordinate.


			*(float*)(P2Character1Data + 0x50) = DeathSiteX;

			//Action/Anm thing.


			*(int*)(P2Character1Data + 0x1310) = FlagA;
			*(int*)(P2Character1Data + 0x1314) = FlagA;


			//TagState Flag Thing.


			*(int*)(P2Character1Data + 0x14F0) = FlagC;

		}
	}

}

void DeathDelay()
{



	if (FrameDelayofDeath > 0)
	{
		Sleep(FrameDelayofDeath * 16.67);
		TheKillingFloor();
	}
	else
	{
		TheKillingFloor();
	}



}

void KOToggles()
{
	int FlagB = 0;
	int FlagC = 1050672;
	int FlagD = 85;
	int FlagE = 87;
	int FlagF = 131104;
	int FlagG = 73728;
	int FlagH = 525313;
	int FlagJ = 73404422;

	GetPlayerData();

	if (Player1Character1Dead == true)
	{
		//Checks if character is already dead.
		if (CheckIfDead(P1Character1Data))
		{

		}
		else
		{
			GetCharacterIDs();
			if (P1Character1ID == 36)
			{
				ForJean(P1Character1Data);
			}
			TheReapersScythe(P1Character1Data);
		}


	}
	else {}

	if (Player1Character2Dead == true)
	{
		if (CheckIfDead(P1Character2Data))
		{

		}
		else
		{
			GetCharacterIDs();
			if (P1Character2ID == 36)
			{
				ForJean(P1Character2Data);
			}
			TheReapersScythe(P1Character2Data);
		}
	}
	else {}

	if (Player1Character3Dead == true)
	{
		if (CheckIfDead(P1Character3Data))
		{

		}
		else
		{
			GetCharacterIDs();
			if (P1Character3ID == 36)
			{
				ForJean(P1Character3Data);
			}
			TheReapersScythe(P1Character3Data);
		}
	}
	else {}

	if (Player2Character1Dead == true)
	{
		if (CheckIfDead(P2Character1Data))
		{

		}
		else
		{
			GetCharacterIDs();
			if (P2Character1ID == 36)
			{
				ForJean(P2Character1Data);
			}
			TheReapersScythe(P2Character1Data);
		}
	}
	else {}

	if (Player2Character2Dead == true)
	{
		if (CheckIfDead(P2Character2Data))
		{

		}
		else {
			GetCharacterIDs();
			if (P2Character2ID == 36)
			{
				ForJean(P2Character2Data);
			}
			TheReapersScythe(P2Character2Data);
		}
	}
	else {}

	if (Player2Character3Dead == true)
	{
		if (CheckIfDead(P2Character3Data))
		{

		}
		else {
			GetCharacterIDs();
			if (P2Character3ID == 36)
			{
				ForJean(P2Character3Data);
			}
			TheReapersScythe(P2Character3Data);
		}
	}
	else {}

}

void GetHitstunTimers()
{

	//First get the Hitstun Timers and update the Max values if needed.
	P1C1GroundHitstunTimer = *(float*)(P1Character1Data + 0x63EC);
	P1C1AirHitstunTimer = *(float*)(P1Character1Data + 0x63F0);

	P1C2GroundHitstunTimer = *(float*)(P1Character2Data + 0x63EC);
	P1C2AirHitstunTimer = *(float*)(P1Character2Data + 0x63F0);

	P1C3GroundHitstunTimer = *(float*)(P1Character3Data + 0x63EC);
	P1C3AirHitstunTimer = *(float*)(P1Character3Data + 0x63F0);

	P2C1GroundHitstunTimer = *(float*)(P2Character1Data + 0x63EC);
	P2C1AirHitstunTimer = *(float*)(P2Character1Data + 0x63F0);

	P2C2GroundHitstunTimer = *(float*)(P2Character2Data + 0x63EC);
	P2C2AirHitstunTimer = *(float*)(P2Character2Data + 0x63F0);

	P2C3GroundHitstunTimer = *(float*)(P2Character3Data + 0x63EC);
	P2C2AirHitstunTimer = *(float*)(P2Character3Data + 0x63F0);

	if (P1C1GroundHitstunTimerMAX < P1C1GroundHitstunTimer)P1C1GroundHitstunTimerMAX = P1C1GroundHitstunTimer;
	if (P1C1AirHitstunTimerMAX < P1C1AirHitstunTimer)P1C1AirHitstunTimerMAX = P1C1AirHitstunTimer;

	if (P1C2GroundHitstunTimerMAX < P1C2GroundHitstunTimer)P1C2GroundHitstunTimerMAX = P1C2GroundHitstunTimer;
	if (P1C2AirHitstunTimerMAX < P1C2AirHitstunTimer)P1C2AirHitstunTimerMAX = P1C2AirHitstunTimer;

	if (P1C3GroundHitstunTimerMAX < P1C3GroundHitstunTimer)P1C3GroundHitstunTimerMAX = P1C3GroundHitstunTimer;
	if (P1C3AirHitstunTimerMAX < P1C3AirHitstunTimer)P1C3AirHitstunTimerMAX = P1C3AirHitstunTimer;

	if (P2C1GroundHitstunTimerMAX < P2C1GroundHitstunTimer)P2C1GroundHitstunTimerMAX = P2C1GroundHitstunTimer;
	if (P2C1AirHitstunTimerMAX < P2C1AirHitstunTimer)P2C1AirHitstunTimerMAX = P2C1AirHitstunTimer;

	if (P2C2GroundHitstunTimerMAX < P2C2GroundHitstunTimer)P2C2GroundHitstunTimerMAX = P2C2GroundHitstunTimer;
	if (P2C2AirHitstunTimerMAX < P2C2AirHitstunTimer)P2C2AirHitstunTimerMAX = P2C2AirHitstunTimer;

	if (P2C3GroundHitstunTimerMAX < P2C3GroundHitstunTimer)P2C3GroundHitstunTimerMAX = P2C3GroundHitstunTimer;
	if (P2C3AirHitstunTimerMAX < P2C3AirHitstunTimer)P2C3AirHitstunTimerMAX = P2C3AirHitstunTimer;

	//The Air/Ground States.
	P1C1AirGroundState = *(char*)(P1Character1Data + 0x14F4);
	P1C2AirGroundState = *(char*)(P1Character2Data + 0x14F4);
	P1C3AirGroundState = *(char*)(P1Character3Data + 0x14F4);
	P2C1AirGroundState = *(char*)(P2Character1Data + 0x14F4);
	P2C2AirGroundState = *(char*)(P2Character2Data + 0x14F4);
	P2C3AirGroundState = *(char*)(P2Character3Data + 0x14F4);

	//And then the Remaining hitstun frames (in floating point for some reason).
	P1C1GroundHitstunRelatedFlag = *(int*)(P1Character1Data + 0x4150);
	P1C1AirHitstunRelatedFlag = *(int*)(P1Character1Data + 0x414C);
	P1C1GroundRemaingHitstun = *(float*)(P1Character1Data + 0x4154);
	P1C1RemaingBlockstun = *(float*)(P1Character1Data + 0x4158);
	P1C1AirRemaingHitstun = *(float*)(P1Character1Data + 0x415C);

	P1C2GroundHitstunRelatedFlag = *(int*)(P1Character2Data + 0x4150);
	P1C2AirHitstunRelatedFlag = *(int*)(P1Character2Data + 0x414C);
	P1C2GroundRemaingHitstun = *(float*)(P1Character2Data + 0x4154);
	P1C2RemaingBlockstun = *(float*)(P1Character2Data + 0x4158);
	P1C2AirRemaingHitstun = *(float*)(P1Character2Data + 0x415C);

	P1C3GroundHitstunRelatedFlag = *(int*)(P1Character3Data + 0x4150);
	P1C3AirHitstunRelatedFlag = *(int*)(P1Character3Data + 0x414C);
	P1C3GroundRemaingHitstun = *(float*)(P1Character3Data + 0x4154);
	P1C3RemaingBlockstun = *(float*)(P1Character3Data + 0x4158);
	P1C3AirRemaingHitstun = *(float*)(P1Character3Data + 0x415C);

	P2C1GroundHitstunRelatedFlag = *(int*)(P2Character1Data + 0x4150);
	P2C1AirHitstunRelatedFlag = *(int*)(P2Character1Data + 0x414C);
	P2C1GroundRemaingHitstun = *(float*)(P2Character1Data + 0x4154);
	P2C1RemaingBlockstun = *(float*)(P2Character1Data + 0x4158);
	P2C1AirRemaingHitstun = *(float*)(P2Character1Data + 0x415C);

	P2C2GroundHitstunRelatedFlag = *(int*)(P2Character2Data + 0x4150);
	P2C2AirHitstunRelatedFlag = *(int*)(P2Character2Data + 0x414C);
	P2C2RemaingBlockstun = *(float*)(P2Character2Data + 0x4158);
	P2C2GroundRemaingHitstun = *(float*)(P2Character2Data + 0x4154);
	P2C2AirRemaingHitstun = *(float*)(P2Character2Data + 0x415C);


	P2C3GroundHitstunRelatedFlag = *(int*)(P2Character3Data + 0x4150);
	P2C3AirHitstunRelatedFlag = *(int*)(P2Character3Data + 0x414C);
	P2C3GroundRemaingHitstun = *(float*)(P2Character3Data + 0x4154);
	P2C3RemaingBlockstun = *(float*)(P2Character3Data + 0x4158);
	P2C3AirRemaingHitstun = *(float*)(P2Character3Data + 0x415C);

	//Pushblock timers.
	P1C1PushblockTimer = *(float*)(P1Character1Data + 0x41a8);
	P1C2PushblockTimer = *(float*)(P1Character2Data + 0x41a8);
	P1C3PushblockTimer = *(float*)(P1Character3Data + 0x41a8);
	P2C1PushblockTimer = *(float*)(P2Character1Data + 0x41a8);
	P2C2PushblockTimer = *(float*)(P2Character2Data + 0x41a8);
	P2C3PushblockTimer = *(float*)(P2Character3Data + 0x41a8);

}

void WackyStuff()
{
	uint64_t LayerA = 0;
	uint64_t LayerB = 0;
	//ReadProcessMemory(hProcess, (LPVOID*)0x140E17698, &LayerA, sizeof(uint64_t), 0);

	LayerA = *(uint64_t*)_addr(0x140E17698);

	//ReadProcessMemory(hProcess, (LPVOID*)(LayerA + 0x88), &LayerB, sizeof(uint64_t), 0);
	//ReadProcessMemory(hProcess, (LPVOID*)(LayerB), &DebugPointerOfSorts, sizeof(uint64_t), 0);
	LayerB = *(uint64_t*)_addr(LayerA + 0x88);
	DebugPointerOfSorts = *(uint64_t*)_addr(LayerB);

	if (DebugPointerOfSorts == 0x140b1f450)
	{
		float NoTime = 0;
		*(float*)(LayerB + 0x40) = NoTime;
		//if (!WriteProcessMemory(hProcess, (LPVOID*)(LayerB + 0x40), &NoTime, sizeof(float), NULL))
		//{

		//}
	}

}

//Reset Hitstun Max Timers.
static void ResetMaxTimers()
{
	P1C1GroundHitstunTimerMAX = 0.0;
	P1C1AirHitstunTimerMAX = 0.0;
	P1C2GroundHitstunTimerMAX = 0.0;
	P1C2AirHitstunTimerMAX = 0.0;
	P1C3GroundHitstunTimerMAX = 0.0;
	P1C3AirHitstunTimerMAX = 0.0;
	P2C1GroundHitstunTimerMAX = 0.0;
	P2C1AirHitstunTimerMAX = 0.0;
	P2C2GroundHitstunTimerMAX = 0.0;
	P2C2AirHitstunTimerMAX = 0.0;
	P2C3GroundHitstunTimerMAX = 0.0;
	P2C3AirHitstunTimerMAX = 0.0;

	P1C1GroundRemaingHitstunMAX = 0.0;
	P1C1AirRemaingHitstunMAX = 0.0;
	P1C2GroundRemaingHitstunMAX = 0.0;
	P1C2AirRemaingHitstunMAX = 0.0;
	P1C3GroundRemaingHitstunMAX = 0.0;
	P1C3AirRemaingHitstunMAX = 0.0;
	P2C1GroundRemaingHitstunMAX = 0.0;
	P2C1AirRemaingHitstunMAX = 0.0;
	P2C2GroundRemaingHitstunMAX = 0.0;
	P2C2AirRemaingHitstunMAX = 0.0;
	P2C3GroundRemaingHitstunMAX = 0.0;
	P2C3AirRemaingHitstunMAX = 0.0;

}

//Carries over changes To restart.
void RestartWithChanges()
{
	ChangeFrankLevel(FrankLevel);
	ResetMaxTimers();
	if (Turnabout == true)
	{
		Objection();
	}
	if (DarkPhoenix == true)
	{
		PopTheBird();
	}
	if (LockEvidence == true)
	{
		ChangeWrightEvidence();
	}
	ChangeMODOKUnderstanding(MODOKLOU);
	SetDormSpellLevels();
	SetGameSpeed(1);
	SetGlobalPlayerSpeed(P1Char1Speed);
	EndlessInstallBoolUpdate();
	SetDeadpoolTeleport();
	JammingToggleRestart();
	if (BreakGlassesAtRestart == true)
	{
		BreakWeskerGlasses();
	}
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

void ResetSettings()
{
	if (GameMode == 5)
	{
		if (Player1CharNodeTree == 0 && Player2CharNodeTree == 0)
		{
			GetMainPointers();
			GetPlayerData();
			GetCharacterIDs();
			p1Pos = -125.0f;
			p2Pos = 125.0f;
			MODOKLOU = 1;
			FrankLevel = 1;
			PrestigePoints = 0.0;
			WrightEvidenceA = -1;
			WrightEvidenceB = -1;
			WrightEvidenceC = -1;
			SaveStateSlot = 0;
			DormRed = 0;
			DormBlue = 0;
			DormSpellSet = false;
			EndlessInstalls = false;
			DeadpoolTeleportCount = 0;
			FreezeDeadpoolTPCounter = false;
			P1Character1Dead = false;
			P1Character2Dead = false;
			P1Character3Dead = false;
			P2Character1Dead = false;
			P2Character2Dead = false;
			P2Character3Dead = false;
			EndlessXFactor = false;
			EndlessXFactorP1 = false;
			EndlessXFactorP2 = false;
			SpecialFeature = false;
			HitboxDisplay = false;
			SetIndividualHP = false;
			AlsoSetRedHealth = false;
			P1Char1Health = 1.0;
			P1Char1RedHealth = 1.0;
			P1Char2Health = 1.0;
			P1Char2RedHealth = 1.0;
			P1Char3Health = 1.0;
			P1Char3RedHealth = 1.0;
			P2Char1Health = 1.0;
			P2Char1RedHealth = 1.0;
			P2Char2Health = 1.0;
			P2Char2RedHealth = 1.0;
			P2Char3Health = 1.0;
			P2Char3RedHealth = 1.0;
			P1Char1HealthNew = 1.0;
			P1Char2HealthNew = 1.0;
			P1Char3HealthNew = 1.0;
			P2Char1HealthNew = 1.0;
			P2Char2HealthNew = 1.0;
			P2Char3HealthNew = 1.0;
			P1Char1Slow = false;
			P1Char2Slow = false;
			P1Char3Slow = false;
			P2Char1Slow = false;
			P2Char2Slow = false;
			P2Char3Slow = false;
			DarkPhoenix = false;
			Turnabout = false;
			LockMODOKLOU = false;
			P1C1MaxHP = 0;
			P1C2MaxHP = 0;
			P1C3MaxHP = 0;
			P2C1MaxHP = 0;
			P2C2MaxHP = 0;
			P2C3MaxHP = 0;
			CharacterSpeed = 1.0;
			ModifyGameSpeed = false;
			GameSpeed = 1.0;
			QuickRestartType = 3;
			selected_item = "Nothing";
			selected_itemTwo = "Nothing";
			selected_itemThree = "Nothing";
			P1C1Slot2Free = false;
			P1C1Slot3Free = false;
			P1C1Slot4Free = false;
			P1C1Slot5Free = false;
			P1C2Slot2Free = false;
			P1C2Slot3Free = false;
			P1C2Slot4Free = false;
			P1C2Slot5Free = false;
			P1C3Slot2Free = false;
			P1C3Slot3Free = false;
			P1C3Slot4Free = false;
			P1C3Slot5Free = false;
			P2C1Slot2Free = false;
			P2C1Slot3Free = false;
			P2C1Slot4Free = false;
			P2C1Slot5Free = false;
			P2C2Slot2Free = false;
			P2C2Slot3Free = false;
			P2C2Slot4Free = false;
			P2C2Slot5Free = false;
			P2C3Slot2Free = false;
			P2C3Slot3Free = false;
			P2C3Slot4Free = false;
			P2C3Slot5Free = false;
			P1C1Jammed = false;
			P1C2Jammed = false;
			P1C3Jammed = false;
			P2C1Jammed = false;
			P2C2Jammed = false;
			P2C3Jammed = false;
			P1C1JammedAlready = false;
			P1C2JammedAlready = false;
			P1C3JammedAlready = false;
			P2C1JammedAlready = false;
			P2C2JammedAlready = false;
			P2C3JammedAlready = false;
			P1Meter = 10000;
			P2Meter = 10000;
			P1C1JammedSlot = 0;
			P1C2JammedSlot = 0;
			P1C3JammedSlot = 0;
			P2C1JammedSlot = 0;
			P2C2JammedSlot = 0;
			P2C3JammedSlot = 0;
			KOActiveCharacterAtStart = false;
			KOActiveOpponentAtStart = false;
			FrameDelayofDeath = 0;
			DeathSiteX = 0.0;
			DeathSiteY = 0.0;
			Player1Character1Dead = false;
			Player1Character2Dead = false;
			Player1Character3Dead = false;
			Player2Character1Dead = false;
			Player2Character2Dead = false;
			Player2Character3Dead = false;
			P1Char1Speed = 1.0;
			P1Char2Speed = 1.0;
			P1Char3Speed = 1.0;
			P2Char1Speed = 1.0;
			P2Char2Speed = 1.0;
			P2Char3Speed = 1.0;
			MoveInputDisplay = false;
			LockEvidence = false;
			LockP1Meter = false;
			LockP2Meter = false;
			AlsoSetRedHealthP1 = false;
			AlsoSetRedHealthP2 = false;
			VergilSpiralSwordsForever = false;
			LockDormSpells = false;
		}


		ResetGameSpeed();
		SetGlobalPlayerSpeed(1);
		RemoveAllInstalls();
	}

}

//Saves Raw Replay Data
void SaveReplayP1()
{

	uint64_t block3 = *(uint64_t*)_addr(0x140D510A0);
	uint64_t P1RecordingData = *(uint64_t*)_addr(block3 + 0x90);
	//int P1RecordingFrameTotal = *(uintptr_t*)_addr(P1RecordingData + 0x40);
	uint64_t P1RecordingRawInput = *(uint64_t*)_addr(P1RecordingData + 0x44);

	Recording P1Recording = {
	*(uint64_t*)_addr(P1RecordingData + 0x40),

	};
	//*P1Recording.FrameCount = *(uintptr_t*)_addr(P1RecordingData + 0x40);



}

//Based on the hook by SanHKHaan. Modified for use with multiple slots.
void FUN_1402b41b0(longlong param_1)
{
	switch (RecordingSlot)
	{

	case 2:

		printf("replay %x\n", param_1);
		if (!inputRefSet) {
			inputRefSet = true;
			inputRef = (uintptr_t)param_1;
			printf("replayu %x\n", param_1);
		}

		((void (*)(longlong))_addr(0x1402b41b0))(param_1);
		if (inputRefSet) {
			if (recording) {
				printf("recording %d\n", recordReplayIndex2);
				memcpy(&replayBuffer3[recordReplayIndex2], (uint8_t*)inputRef, ReplayBufferSize);
				memcpy(&replayBuffer4[recordReplayIndex2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex2++;
				recordReplayIndex2P1++;
				recordReplayIndex2P2++;
				replayAvailable3 = true;
				if (recordReplayIndex >= ReplayLength - 1) {
					recording = false;
					recordedLength2 = recordReplayIndex2 - 1;
				}
			}
			if (recordingP1) {
				printf("recording %d\n", recordReplayIndex2P1);
				memcpy(&replayBuffer3[recordReplayIndex2P1], (uint8_t*)inputRef, ReplayBufferSize);
				recordReplayIndex2P1++;
				replayAvailable3P1 = true;
				if (recordReplayIndex2P1 >= ReplayLength - 1) {
					recordingP1 = false;
					recordedLength2P1 = recordReplayIndex2P1 - 1;
				}
			}
			if (recordingP2) {
				printf("recording %d\n", recordReplayIndexP2);
				memcpy(&replayBuffer4[recordReplayIndex2P2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex2P2++;
				replayAvailable3P2 = true;
				if (recordReplayIndex2P2 >= ReplayLength - 1) {
					recordingP2 = false;
					recordedLength2P2 = recordReplayIndex2P2 - 1;
				}
			}
			if (replaying) {
				printf("replaying %d\n", recordReplayIndex2);
				memcpy((uint8_t*)inputRef, &replayBuffer3[recordReplayIndex2], ReplayBufferSize);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer4[recordReplayIndex2], ReplayBufferSize);
				recordReplayIndex2P1++;
				recordReplayIndex2P2++;
				if (recordReplayIndex2 >= recordedLength2) {
					replaying = false;
				}
				if (recordReplayIndex2P1 >= recordedLength2) {
					replayingP1 = false;
				}
				if (recordReplayIndex2P2 >= recordedLength2) {
					replayingP2 = false;
				}
			}
			if (replayingP1) {
				printf("replaying %d\n", recordReplayIndex2P1);
				memcpy((uint8_t*)inputRef, &replayBuffer3[recordReplayIndex2P1], ReplayBufferSize);
				recordReplayIndex2P1++;
				if (recordReplayIndex2P1 >= recordedLength2P1) {
					replayingP1 = false;
				}
			}
			if (replayingP2) {
				printf("replaying %d\n", recordReplayIndex2P2);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer4[recordReplayIndex2P2], ReplayBufferSize);
				recordReplayIndex2P2++;
				if (recordReplayIndex2P2 >= recordedLength2P2) {
					replayingP2 = false;
				}
			}

		}

		break;

	case 3:

		printf("replay %x\n", param_1);
		if (!inputRefSet) {
			inputRefSet = true;
			inputRef = (uintptr_t)param_1;
			printf("replayu %x\n", param_1);
		}

		((void (*)(longlong))_addr(0x1402b41b0))(param_1);
		if (inputRefSet) {
			if (recording) {
				printf("recording %d\n", recordReplayIndex3);
				memcpy(&replayBuffer5[recordReplayIndex3], (uint8_t*)inputRef, ReplayBufferSize);
				memcpy(&replayBuffer6[recordReplayIndex3], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex3++;
				recordReplayIndex3P1++;
				recordReplayIndex3P2++;
				replayAvailable3 = true;
				if (recordReplayIndex >= ReplayLength - 1) {
					recording = false;
					recordedLength3 = recordReplayIndex3 - 1;
				}
			}
			if (recordingP1) {
				printf("recording %d\n", recordReplayIndex3P1);
				memcpy(&replayBuffer5[recordReplayIndex3P1], (uint8_t*)inputRef, ReplayBufferSize);
				recordReplayIndex3P1++;
				replayAvailable3P1 = true;
				if (recordReplayIndex3P1 >= ReplayLength - 1) {
					recordingP1 = false;
					recordedLength3P1 = recordReplayIndex3P1 - 1;
				}
			}
			if (recordingP2) {
				printf("recording %d\n", recordReplayIndexP2);
				memcpy(&replayBuffer6[recordReplayIndex3P2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex3P2++;
				replayAvailable3P2 = true;
				if (recordReplayIndex3P2 >= ReplayLength - 1) {
					recordingP2 = false;
					recordedLength3P2 = recordReplayIndex3P2 - 1;
				}
			}
			if (replaying) {
				printf("replaying %d\n", recordReplayIndex3);
				memcpy((uint8_t*)inputRef, &replayBuffer5[recordReplayIndex3], ReplayBufferSize);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer6[recordReplayIndex3], ReplayBufferSize);
				recordReplayIndex3P1++;
				recordReplayIndex3P2++;
				if (recordReplayIndex3 >= recordedLength3) {
					replaying = false;
				}
				if (recordReplayIndex3P1 >= recordedLength3) {
					replayingP1 = false;
				}
				if (recordReplayIndex3P2 >= recordedLength3) {
					replayingP2 = false;
				}
			}
			if (replayingP1) {
				printf("replaying %d\n", recordReplayIndex3P1);
				memcpy((uint8_t*)inputRef, &replayBuffer5[recordReplayIndex3P1], ReplayBufferSize);
				recordReplayIndex3P1++;
				if (recordReplayIndex3P1 >= recordedLength3P1) {
					replayingP1 = false;
				}
			}
			if (replayingP2) {
				printf("replaying %d\n", recordReplayIndex3P2);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer6[recordReplayIndex3P2], ReplayBufferSize);
				recordReplayIndex3P2++;
				if (recordReplayIndex3P2 >= recordedLength3P2) {
					replayingP2 = false;
				}
			}

		}

		break;

	case 4:

		printf("replay %x\n", param_1);
		if (!inputRefSet) {
			inputRefSet = true;
			inputRef = (uintptr_t)param_1;
			printf("replayu %x\n", param_1);
		}

		((void (*)(longlong))_addr(0x1402b41b0))(param_1);
		if (inputRefSet) {
			if (recording) {
				printf("recording %d\n", recordReplayIndex4);
				memcpy(&replayBuffer7[recordReplayIndex4], (uint8_t*)inputRef, ReplayBufferSize);
				memcpy(&replayBuffer8[recordReplayIndex4], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex4++;
				recordReplayIndex4P1++;
				recordReplayIndex4P2++;
				replayAvailable4 = true;
				if (recordReplayIndex >= ReplayLength - 1) {
					recording = false;
					recordedLength4 = recordReplayIndex4 - 1;
				}
			}
			if (recordingP1) {
				printf("recording %d\n", recordReplayIndex4P1);
				memcpy(&replayBuffer7[recordReplayIndex4P1], (uint8_t*)inputRef, ReplayBufferSize);
				recordReplayIndex4P1++;
				replayAvailable4P1 = true;
				if (recordReplayIndex4P1 >= ReplayLength - 1) {
					recordingP1 = false;
					recordedLength4P1 = recordReplayIndex4P1 - 1;
				}
			}
			if (recordingP2) {
				printf("recording %d\n", recordReplayIndexP2);
				memcpy(&replayBuffer8[recordReplayIndex4P2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex4P2++;
				replayAvailable4P2 = true;
				if (recordReplayIndex4P2 >= ReplayLength - 1) {
					recordingP2 = false;
					recordedLength4P2 = recordReplayIndex4P2 - 1;
				}
			}
			if (replaying) {
				printf("replaying %d\n", recordReplayIndex4);
				memcpy((uint8_t*)inputRef, &replayBuffer7[recordReplayIndex4], ReplayBufferSize);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer8[recordReplayIndex4], ReplayBufferSize);
				recordReplayIndex4P1++;
				recordReplayIndex4P2++;
				if (recordReplayIndex4 >= recordedLength4) {
					replaying = false;
				}
				if (recordReplayIndex4P1 >= recordedLength4) {
					replayingP1 = false;
				}
				if (recordReplayIndex4P2 >= recordedLength4) {
					replayingP2 = false;
				}
			}
			if (replayingP1) {
				printf("replaying %d\n", recordReplayIndex4P1);
				memcpy((uint8_t*)inputRef, &replayBuffer7[recordReplayIndex4P1], ReplayBufferSize);
				recordReplayIndex4P1++;
				if (recordReplayIndex4P1 >= recordedLength4P1) {
					replayingP1 = false;
				}
			}
			if (replayingP2) {
				printf("replaying %d\n", recordReplayIndex4P2);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer8[recordReplayIndex4P2], ReplayBufferSize);
				recordReplayIndex4P2++;
				if (recordReplayIndex4P2 >= recordedLength4P2) {
					replayingP2 = false;
				}
			}

		}

		break;

	case 5:

		printf("replay %x\n", param_1);
		if (!inputRefSet) {
			inputRefSet = true;
			inputRef = (uintptr_t)param_1;
			printf("replayu %x\n", param_1);
		}

		((void (*)(longlong))_addr(0x1402b41b0))(param_1);
		if (inputRefSet) {
			if (recording) {
				printf("recording %d\n", recordReplayIndex5);
				memcpy(&replayBuffer9[recordReplayIndex5], (uint8_t*)inputRef, ReplayBufferSize);
				memcpy(&replayBuffer10[recordReplayIndex5], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex5++;
				recordReplayIndex5P1++;
				recordReplayIndex5P2++;
				replayAvailable5 = true;
				if (recordReplayIndex >= ReplayLength - 1) {
					recording = false;
					recordedLength5 = recordReplayIndex5 - 1;
				}
			}
			if (recordingP1) {
				printf("recording %d\n", recordReplayIndex5P1);
				memcpy(&replayBuffer9[recordReplayIndex5P1], (uint8_t*)inputRef, ReplayBufferSize);
				recordReplayIndex5P1++;
				replayAvailable5P1 = true;
				if (recordReplayIndex5P1 >= ReplayLength - 1) {
					recordingP1 = false;
					recordedLength5P1 = recordReplayIndex5P1 - 1;
				}
			}
			if (recordingP2) {
				printf("recording %d\n", recordReplayIndexP2);
				memcpy(&replayBuffer10[recordReplayIndex5P2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex5P2++;
				replayAvailable5P2 = true;
				if (recordReplayIndex5P2 >= ReplayLength - 1) {
					recordingP2 = false;
					recordedLength5P2 = recordReplayIndex5P2 - 1;
				}
			}
			if (replaying) {
				printf("replaying %d\n", recordReplayIndex5);
				memcpy((uint8_t*)inputRef, &replayBuffer9[recordReplayIndex5], ReplayBufferSize);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer10[recordReplayIndex5], ReplayBufferSize);
				//recordReplayIndex5++;
				recordReplayIndex5P1++;
				recordReplayIndex5P2++;
				if (recordReplayIndex5 >= recordedLength5) {
					replaying = false;
				}
				if (recordReplayIndex5P1 >= recordedLength5) {
					replayingP1 = false;
				}
				if (recordReplayIndex5P2 >= recordedLength5) {
					replayingP2 = false;
				}
			}
			if (replayingP1) {
				printf("replaying %d\n", recordReplayIndex5P1);
				memcpy((uint8_t*)inputRef, &replayBuffer9[recordReplayIndex5P1], ReplayBufferSize);
				//recordReplayIndex5++;
				recordReplayIndex5P1++;
				if (recordReplayIndex5P1 >= recordedLength5P1) {
					replayingP1 = false;
				}
			}
			if (replayingP2) {
				printf("replaying %d\n", recordReplayIndex5P2);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer10[recordReplayIndex5P2], ReplayBufferSize);
				//recordReplayIndex5++;
				recordReplayIndex5P2++;
				if (recordReplayIndex5P2 >= recordedLength5P2) {
					replayingP2 = false;
				}
			}

		}

		break;

	case 1:
	default:

		printf("replay %x\n", param_1);
		if (!inputRefSet) {
			inputRefSet = true;
			inputRef = (uintptr_t)param_1;
			printf("replayu %x\n", param_1);
		}

		((void (*)(longlong))_addr(0x1402b41b0))(param_1);
		if (inputRefSet) {
			if (recording) {
				printf("recording %d\n", recordReplayIndex);
				memcpy(&replayBuffer[recordReplayIndex], (uint8_t*)inputRef, ReplayBufferSize);
				memcpy(&replayBuffer2[recordReplayIndex], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndex++;
				recordReplayIndexP1++;
				recordReplayIndexP2++;
				replayAvailable = true;
				if (recordReplayIndex >= ReplayLength - 1) {
					recording = false;
					recordedLength = recordReplayIndex - 1;
				}
			}
			if (recordingP1) {
				printf("recording %d\n", recordReplayIndexP1);
				memcpy(&replayBuffer[recordReplayIndexP1], (uint8_t*)inputRef, ReplayBufferSize);
				recordReplayIndexP1++;
				replayAvailableP1 = true;
				if (recordReplayIndexP1 >= ReplayLength - 1) {
					recordingP1 = false;
					recordedLengthP1 = recordReplayIndexP1 - 1;
				}
			}
			if (recordingP2) {
				printf("recording %d\n", recordReplayIndexP2);
				memcpy(&replayBuffer2[recordReplayIndexP2], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
				recordReplayIndexP2++;
				replayAvailableP2 = true;
				if (recordReplayIndexP2 >= ReplayLength - 1) {
					recordingP2 = false;
					recordedLengthP2 = recordReplayIndexP2 - 1;
				}
			}
			if (replaying) {
				printf("replaying %d\n", recordReplayIndex);
				memcpy((uint8_t*)inputRef, &replayBuffer[recordReplayIndex], ReplayBufferSize);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer2[recordReplayIndex], ReplayBufferSize);
				//recordReplayIndex++;
				recordReplayIndexP1++;
				recordReplayIndexP2++;
				if (recordReplayIndex >= recordedLength) {
					replaying = false;
				}
				if (recordReplayIndexP1 >= recordedLength) {
					replayingP1 = false;
				}
				if (recordReplayIndexP2 >= recordedLength) {
					replayingP2 = false;
				}
			}
			if (replayingP1) {
				printf("replaying %d\n", recordReplayIndexP1);
				memcpy((uint8_t*)inputRef, &replayBuffer[recordReplayIndexP1], ReplayBufferSize);
				//recordReplayIndex++;
				recordReplayIndexP1++;
				if (recordReplayIndexP1 >= recordedLengthP1) {
					replayingP1 = false;
				}
			}
			if (replayingP2) {
				printf("replaying %d\n", recordReplayIndexP2);
				memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer2[recordReplayIndexP2], ReplayBufferSize);
				//recordReplayIndex++;
				recordReplayIndexP2++;
				if (recordReplayIndexP2 >= recordedLengthP2) {
					replayingP2 = false;
				}
			}

		}
		break;
	}

	//Original Code here.
	/*
	((void (*)(longlong))_addr(0x1402b41b0))(param_1);
	if (inputRefSet) {
		if (recording) {
			printf("recording %d\n", recordReplayIndex);
			memcpy(&replayBuffer[recordReplayIndex], (uint8_t*)inputRef, ReplayBufferSize);
			memcpy(&replayBuffer2[recordReplayIndex], (uint8_t*)inputRef + 0x2c0, ReplayBufferSize);
			recordReplayIndex++;
			replayAvailable = true;
			if (recordReplayIndex >= ReplayLength - 1) {
				recording = false;
				recordedLength = recordReplayIndex - 1;
			}
		}
		if (replaying) {
			printf("replaying %d\n", recordReplayIndex);
			memcpy((uint8_t*)inputRef, &replayBuffer[recordReplayIndex], ReplayBufferSize);
			memcpy((uint8_t*)inputRef + 0x2c0, &replayBuffer2[recordReplayIndex], ReplayBufferSize);
			recordReplayIndex++;
			if (recordReplayIndex >= recordedLength) {
				replaying = false;
			}
		}
	}
	*/

}

//This is where the relevant Imgui stuff is!
void UMVC3Menu::Draw()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::GetIO().MouseDrawCursor = true;
	ImGui::SetNextWindowSize(ImVec2(350, 700), ImGuiCond_Once);
	ImGui::Begin("SphereTest by Eternal Yoshi", &ToolActive, flags);
	if (ImGui::BeginTabBar("##tabs"))
	{
		if (ImGui::BeginTabItem("The Main Stuff"))
		{
			//Gets the Camera Data first and foremost.
			ImGui::SeparatorText("Camera Data");

			auto main = *(sMvc3Main**)_addr(0x140E177e8);
			//Should Speak for Itself.
			if (CheckTheMode() == true)
			{
				GetMainPointers();
				CheckIfInMatch();
				if (InMatch)
				{
					TickUpdates();
					GetPlayerData();
					//GetHurtboxData();
					//GetHitboxDataPart1();
					//GetCameraStuff();

					if (!BeginHitboxDisplay)
					{
						if (ImGui::Button("Start Hitbox Display"))
						{
							BeginHitboxDisplay = true;
						}
					}

					if (BeginHitboxDisplay)
					{
						if (ImGui::Button("Stop Hitboxes Display"))
						{
							BeginHitboxDisplay = false;
						}
					}

					if (uActionSideCamera)
					{
						ImGui::Text("uActionSideCamera: %llX", uActionSideCamera);

						ImGui::Text("Near Plane: %f", NearPlane);
						ImGui::Text("Far Plane: %f", FarPlane);

						ImGui::Text("CameraPos");
						ImGui::Text("X: %f", CameraPos.x);
						ImGui::Text("Y: %f", CameraPos.y);
						ImGui::Text("Z: %f", CameraPos.z);

						ImGui::Text("CameraUp");
						ImGui::Text("X: %f", CameraUp.x);
						ImGui::Text("Y: %f", CameraUp.y);
						ImGui::Text("Z: %f", CameraUp.z);

						ImGui::Text("TargetPos");
						ImGui::Text("X: %f", TargetPos.x);
						ImGui::Text("Y: %f", TargetPos.y);
						ImGui::Text("Z: %f", TargetPos.z);

					}

					ImGui::SeparatorText("Hurtbox Data");
					//The Hurtboxes aren't really stored in the same way as the character data stuff I'm used to working with normally.
					//Have to use this funky looking method to get the characters' hurtboxes during runtime.
					uint64_t t;
					uint64_t tt;
					uint64_t ttt;
					uint64_t tttt;
					uint64_t v;
					//std::list<uint64_t>HurtboxPointers;
					int HurtboxCount = 0;
					//140D47F98

					if (ImGui::TreeNode("Hitbox Coordinates"))
					{

#pragma region P1C1

						ImGui::SeparatorText("Player 1 Character 1");
						//ReadProcessMemory(hProcess, (LPVOID*)(P1Character1Data + 0x4E10), &t, sizeof(uint64_t), 0);
						//t = *(uint64_t*)_addr(P1Character1Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", t);
						//ReadProcessMemory(hProcess, (LPVOID*)(t + 0x30), &tt, sizeof(uint64_t), 0);		
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P1C1HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P1C1Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P1C1Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P1C1Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P1C1Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P1C1HitboxCount; i++)
						{
							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C1Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P1C1Hitboxes[i].Radius);

						}

#pragma endregion

#pragma region P1C2

						ImGui::SeparatorText("Player 1 Character 2");
						//t = *(uint64_t*)_addr(P1Character2Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P1C2HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P1C2Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P1C2Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P1C2Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P1C2Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P1C2HitboxCount; i++)
						{

							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C2Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P1C2Hitboxes[i].Radius);

						}

#pragma endregion

#pragma region P1C3

						ImGui::SeparatorText("Player 1 Character 3");
						//t = *(uint64_t*)_addr(P1Character3Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P1C3HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P1C3Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P1C3Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P1C3Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P1C3Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P1C3HitboxCount; i++)
						{

							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P1C3Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P1C3Hitboxes[i].Radius);

						}

#pragma endregion

#pragma region P2C1

						ImGui::SeparatorText("Player 2 Character 1");
						//t = *(uint64_t*)_addr(P2Character1Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P2C1HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P2C1Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P2C1Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P2C1Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P2C1Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P2C1HitboxCount; i++)
						{

							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C1Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P2C1Hitboxes[i].Radius);

						}

#pragma endregion

#pragma region P2C2

						ImGui::SeparatorText("Player 2 Character 2");
						//t = *(uint64_t*)_addr(P2Character2Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P2C2HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P2C2Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P2C2Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P2C2Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P2C2Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P2C2HitboxCount; i++)
						{

							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C2Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P2C2Hitboxes[i].Radius);

						}

#pragma endregion

#pragma region P2C3

						ImGui::SeparatorText("Player 2 Character 3");
						//t = *(uint64_t*)_addr(P2Character3Data + 0x4E10);
						//tt = *(uint64_t*)_addr(t + 0x30);
						//ImGui::Text("%x", tt);
						ImGui::Text("Hurtboxes");
						//tttt = tt;

						for (int i = 0; i < P2C3HurtboxCount; i++)
						{

							ImGui::Text("X: %f", P2C3Hurtboxes[i].DeFactoX);
							ImGui::SameLine();
							ImGui::Text("Y: %f", P2C3Hurtboxes[i].DeFactoY);
							ImGui::SameLine();
							ImGui::Text("Z: %f", P2C3Hurtboxes[i].DeFactoZ);
							ImGui::SameLine();
							ImGui::Text("Size: %f", P2C3Hurtboxes[i].CollData.Radius);

						}

						ImGui::Text("Hitboxes");

						for (int i = 0; i < P2C3HitboxCount; i++)
						{

							ImGui::Text("Hitbox No: %X", (i + 1));

							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsulePrimaryPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsulePrimaryPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsulePrimaryPos.z);
							//ImGui::SameLine();
							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsuleSecondPos.x);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsuleSecondPos.y);
							ImGui::SameLine();
							ImGui::Text("X: %f", P2C3Hitboxes[i].CapsuleSecondPos.z);

							ImGui::Text("Size: %f", P2C3Hitboxes[i].Radius);

						}

#pragma endregion


						ImGui::TreePop();
					}

					ImGui::SeparatorText("Character State Data");

					ImGui::Text("P1 Character 1");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P1C1TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P1C1CharState);

					//P1C1TagState


					ImGui::Text("P1 Character 2");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P1C2TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P1C2CharState);



					ImGui::Text("P1 Character 3");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P1C3TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P1C3CharState);


					ImGui::Text("P2 Character 1");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P2C1TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P2C1CharState);


					ImGui::Text("P2 Character 2");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P2C2TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P2C1CharState);


					ImGui::Text("P2 Character 3");
					ImGui::SameLine();
					ImGui::Text("TagState: %X", P2C3TagState);
					ImGui::SameLine();
					ImGui::Text("State: % llX", P2C1CharState);

					for (int h = 0; h < P1C1ChildrenCount; h++)
					{
						ImGui::Text("Player 1 Child: % llX", P1Character1Children[h]);
					}

				}

			}
			//To be ran when not in a match to clear certain variables to prevent out of match crashes.
			if (!InMatch)
			{
				BeginHitboxDisplay = false;
				ImGui::Text("Need to be in Training Mode.");
			}

			ImGui::Text("Based on UMVC3Hook by ermaccer with some\ncode from HKHaan's repository used to power\nthis tool.");



			auto sigger = sigscan::get();

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void UMVC3Menu::UpdateControls()
{
	if (GetAsyncKeyState(ToggleDisplayKey))
	{
		if (GetTickCount64() - timer <= 150) return;
		timer = GetTickCount64();
		m_bIsActive ^= 1;
	}

}

bool UMVC3Menu::GetActiveState()
{
	return m_bIsActive;
}

void UMVC3Menu::Initialize()
{
	m_bIsActive = false;
	m_bIsFocused = false;
	//m_bCustomCameraFOV = false;
	//m_bCustomCameraPos = false;
	//m_bCustomCameraRot = false;

	//m_nFreeCameraSpeed = 1;
	//m_nFreeCameraRotationSpeed = 1;


	//camFov = 0;
	//camPos = { 0,0,0 };
	//camRot = { 0,0,0 };
}

void UMVC3Menu::PostInit()
{
	//hookCtrs(tramp);
}

void UMVC3Menu::Process()
{
	UpdateControls();
}
