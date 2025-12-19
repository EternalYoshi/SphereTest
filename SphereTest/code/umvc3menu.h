#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <list>
#include <string>
#include <tchar.h>
#include "umvc3_sMvc3Main.h"
#include "..\utils\Trampoline.h"
//#include "../glm/vec2.hpp"
//#include "../glm/vec3.hpp"

inline bool BeginHitboxDisplay = false;

inline HANDLE HProc = 0;
inline bool ValidVer;
inline int GameMode;
inline uint64_t moduleBase;
inline uint64_t sBattleSetting;
inline uint64_t gSound;
inline uint64_t sAction;
inline uint64_t sCharacter;
inline uint64_t Player1CharNodeTree;
inline uint64_t Player2CharNodeTree;
inline uint64_t BaseFighterUpdateMethod = 0x14004bd30;
inline uint64_t sGameConfig;
inline uint64_t uActionSideCamera;
inline int MatchFlag;
inline bool ToolActive = true;
inline float NearPlane;
inline float FarPlane;
inline float mFOV;

struct __declspec(align(8))Vector3
{
	float x;
	float y;
	float z;
};

inline Vector3 CameraPos;
inline Vector3 CameraUp;
inline Vector3 TargetPos;

inline static int RESOLUTION_X = 1800;
inline static int RESOLUTION_Y = 900;

inline DWORD errorMessageID;
inline int P1Character1ID;
inline int P1Character2ID;
inline int P1Character3ID;
inline int P2Character1ID;
inline int P2Character2ID;
inline int P2Character3ID;
inline uint64_t P1Character1Data;
inline uint64_t P1Character2Data;
inline uint64_t P1Character3Data;
inline uint64_t P2Character1Data;
inline uint64_t P2Character2Data;
inline uint64_t P2Character3Data;
inline uint64_t team1ptr;
inline uint64_t team2ptr;
inline int TempA;
inline int TempB;
inline unsigned char val;
#define undefined4 int

inline std::vector<uint64_t> P1Character1Children;
inline std::vector<uint64_t> P1Character2Children;
inline std::vector<uint64_t> P1Character3Children;
inline std::vector<uint64_t> P2Character1Children;
inline std::vector<uint64_t> P2Character2Children;
inline std::vector<uint64_t> P2Character3Children;

inline int P1C1ChildrenCount = 0;
inline int P1C2ChildrenCount = 0;
inline int P1C3ChildrenCount = 0;
inline int P2C1ChildrenCount = 0;
inline int P2C2ChildrenCount = 0;
inline int P2C3ChildrenCount = 0;

//inline static int64 timer = GetTickCount64();

inline float p1Pos = -125.0f;
inline float p2Pos = 125.0f;
inline bool restarted = false;
inline int restartTimer = 0;
inline int MODOKLOU = 1;
inline int FrankLevel = 1;
inline float PrestigePoints = 0.0;
inline int WrightEvidenceA = -1;
inline int WrightEvidenceB = -1;
inline int WrightEvidenceC = -1;
inline int SaveStateSlot = 0;
inline int DormRed = 0;
inline int DormBlue = 0;
inline bool DormSpellSet = false;
inline bool VeilOfMistForever = false;
inline bool EndlessInstalls = false;
inline int DeadpoolTeleportCount = 0;
inline bool FreezeDeadpoolTPCounter = false;
inline bool P1Character1Dead = false;
inline bool P1Character2Dead = false;
inline bool P1Character3Dead = false;
inline bool P2Character1Dead = false;
inline bool P2Character2Dead = false;
inline bool P2Character3Dead = false;
inline bool EndlessXFactor = false;
inline bool EndlessXFactorP1 = false;
inline bool EndlessXFactorP2 = false;
inline bool SpecialFeature = false;
inline bool HitboxDisplay = false;
inline bool SetIndividualHP = false;
inline bool AlsoSetRedHealth = false;
inline bool AlsoSetRedHealthP1 = false;
inline bool AlsoSetRedHealthP2 = false;
inline float P1Char1Health = 1.0;
inline float P1Char1RedHealth = 1.0;
inline float P1Char2Health = 1.0;
inline float P1Char2RedHealth = 1.0;
inline float P1Char3Health = 1.0;
inline float P1Char3RedHealth = 1.0;
inline float P2Char1Health = 1.0;
inline float P2Char1RedHealth = 1.0;
inline float P2Char2Health = 1.0;
inline float P2Char2RedHealth = 1.0;
inline float P2Char3Health = 1.0;
inline float P2Char3RedHealth = 1.0;
inline float P1Char1HealthNew = 1.0;
inline float P1Char2HealthNew = 1.0;
inline float P1Char3HealthNew = 1.0;
inline float P2Char1HealthNew = 1.0;
inline float P2Char2HealthNew = 1.0;
inline float P2Char3HealthNew = 1.0;
inline bool P1Char1Slow = false;
inline bool P1Char2Slow = false;
inline bool P1Char3Slow = false;
inline bool P2Char1Slow = false;
inline bool P2Char2Slow = false;
inline bool P2Char3Slow = false;
inline bool DarkPhoenix = false;
inline bool Turnabout = false;
inline bool LockMODOKLOU = false;
inline int P1C1MaxHP = 0;
inline int P1C2MaxHP = 0;
inline int P1C3MaxHP = 0;
inline int P2C1MaxHP = 0;
inline int P2C2MaxHP = 0;
inline int P2C3MaxHP = 0;
inline float CharacterSpeed = 1.0;
inline float P1Char1Speed = 1.0;
inline float P1Char2Speed = 1.0;
inline float P1Char3Speed = 1.0;
inline float P2Char1Speed = 1.0;
inline float P2Char2Speed = 1.0;
inline float P2Char3Speed = 1.0;
inline bool LockEvidence = false;
inline float ZeroHP = 0;
inline bool BreakGlassesAtRestart = false;

inline bool ModifyGameSpeed = false;
inline float GameSpeed = 1.0;
inline int QuickRestartType = 3;
inline const char* EvidenceTypes[] = { "Nothing","Photo","Watch", "Vase","Folder","Phone","Knife","Sunglasses","Beer","Bonsai", "Doll","Figurine","Plunger","Chicken" };
inline const char* EvidenceTypesTwo[] = { "Nothing","Photo","Watch", "Vase","Folder","Phone","Knife","Sunglasses","Beer","Bonsai", "Doll","Figurine","Plunger","Chicken" };
inline const char* EvidenceTypesThree[] = { "Nothing","Photo","Watch", "Vase","Folder","Phone","Knife","Sunglasses","Beer","Bonsai", "Doll","Figurine","Plunger","Chicken" };
inline static const char* selected_item = "Nothing";
inline static const char* selected_itemTwo = "Nothing";
inline static const char* selected_itemThree = "Nothing";

#define longlong  long long
#define ulonglong  unsigned long long
#define undefined8  long long*
#define undefined7  long long
#define undefined2  int
typedef int (*code)(longlong* param_1);
typedef int (*codenoarg)();
typedef void (*method)(void);
#define undefined int
#define undefined4 int
#define CONCAT71(a, b) (a << 32 | b)
#define backupsize 10000//100000
#define prebackup 0 //100000
inline constexpr int backupSize = 1000;
inline constexpr int teamBackupSize = 0x960;
inline uint64_t inputRef;
inline bool inputRefSet = false;
inline bool recording = false;
inline bool recordingP1 = false;
inline bool recordingP2 = false;
inline bool replaying = false;
inline bool replayingP1 = false;
inline bool replayingP2 = false;
inline bool replayAvailable = false;
inline bool replayAvailableP1 = false;
inline bool replayAvailableP2 = false;
inline bool replayAvailable2 = false;
inline bool replayAvailable2P1 = false;
inline bool replayAvailable2P2 = false;
inline bool replayAvailable3 = false;
inline bool replayAvailable3P1 = false;
inline bool replayAvailable3P2 = false;
inline bool replayAvailable4 = false;
inline bool replayAvailable4P1 = false;
inline bool replayAvailable4P2 = false;
inline bool replayAvailable5 = false;
inline bool replayAvailable5P1 = false;
inline bool replayAvailable5P2 = false;
inline int recordReplayIndex = 0;
inline int recordReplayIndex2 = 0;
inline int recordReplayIndex3 = 0;
inline int recordReplayIndex4 = 0;
inline int recordReplayIndex5 = 0;
inline int recordReplayIndexP1 = 0;
inline int recordReplayIndex2P1 = 0;
inline int recordReplayIndex3P1 = 0;
inline int recordReplayIndex4P1 = 0;
inline int recordReplayIndex5P1 = 0;
inline int recordReplayIndexP2 = 0;
inline int recordReplayIndex2P2 = 0;
inline int recordReplayIndex3P2 = 0;
inline int recordReplayIndex4P2 = 0;
inline int recordReplayIndex5P2 = 0;
inline int recordedLength = 0;
inline int recordedLengthP1 = 0;
inline int recordedLengthP2 = 0;
inline int recordedLength2 = 0;
inline int recordedLength2P1 = 0;
inline int recordedLength2P2 = 0;
inline int recordedLength3 = 0;
inline int recordedLength3P1 = 0;
inline int recordedLength3P2 = 0;
inline int recordedLength4 = 0;
inline int recordedLength4P1 = 0;
inline int recordedLength4P2 = 0;
inline int recordedLength5 = 0;
inline int recordedLength5P1 = 0;
inline int recordedLength5P2 = 0;
inline int RecordingSlot = 1;
#define ReplayLength (60*120)
#define ReplayBufferSize 550
inline uint8_t replayBuffer[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer2[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer3[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer4[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer5[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer6[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer7[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer8[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer9[ReplayLength][ReplayBufferSize];
inline uint8_t replayBuffer10[ReplayLength][ReplayBufferSize];
inline FILE* pRec;
static bool CheckGame();

inline int P1C1InstallID1 = 0;
inline int P1C1InstallID2 = 0;
inline int P1C1InstallID3 = 0;
inline int P1C1InstallID4 = 0;
inline int P1C1InstallID5 = 0;

inline int P1C1InstallType1 = 0;
inline int P1C1InstallType2 = 0;
inline int P1C1InstallType3 = 0;
inline int P1C1InstallType4 = 0;
inline int P1C1InstallType5 = 0;

inline int P1C2InstallID1 = 0;
inline int P1C2InstallID2 = 0;
inline int P1C2InstallID3 = 0;
inline int P1C2InstallID4 = 0;
inline int P1C2InstallID5 = 0;

inline int P1C2InstallType1 = 0;
inline int P1C2InstallType2 = 0;
inline int P1C2InstallType3 = 0;
inline int P1C2InstallType4 = 0;
inline int P1C2InstallType5 = 0;

inline int P1C3InstallID1 = 0;
inline int P1C3InstallID2 = 0;
inline int P1C3InstallID3 = 0;
inline int P1C3InstallID4 = 0;
inline int P1C3InstallID5 = 0;

inline int P1C3InstallType1 = 0;
inline int P1C3InstallType2 = 0;
inline int P1C3InstallType3 = 0;
inline int P1C3InstallType4 = 0;
inline int P1C3InstallType5 = 0;

inline int P2C1InstallID1 = 0;
inline int P2C1InstallID2 = 0;
inline int P2C1InstallID3 = 0;
inline int P2C1InstallID4 = 0;
inline int P2C1InstallID5 = 0;

inline int P2C1InstallType1 = 0;
inline int P2C1InstallType2 = 0;
inline int P2C1InstallType3 = 0;
inline int P2C1InstallType4 = 0;
inline int P2C1InstallType5 = 0;

inline int P2C2InstallID1 = 0;
inline int P2C2InstallID2 = 0;
inline int P2C2InstallID3 = 0;
inline int P2C2InstallID4 = 0;
inline int P2C2InstallID5 = 0;

inline int P2C2InstallType1 = 0;
inline int P2C2InstallType2 = 0;
inline int P2C2InstallType3 = 0;
inline int P2C2InstallType4 = 0;
inline int P2C2InstallType5 = 0;

inline int P2C3InstallID1 = 0;
inline int P2C3InstallID2 = 0;
inline int P2C3InstallID3 = 0;
inline int P2C3InstallID4 = 0;
inline int P2C3InstallID5 = 0;

inline int P2C3InstallType1 = 0;
inline int P2C3InstallType2 = 0;
inline int P2C3InstallType3 = 0;
inline int P2C3InstallType4 = 0;
inline int P2C3InstallType5 = 0;

inline bool P1C1Slot2Free = false;
inline bool P1C1Slot3Free = false;
inline bool P1C1Slot4Free = false;
inline bool P1C1Slot5Free = false;

inline bool P1C2Slot2Free = false;
inline bool P1C2Slot3Free = false;
inline bool P1C2Slot4Free = false;
inline bool P1C2Slot5Free = false;

inline bool P1C3Slot2Free = false;
inline bool P1C3Slot3Free = false;
inline bool P1C3Slot4Free = false;
inline bool P1C3Slot5Free = false;

inline bool P2C1Slot2Free = false;
inline bool P2C1Slot3Free = false;
inline bool P2C1Slot4Free = false;
inline bool P2C1Slot5Free = false;

inline bool P2C2Slot2Free = false;
inline bool P2C2Slot3Free = false;
inline bool P2C2Slot4Free = false;
inline bool P2C2Slot5Free = false;

inline bool P2C3Slot2Free = false;
inline bool P2C3Slot3Free = false;
inline bool P2C3Slot4Free = false;
inline bool P2C3Slot5Free = false;

inline bool P1C1Jammed = false;
inline bool P1C2Jammed = false;
inline bool P1C3Jammed = false;
inline bool P2C1Jammed = false;
inline bool P2C2Jammed = false;
inline bool P2C3Jammed = false;

inline bool P1C1JammedAlready = false;
inline bool P1C2JammedAlready = false;
inline bool P1C3JammedAlready = false;
inline bool P2C1JammedAlready = false;
inline bool P2C2JammedAlready = false;
inline bool P2C3JammedAlready = false;

inline int P1Meter = 10000;
inline int P2Meter = 10000;

inline bool LockP1Meter = false;
inline bool LockP2Meter = false;

inline int P1C1HurtboxCount;
inline int P1C2HurtboxCount;
inline int P1C3HurtboxCount;
inline int P2C1HurtboxCount;
inline int P2C2HurtboxCount;
inline int P2C3HurtboxCount;

inline int P1C1HitboxCount;
inline int P1C2HitboxCount;
inline int P1C3HitboxCount;
inline int P2C1HitboxCount;
inline int P2C2HitboxCount;
inline int P2C3HitboxCount;

inline int P1ProjectileCount;
inline int P2ProjectileCount;

inline std::vector<uint64_t> P1C1HurtboxPointers;
inline std::vector<uint64_t> P1C2HurtboxPointers;
inline std::vector<uint64_t> P1C3HurtboxPointers;
inline std::vector<uint64_t> P2C1HurtboxPointers;
inline std::vector<uint64_t> P2C2HurtboxPointers;
inline std::vector<uint64_t> P2C3HurtboxPointers;

inline bool RNGIncludeSlot1 = false;
inline bool RNGIncludeSlot2 = false;
inline bool RNGIncludeSlot3 = false;
inline bool RNGIncludeSlot4 = false;
inline bool RNGIncludeSlot5 = false;

inline int P1C1JammedSlot = 0;
inline int P1C2JammedSlot = 0;
inline int P1C3JammedSlot = 0;
inline int P2C1JammedSlot = 0;
inline int P2C2JammedSlot = 0;
inline int P2C3JammedSlot = 0;

inline int FrameDelayofDeath = 0;
inline float DeathSiteX = 0.0;
inline float DeathSiteY = 0.0;
inline bool KOActiveOpponentAtStart = false;
inline bool KOActiveCharacterAtStart = false;
inline bool Player1Character1Dead = false;
inline bool Player1Character2Dead = false;
inline bool Player1Character3Dead = false;
inline bool Player2Character1Dead = false;
inline bool Player2Character2Dead = false;
inline bool Player2Character3Dead = false;
inline uint64_t Player1ActiveCharacter;
inline uint64_t Player2ActiveCharacter;
inline uint64_t Player1TeamTable;
inline uint64_t Player2TeamTable;
inline uint64_t uBtKeyDisp;
inline int ButtonCount;
inline uint64_t uBtKeyDispBtnTable;
inline bool MoveInputDisplay = false;
inline bool VergilSpiralSwordsForever = false;

inline uint64_t sShotList;
inline uint64_t sShotListEnd;
inline uint64_t sTargetNode;
inline uint64_t sTargetShot;
inline uint64_t P1Shots;
inline uint64_t P2Shots;
inline std::vector< uint64_t>Player1ShotPointers;
inline std::vector< uint64_t>Player2ShotPointers;

inline int ShotCount;

inline float P1C1GroundHitstunTimer;
inline float P1C1AirHitstunTimer;
inline float P1C2GroundHitstunTimer;
inline float P1C2AirHitstunTimer;
inline float P1C3GroundHitstunTimer;
inline float P1C3AirHitstunTimer;
inline float P2C1GroundHitstunTimer;
inline float P2C1AirHitstunTimer;
inline float P2C2GroundHitstunTimer;
inline float P2C2AirHitstunTimer;
inline float P2C3GroundHitstunTimer;
inline float P2C3AirHitstunTimer;

inline float P1C1GroundHitstunTimerMAX = 0.0;
inline float P1C1AirHitstunTimerMAX = 0.0;
inline float P1C2GroundHitstunTimerMAX = 0.0;
inline float P1C2AirHitstunTimerMAX = 0.0;
inline float P1C3GroundHitstunTimerMAX = 0.0;
inline float P1C3AirHitstunTimerMAX = 0.0;
inline float P2C1GroundHitstunTimerMAX = 0.0;
inline float P2C1AirHitstunTimerMAX = 0.0;
inline float P2C2GroundHitstunTimerMAX = 0.0;
inline float P2C2AirHitstunTimerMAX = 0.0;
inline float P2C3GroundHitstunTimerMAX = 0.0;
inline float P2C3AirHitstunTimerMAX = 0.0;
inline int CheckedID;
inline int CheckedMType;
inline bool ColorHitstunTimers;
inline bool LockDormSpells = false;
inline uint64_t PointerToGameSpeedA = 0;
inline bool InMatch = false;

inline int P1C1GroundHitstunRelatedFlag;
inline int P1C1AirHitstunRelatedFlag;
inline float P1C1GroundRemaingHitstun = 0.0;
inline float P1C1RemaingBlockstun = 0.0;
inline float P1C1AirRemaingHitstun = 0.0;

inline int P1C2GroundHitstunRelatedFlag;
inline int P1C2AirHitstunRelatedFlag;
inline float P1C2GroundRemaingHitstun = 0.0;
inline float P1C2RemaingBlockstun = 0.0;
inline float P1C2AirRemaingHitstun = 0.0;

inline int P1C3GroundHitstunRelatedFlag;
inline int P1C3AirHitstunRelatedFlag;
inline float P1C3GroundRemaingHitstun = 0.0;
inline float P1C3RemaingBlockstun = 0.0;
inline float P1C3AirRemaingHitstun = 0.0;

inline int P2C1GroundHitstunRelatedFlag;
inline int P2C1AirHitstunRelatedFlag;
inline float P2C1GroundRemaingHitstun = 0.0;
inline float P2C1RemaingBlockstun = 0.0;
inline float P2C1AirRemaingHitstun = 0.0;

inline int P2C2GroundHitstunRelatedFlag;
inline int P2C2AirHitstunRelatedFlag;
inline float P2C2RemaingBlockstun = 0.0;
inline float P2C2GroundRemaingHitstun = 0.0;
inline float P2C2AirRemaingHitstun = 0.0;

inline int P2C3GroundHitstunRelatedFlag;
inline int P2C3AirHitstunRelatedFlag;
inline float P2C3RemaingBlockstun = 0.0;
inline float P2C3GroundRemaingHitstun = 0.0;
inline float P2C3AirRemaingHitstun = 0.0;

inline float P1C1GroundRemaingHitstunMAX = 0.0;
inline float P1C1AirRemaingHitstunMAX = 0.0;
inline float P1C2GroundRemaingHitstunMAX = 0.0;
inline float P1C2AirRemaingHitstunMAX = 0.0;
inline float P1C3GroundRemaingHitstunMAX = 0.0;
inline float P1C3AirRemaingHitstunMAX = 0.0;
inline float P2C1GroundRemaingHitstunMAX = 0.0;
inline float P2C1AirRemaingHitstunMAX = 0.0;
inline float P2C2GroundRemaingHitstunMAX = 0.0;
inline float P2C2AirRemaingHitstunMAX = 0.0;
inline float P2C3GroundRemaingHitstunMAX = 0.0;
inline float P2C3AirRemaingHitstunMAX = 0.0;

inline float P1C1GroundRemaingHitstunPREV = 0.0;
inline float P1C1AirRemaingHitstunPREV = 0.0;
inline float P1C2GroundRemaingHitstunPREV = 0.0;
inline float P1C2AirRemaingHitstunPREV = 0.0;
inline float P1C3GroundRemaingHitstunPREV = 0.0;
inline float P1C3AirRemaingHitstunPREV = 0.0;
inline float P2C1GroundRemaingHitstunPREV = 0.0;
inline float P2C1AirRemaingHitstunPREV = 0.0;
inline float P2C2GroundRemaingHitstunPREV = 0.0;
inline float P2C2AirRemaingHitstunPREV = 0.0;
inline float P2C3GroundRemaingHitstunPREV = 0.0;
inline float P2C3AirRemaingHitstunPREV = 0.0;

inline char P1C1AirGroundState;
inline char P1C2AirGroundState;
inline char P1C3AirGroundState;
inline char P2C1AirGroundState;
inline char P2C2AirGroundState;
inline char P2C3AirGroundState;

inline float P1C1RemaingBlockstunMAX = 0.0;
inline float P1C2RemaingBlockstunMAX = 0.0;
inline float P1C3RemaingBlockstunMAX = 0.0;
inline float P2C1RemaingBlockstunMAX = 0.0;
inline float P2C2RemaingBlockstunMAX = 0.0;
inline float P2C3RemaingBlockstunMAX = 0.0;

inline float P1C1RemaingBlockstunPREV = 0.0;
inline float P1C2RemaingBlockstunPREV = 0.0;
inline float P1C3RemaingBlockstunPREV = 0.0;
inline float P2C1RemaingBlockstunPREV = 0.0;
inline float P2C2RemaingBlockstunPREV = 0.0;
inline float P2C3RemaingBlockstunPREV = 0.0;

inline float P1C1PushblockTimer = 0.0;
inline float P1C2PushblockTimer = 0.0;
inline float P1C3PushblockTimer = 0.0;
inline float P2C1PushblockTimer = 0.0;
inline float P2C2PushblockTimer = 0.0;
inline float P2C3PushblockTimer = 0.0;

inline float P1C1PushblockTimerMAX = 0.0;
inline float P1C2PushblockTimerMAX = 0.0;
inline float P1C3PushblockTimerMAX = 0.0;
inline float P2C1PushblockTimerMAX = 0.0;
inline float P2C2PushblockTimerMAX = 0.0;
inline float P2C3PushblockTimerMAX = 0.0;

inline float P1C1PushblockTimerPREV = 0.0;
inline float P1C2PushblockTimerPREV = 0.0;
inline float P1C3PushblockTimerPREV = 0.0;
inline float P2C1PushblockTimerPREV = 0.0;
inline float P2C2PushblockTimerPREV = 0.0;
inline float P2C3PushblockTimerPREV = 0.0;

inline int P1C1CharState;
inline int P1C2CharState;
inline int P1C3CharState;
inline int P2C1CharState;
inline int P2C2CharState;
inline int P2C3CharState;

inline byte P1C1VulnState;
inline byte P1C2VulnState;
inline byte P1C3VulnState;
inline byte P2C1VulnState;
inline byte P2C2VulnState;
inline byte P2C3VulnState;

inline byte P1C1TagState;
inline byte P1C2TagState;
inline byte P1C3TagState;
inline byte P2C1TagState;
inline byte P2C2TagState;
inline byte P2C3TagState;

inline int P1C1CurAnmchrID;
inline int P1C2CurAnmchrID;
inline int P1C3CurAnmchrID;
inline int P2C1CurAnmchrID;
inline int P2C2CurAnmchrID;
inline int P2C3CurAnmchrID;

inline byte P1Child1TempVulnState;
inline int P1Child1TempCharState;
inline int P1Child1CurAnmchrID;

inline bool Emptytied = false;

inline std::vector<int> ChildCharacterCurAnmchrID;

inline uint32_t DebugPointerOfSorts;

inline std::string ThreeHookPath = "E:\\ULTIMATE MARVEL VS. CAPCOM 3\\Scripts\\ThreeHook.asi";

//inline uint64_t moduleBaseInternal = (uint64_t)GetModuleHandle(_T("umvc3.exe"));

inline sMvc3Manager* ThreeManager;

struct vector {
	float X;
	float Y;
	float Z;
};

struct vector4 {
	float X;
	float Y;
	float Z;
	float W;
};

struct cHitPrimSphere {
	long PointerIdentifier;
	int mType;
	int Unknown0C;
	int mMargin;
	int Unknown14;
	int Unknown18;
	int Unknown1C;
	vector Coordinates;
	float Radius;
	int Flag;
	int Unknown34;
	long ParentPointer;
	int Unknown3C;
	int BoneIndex;
	float SizeRateX;
	float SizeRateY;
	float SizeRateZ;
	int mOfs50;
	int mOfs54;
	int mOfs58;
	int mOfs5C;
	int mOfs60;
	int mOfs64;
	int mOfs68;
	int mOfs6C;
};

//Spheres.
struct Hurtbox
{
	long PointerIdentifier;
	int mType;
	int Unknown0C;
	long PointerToMoreData;
	cHitPrimSphere CollData;
	int Unknown18;
	int Unknown1C;
	float GlobalX;
	float GlobalY;
	float GlobalZ;
	int Unknown2C;
	float SecondaryX;
	float SecondaryY;
	float SecondaryZ;
	int Unknown3C;
	float DeFactoX;
	float DeFactoY;
	float DeFactoZ;
};

//Capsules.
struct Hitbox
{
	long ContainerIdentifier;
	int mType;
	long PointerToCapsuleData;
	long CapsuleIdentifier;
	cHitPrimSphere CollData;
	Vector3 ContainerPos;
	Vector3 CapsulePrimaryPos;
	Vector3 CapsuleSecondPos;
	float Radius;
	int mMargin;

};

struct cHitPrimCapsule {
	long PointerIdentifier;
	int mType;
	int Unknown0C;
	int mMargin;
	int Unknown14;
	int Unknown18;
	int Unknown1C;

	vector4 Point1;

	vector4 Point2;

	float Radius;
	int Unknown44;
	int Unknown48;
	int Unknown4C;

	int Flag;
	int Uknown54;
	long mpParent0;

	long mpParent1;
	int mParentNo0;
	int mParentNo1;

	float SizeRateX;
	float SizeRateY;
	float SizeRateZ;
	int Unknown7C;

	float Unknown80;
	float Unknown84;
	float Unknown88;
	float Unknown8C;

	float Unknown90;
	float Unknown94;
	float Unknown98;
	float Unknown9C;

	int mOfsA0;
	int mOfsA4;
	int mOfsA8;
	int mOfsAC;


};

struct MarvelVector {
	float x;
	float y;
	float xVelocity;
};

enum EGroundedState :uint8_t {
	NoGround = 0,
	Grounded = 0xF4,
	InAir = 0xF2,
};

enum ETagState :uint8_t {
	NoTag = 0,
	Active = 0x0D,
	Inactive = 0x09,
	Active2 = 0x68,
	TagginOut = 0xB4,
	TagginIn = 0x6c
};

struct mShotGroupAnmSerial {
	uint8_t ParamA;
	uint8_t ParamB;
	uint8_t ParamC;
	uint8_t ParamD;
};

struct mbShotGroupHit {
	bool Hit;
};

struct FighterInstall
{
	int IdentifyingHashA;
	int IdentifyingHashB;
	int InstallID;
	int mType;
	float Duration;
	int PossibleRelatedAnmchrEntry;
	int mAblF;
	int mAb2F;
	float mLifeAdd;
	float RedHealthRegen;//Either Converts Yellow Health to Red Health or heals current Red Health.
	float MeterRegen;//Meter Regen.
	float YellowHealthRegen;//Gained Yellow Health becomes Red Health. Can lose total HP with this and be set to 1HP.
	float mRLifeAdd;
	float MeterGain;
	float mRLifeAddRate;
	float DamageMultiplier;
	float DefenseMultiplier;//Incoming Damage Multiplier.
	float SpeedMultiplier;
	float mSpArmorDmgRate;
	int mSpArmorShield;
	int UnknowmSpArmorShieldAddChr;
	int mUserI;
	int Unknown58;
	int Unknown5C;
	int Unknown60;
	int mUserF;
	int Unknown68;
	int Unknown6C;
	int Unknown70;
	int Unknown74;
	int Unknown78;
	int Unknown7C;

};

struct Fighter { // TODO map this out
	char offset[0x14];//0x14
	ETagState tagState;//0x15
	char offset2[0x3B];//0x50
	MarvelVector vector;//0x5C
	char offset3[0x10];//0x6C
	EGroundedState  groundedState;//0x6D
	char offset4[0x12A3];//0x1310
	int AnimIDA;//0x1314
	int AnimIDOld;//0x1318
	char offset5[0x10];//0x1328
	float CurrentFrameA;//0x132C
	float CurrentFrameB;//0x1330
	//....
	FighterInstall Installs[5] = { 0 };//0x1570
	//.....
	char offset6[0x50D0];//0x6400
	float ArmorTimer;//0x6404
	float ArmorDamageMultiplier;//0x6408
	int ArmorShield;//I think this means the hit threshold??//0x640C
	char offset7[0x24];//0x6430
	int CurrentShotGroup;//0x6434
	mShotGroupAnmSerial ShotGroupSerials[8];//0x6454
	mbShotGroupHit ShotGroupHits[8];//0x645C
	//Commented Out because I don't quite understand it and need to set it aside for further testing and mapping out.
	//char offset4[0xc7];//0x134
	//uint16_t specialState;//specialstate? 0x136??
	//char offset5[0x528];
	//uint32_t anim; // anim??
};

struct ScriptableFighter {
	Fighter* fighter = 0;
	intptr_t fighterController = 0; // TODO find better name for this
	char* name = nullptr;

	uint64_t tickPtr = 0x14004bd30;
	void Tick() {
		//((void(__fastcall*)(longlong*))_addr(tickPtr))((longlong*)fighter);
		if (HProc) {
			ReadProcessMemory(HProc, (LPVOID*)(0x14004bd30), &fighter, sizeof(longlong), 0);
		}
	};

	const char* GetGroundedState() {
		switch (fighter->groundedState)
		{
		case Grounded:
			return "Grounded";
		case InAir:
			return "InAir";
		default:
			return "Unkown";
			break;
		}
	}
	const char* GetTagState() {
		switch (fighter->tagState)
		{
		case Active:
		case Active2:
			return "Active";
		case Inactive:
			return "Inactive";
		case TagginOut:
			return "TagginOut";
		case TagginIn:
			return "TagginIn";
		default:
			return "Unkown";
			break;
		}
	}
	int CollisionCount;
	std::list<cHitPrimSphere> HurtSpheres;
	std::list<cHitPrimCapsule> HurtCapsules;
};

inline ScriptableFighter scriptableFighters[6] = { 0 };

enum WrightEvidence
{
	Nothing = -1,
	Photo = 0,
	Watch = 1,
	Vase = 2,
	Folder = 3,
	Phone = 4,
	Knife = 5,
	Sunglasses = 6,
	Beer = 7,
	Bonsai = 8,
	Doll = 9,
	Figurine = 10,
	Plunger = 11,
	Chicken = 12,
};

struct Recording {

	int FrameCount;
	char* InputBinary[3844];

};

struct HBoxInfo {
	vector pos;
	unsigned long color;
	bool enabled;
	float size;
	HBoxInfo(float x, float y, float z, unsigned long col, float sz, bool en) {
		pos.X = x;
		pos.Y = y;
		pos.Z = z;
		color = col;
		enabled = en;
		size = sz;
	}
};

struct PlayerATIStuff {
	int AtiID;
	float FramesBeforeActive;
	float RemainingActiveFrames;
	float MaybeRecoveryFrames;
	float ATIFrame;
};

struct ShotATIStuff {
	int AtiID;
	float FramesBeforeActive;
	float RemainingActiveFrames;
	float MaybeRecoveryFrames;
	float ATIFrame;
};

inline PlayerATIStuff P1C1ActiveATIChunk;
inline PlayerATIStuff P1C2ActiveATIChunk;
inline PlayerATIStuff P1C3ActiveATIChunk;
inline PlayerATIStuff P2C1ActiveATIChunk;
inline PlayerATIStuff P2C2ActiveATIChunk;
inline PlayerATIStuff P2C3ActiveATIChunk;

inline std::vector<PlayerATIStuff> ChildCharacterActiveATIChunks;

inline FighterInstall EmptyInstall =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	0,//InstallID
	0,//mType
	0.0,//Duration
	0,//PossibleRelatedAnmchrEntry
	0,
	0,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall DarkPhoenixInstall =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	32784,//InstallID
	18,//mType
	0.0,//Duration
	224,//PossibleRelatedAnmchrEntry
	0,
	0,
	-2.34,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall TurnaboutMode =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	17,//InstallID
	10,//mType
	999999.0,//Duration
	188,//PossibleRelatedAnmchrEntry
	0,
	0,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.2,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall InvestigationMode =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	0,//InstallID
	8,//mType
	0.0,//Duration
	0,//PossibleRelatedAnmchrEntry
	0,
	0,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall Jammed =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	16,//InstallID
	23,//mType
	999999.0,//Duration
	305,//PossibleRelatedAnmchrEntry
	0,
	64,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall VergilSwords =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	0,//InstallID
	34,//mType
	0.0,//Duration
	220,//PossibleRelatedAnmchrEntry
	0,
	0,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	0.0,//Meter Gain
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall VeilOfMist =
{
	1084673544,//IdentifyingHashA
	1,//IdentifyingHashA
	65537,//InstallID
	20,//mType
	0.0,//Duration
	222,//PossibleRelatedAnmchrEntry
	0,
	128,
	0,//mLifeAdd
	0,
	0,
	0,
	1.0,
	1.0,//Meter Gain
	1.0,
	1.0,
	1.0,
	1.0,
	-1.0,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	-1,
	-0
};

inline FighterInstall RedChi =
{

};

inline FighterInstall RedBlue =
{

};

inline FighterInstall RedGreen =
{

};

inline std::vector<Hurtbox> P1C1Hurtboxes;
inline std::vector<Hurtbox> P1C2Hurtboxes;
inline std::vector<Hurtbox> P1C3Hurtboxes;
inline std::vector<Hurtbox> P2C1Hurtboxes;
inline std::vector<Hurtbox> P2C2Hurtboxes;
inline std::vector<Hurtbox> P2C3Hurtboxes;

inline std::vector<Hitbox> P1C1Hitboxes;
inline std::vector<Hitbox> P1C2Hitboxes;
inline std::vector<Hitbox> P1C3Hitboxes;
inline std::vector<Hitbox> P2C1Hitboxes;
inline std::vector<Hitbox> P2C2Hitboxes;
inline std::vector<Hitbox> P2C3Hitboxes;

inline std::vector<Hitbox> P1ShotHitCapsules;
inline std::vector<Hitbox> P2ShotHitCapsules;

inline std::vector<Hurtbox> P1ShotHitSpheres;
inline std::vector<Hurtbox> P2ShotHitSpheres;

inline std::vector<Hurtbox> ChildHurtboxes;
inline std::vector<Hitbox> ChildHitboxes;



struct ProjectileData
{
	uint64_t ShotVTAddress;
	std::string ProjName;
	struct cHitPrimCapsule Hitbox;
	struct cHitPrimCapsule Hurtbox;
	vector PossibleAbsPos;
	vector4 PossibleQuat;
	vector PossibleScale;
};

inline std::vector<ProjectileData> ActiveShots;

struct PrimHitSphere{
	vector Position;
	unsigned long Color;
	bool Enabled;
	float Radius;
	//HurtboxR(float x, float y, float z, unsigned long col, float sz, bool en)
	//{
	//	Position.X = x;
	//	Position.Y = y;
	//	Position.Z = z;
	//	Color = col;
	//	Enabled = en;
	//	Radius = sz;
	//}
};

struct PrimHitCapsule {
	vector Position;
	unsigned long Color;
	bool Enabled;
	float Radius;
	vector Position2;
};

inline std::vector<PrimHitSphere> P1C1ActiveSpheres;
inline std::vector<PrimHitSphere> P1C2ActiveSpheres;
inline std::vector<PrimHitSphere> P1C3ActiveSpheres;

inline std::vector<PrimHitSphere> P2C1ActiveSpheres;
inline std::vector<PrimHitSphere> P2C2ActiveSpheres;
inline std::vector<PrimHitSphere> P2C3ActiveSpheres;

//P1C1ActiveHitSpheres
inline std::vector<PrimHitCapsule> P1C1ActiveHitSpheres;
inline std::vector<PrimHitCapsule> P1C2ActiveHitSpheres;
inline std::vector<PrimHitCapsule> P1C3ActiveHitSpheres;

inline std::vector<PrimHitCapsule> P2C1ActiveHitSpheres;
inline std::vector<PrimHitCapsule> P2C2ActiveHitSpheres;
inline std::vector<PrimHitCapsule> P2C3ActiveHitSpheres;

//inline std::vector<HurtboxR> P1C1ChildActiveSpheres;
//inline std::vector<HitboxR> P1C1Child1ActiveHitSpheres;

//Going to use a struct to get the data that I care about.
struct ChildData {
	//int HostCharacterID;
	PlayerATIStuff ChildActiveATIChunk;
	std::vector<Hurtbox> ChildActiveSpheres;
	std::vector<Hitbox> Child1ActiveHitSpheres;
	int CurAnmchrID;
	int ChildCharState;
	byte ChildVulnState;
	float WeirdFloat;
};
inline std::vector<ChildData> P1C1ActiveChildData;
inline std::vector<ChildData> P1C2ActiveChildData;
inline std::vector<ChildData> P1C3ActiveChildData;

inline std::vector<ChildData> P2C1ActiveChildData;
inline std::vector<ChildData> P2C2ActiveChildData;
inline std::vector<ChildData> P2C3ActiveChildData;

//Struct I made to get relevant data
struct ShotData {
	float LifeTime;
	float BaseDuration;
	uint64_t ShotVTAddress;
	uint64_t rAtkInfoPtr;
	bool CollIsCapsule;
	std::vector<Hurtbox> ShotActiveSpheres;
	std::vector<Hitbox> ShotActiveHitSpheres;
	vector FirstVector;
	vector SecondVector;
	vector ThirdVector;
	//float FirstRadius;
	//float SecondRadius;
	//float ThirdRadius;
	int PeculiarCounter;
	uint64_t FirstCollPtr;
};

inline std::vector<ShotData> Player1Shots;
inline std::vector<ShotData> Player2Shots;

//class UMVC3Menu {
//public:
//	bool m_bIsActive;
//	bool m_bIsFocused;
//	bool m_bCustomCameraPos;
//	bool m_bCustomCameraFOV;
//	bool m_bCustomCameraRot;
//	bool m_bFreeCam;
//
//	int  m_nFreeCameraSpeed;
//	int  m_nFreeCameraRotationSpeed;
//
//	vector camPos;
//	vector camRot;
//	float  camFov;
//
//	void Initialize();
//	void PostInit();
//	void Process();
//	void Draw();
//	void UpdateControls();
//	bool GetActiveState();
//	Trampoline* tramp = nullptr;
//};
//
////char* GetUMVC3HookVersion();
//extern UMVC3Menu* TheMenu;

bool CheckTheMode();

void GetMainPointers();

void GetCharacterIDs();

void GetPlayerData();

void EndlessXFactorUpdate();

void SetIndividualCharacterHealth();

void ChangeFrankLevel(int FrankLevel);

void ChangeWrightEvidence();

int EvidenceNameToID(const char* name);

void Objection();

void PopTheBird();

void ChangeMODOKUnderstanding(int MODOKLOU);

void SetDormSpellLevels();

void EndlessInstallBoolUpdate();

void SetDeadpoolTeleport();

void BreakWeskerGlasses();

int CheckInstallSlotA(int InstallSlot, int CharID, FighterInstall install, uint64_t PlayerDataPointer);

void VergilSpiralSwords();

void TurnOnTheSwords();

void CheckIfInMatch();

void JammingToggle();

void JammingToggleRestart();

void SetGlobalPlayerSpeed(float GlobalSpeed);

void SetGameSpeed(float GameSpeed);

void ResetGameSpeed();

void GetActiveInstallData();

void SetMeters();

void GetShots();

ShotData GetShotData();

static std::string HashToTypeName(uint64_t Hash);

static void GetCharacterHurtboxData(std::vector<Hurtbox> PlayerHurtboxes, int HurtboxCount, uint64_t PlayerPtr);

void GetHurtboxData();

static void GetCharacterHitboxData(std::vector<Hitbox> PlayerHitboxes, int HitboxCount, uint64_t PlayerPtr);

static void GetCharacterHurtboxData(std::vector<Hurtbox> PlayerHurtboxes, int HurtboxCount, uint64_t PlayerPtr);

void GetHurtboxData();

void RemoveAllSpheres();

void GetHitboxDataPart1();

void RemoveAllInstalls();

bool CheckIfDead(uint64_t PlayerPtr);

static void ForJean(uint64_t PlayerPtr);

void RestartWithChanges();

void TickUpdates();

void DeathDelay();

void KOToggles();

void GetChildData();

void GetDebugData();

//Sets all settings back to default.
void ResetSettings();

void LeftSideInputDisplay();

void EmptyShotLists();

void GetHitstunTimers();

void FUN_1402b41b0(longlong param_1);

void GetCameraStuff();

void GetChildCharacters();

void GetAnmchrIDs();

void GetEvenMorePlayerData();

std::vector<ChildData> GetChildCharacterData(uint64_t PlayerPtr, int CharacterID, uint64_t NodeTree, std::vector<ChildData> ActiveChildData);

ChildData GetChildCollisionData(uint64_t PlayerPtr, uint64_t ChildPtr);

void EmptyTheChildLists();

class UMVC3Menu {
public:
	bool m_bIsActive;
	bool m_bIsFocused;
	bool m_bCustomCameraPos;
	bool m_bCustomCameraFOV;
	bool m_bCustomCameraRot;
	bool m_bFreeCam;

	int  m_nFreeCameraSpeed;
	int  m_nFreeCameraRotationSpeed;

	vector camPos;
	vector camRot;
	float  camFov;

	void Initialize();
	void PostInit();
	void Process();
	void Draw();
	void UpdateControls();
	bool GetActiveState();
	Trampoline* tramp = nullptr;


	////TODO put these in better place
	//static vector getP1Pos();
	//static bool startedHitbox();
	//static int getHBoxCount(int fighterIndex);
	//static int getFighterIndex(uintptr_t ptr);
	//static HBoxInfo getHBoxPos(int index, int fighterIndex);
	//static HBoxInfo getHitBoxPos(int index);
	//static int getHitboxCount();
	//static glm::vec2 screenPosition(glm::vec3& _coord);
	//static float getXMultiplier();
	//static float* getXMultiplierPtr();
	//static void setXMultiplier(float val);
	//static float getYMultiplier();
	//static float* getYMultiplierPtr();
	//static void setYMultiplier(float val);
	//static void setCameraPtr(uintptr_t camptr);
};

char* GetUMVC3HookVersion();
extern UMVC3Menu* TheMenu;