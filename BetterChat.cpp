#include "pch.h"
#include "BetterChat.h"
#include <nlohmann/json.hpp>
#include <set>
#include <iostream>
#include <fstream>
#include <chrono>
#include <map>
#include <tuple>
#include <string>

BAKKESMOD_PLUGIN(BetterChat, "BetterChat Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

using namespace std;
using json = nlohmann::json;

set<string> blacklist;

struct pInfos {
	string previousMsg;
	chrono::system_clock::time_point previousTime;
	int blockedMsg;
	int numMsg;
	unsigned char teamNum;
};

map<string, pInfos> playerInfo;
chrono::seconds delay;

chrono::system_clock::time_point lastSaveTime;
bool save;

unsigned char numTeam;
int blueScore;
int orangeScore;
bool replayEnded;
bool goal;
bool assist;
unsigned char lastTouchTeam;
unsigned char secondLastTouchTeam;

void BetterChat::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("betterchat_enabled", "1", "Enable BetterChat Plugin", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_antispam", "1", "Enable AntiSpam", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_chatfilter", "1", "Enable ChatFilter", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_delay", "5", "Delay between two same messages from the same player");
	cvarManager->registerCvar("betterchat_score_X", "1530", "", true, true, 0, true, 1920);
	cvarManager->registerCvar("betterchat_score_Y", "15", "", true, true, 0, true, 1080);

	jsonFileExists();

	resetBlacklist();

	gameWrapper->HookEventWithCaller<ActorWrapper>("Function GameEvent_TA.Countdown.BeginState", bind(&BetterChat::gameBegin, this));
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&BetterChat::onStatTickerMessage, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", bind(&BetterChat::chatMessageEvent, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Replay_TA.StopPlayback", bind(&BetterChat::replayEnd, this));
	gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.EventHitBall", bind(&BetterChat::hitBall, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Ball_TA.OnHitGoal", bind(&BetterChat::onGoal, this));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TA.Destroyed", bind(&BetterChat::gameDestroyed, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", std::bind(&BetterChat::gameEnd, this));
	LOG("Plugin On");
}

void BetterChat::onUnload()
{
	LOG("Plugin Off");
	gameWrapper->UnhookEvent("Function TAGame.HUDBase_TA.OnChatMessage");
	gameWrapper->UnhookEvent("Function TAGame.GFxData_Chat_TA.OnChatMessage");
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");
}

/// <summary>
/// Reset the blacklist										 
/// </summary>												 
void BetterChat::resetBlacklist() {
	LOG("Messages have been reset");
	blacklist.clear();
	set<string> defaultMsg = readJson("default");
	for (const auto& id : defaultMsg) {
		blacklist.emplace(id);
	}
}

/// <summary>
/// Check if the json file exists. If not, it create it
/// </summary>
void BetterChat::jsonFileExists() {
	if (!filesystem::exists(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json")) {
		ofstream NewFile(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
		NewFile << setw(4) << "{ \"default\" : [\"Group2Message1\", \"Group2Message2\",	\"Group2Message3\",	\"Group2Message4\",	\"Group2Message5\",	\"Group2Message6\",	\"Group2Message7\",	\"Group2Message8\",	\"Group2Message9\",	\"Group2Message10\", \"Group3Message1\", \"Group3Message2\", \"Group3Message3\", \"Group3Message4\", \"Group3Message5\", \"Group3Message6\", \"Group3Message7\", \"Group3Message8\", \"Group3Message9\", \"Group3Message10\", \"Group3Message11\", \"Group1Message10\", \"Group4Message1\", \"Group5Message7\", \"Group6Message4\"], \"beforeKickoff\" : [\"Group1Message10\"], \"afterAlliedGoal\" : [\"Group1Message10\",	\"Group2Message1\",	\"Group2Message3\",	\"Group2Message5\",	\"Group2Message9\", \"Group2Message10\", \"Group3Message1\", \"Group3Message3\", \"Group3Message5\", \"Group3Message8\", \"Group3Message10\", \"Group2Message6\", \"Group6Message4\"], \"afterEnemyGoal\" : [\"Group2Message1\", \"Group2Message3\", \"Group2Message5\"], \"afterPass\" : [\"Group2Message2\"], \"afterSave\" : [\"Group2Message4\", \"Group2Message8\"]}" << endl;
		NewFile.close();
	}
}
/// <summary>
///	Return the message list of the specified category
/// </summary>
set<string> BetterChat::readJson(string category) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
	json jsonData;
	file >> jsonData;

	set<string> messages = jsonData[category];
	return messages;
}

/// <summary>
/// Add a quickchat to the blacklist
/// </summary>
/// <param name="category"> The category to which the quickchat is to be added </param>
/// <param name="idMsg"> The id of the quickchat </param>
bool BetterChat::addToJson(string category, string idMsg) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();

	auto index = find(jsonData[category].begin(), jsonData[category].end(), idMsg);

	if (index == jsonData[category].end()) {
		jsonData[category].push_back(idMsg);
		ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
		outputFile << std::setw(4) << jsonData << endl;
		outputFile.close();
		LOG("\"" + BetterChat::idQuickchats[idMsg] + "\" added to '" + category + "' category.");
		return true;
	}
	else {
		return false;
	}
}

/// <summary>
/// Remove a quickchat from the blacklist
/// </summary>
/// <param name="category"> The category from which the quickchat is to be removed </param>
/// <param name="idMsg"> The id of the quickchat </param>
bool BetterChat::removeFromJson(string category, string idMsg) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();

	auto index = find(jsonData[category].begin(), jsonData[category].end(), idMsg);

	if (index != jsonData[category].end()) {
		jsonData[category].erase(index);
		ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json");
		outputFile << std::setw(4) << jsonData << endl;
		outputFile.close();
		LOG("\"" + BetterChat::idQuickchats[idMsg] + "\" removed from '" + category + "' category.");
		return true;
	}
	else {
		return false;
	}
}

// Game begin
void BetterChat::gameBegin() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	gameWrapper->UnregisterDrawables();

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	int maxScore = server.GetTotalScore();
	if (maxScore > 0) { return; }
	resetBlacklist();
	playerInfo.clear();
	LOG("[EVENT] Game start");
	LOG("Players list reset");

	GameEventWrapper gew = gameWrapper->GetLocalCar().GetGameEvent();
	ArrayWrapper listeJoueurs = gew.GetPRIs();
	numTeam = gameWrapper->GetLocalCar().GetPRI().GetTeamNum();
	blueScore = 0;
	orangeScore = 0;
	replayEnded = false;
	goal = false;
	assist = false;
	lastTouchTeam = -1;
	secondLastTouchTeam = -1;
	lastSaveTime = chrono::system_clock::now();
	save = false;

	pInfos infos;
	for (int i = 0; i < listeJoueurs.Count(); i++) {
		infos.previousMsg = "";
		infos.previousTime = chrono::system_clock::now();
		infos.blockedMsg = 0;
		infos.numMsg = 0;
		infos.teamNum = listeJoueurs.Get(i).GetTeamNum();
		playerInfo.emplace(listeJoueurs.Get(i).GetPlayerName().ToString(), infos);
		LOG(listeJoueurs.Get(i).GetPlayerName().ToString());
	}
}

// Event
void BetterChat::onStatTickerMessage(ServerWrapper caller, void* params) {
	StatTickerParams* pstats = (StatTickerParams*)params;
	StatEventWrapper event = StatEventWrapper(pstats->StatEvent);

	CarWrapper me = gameWrapper->GetLocalCar();
	PriWrapper receiver = PriWrapper(pstats->Receiver);
	PriWrapper victim = PriWrapper(pstats->Victim);
	std::string name = event.GetEventName();

	if (!gameWrapper->IsInOnlineGame()) { return; }

	ServerWrapper server = gameWrapper->GetCurrentGameState();

	if (name == "Goal") { // Goal
		goal = true;
		save = false;
	}
	else if (name == "Save" || name == "EpicSave") { // Save
		resetBlacklist();
		LOG("[EVENT] Save");
		set<string> afterSaveMsg = readJson("afterSave");
		for (const auto& id : afterSaveMsg) {
			blacklist.erase(id);
		}
		lastSaveTime = chrono::system_clock::now();
		save = true;
	}
	else if (name == "Assist") { // Pass
		LOG("[EVENT] Assist");
		assist = true;
	}
}

// Goal
void BetterChat::onGoal() {
	if (!gameWrapper->IsInOnlineGame() || !goal) { return; }
	resetBlacklist();
	goal = false;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();
	int scorerTeam = -1;
	if (teams.Get(0).GetScore() > blueScore) {
		blueScore = teams.Get(0).GetScore();
		scorerTeam = 0;
	}
	else if (teams.Get(1).GetScore() > orangeScore) {
		orangeScore = teams.Get(1).GetScore();
		scorerTeam = 1;
	}

	if (scorerTeam == numTeam) { // Allied goal
		LOG("[EVENT] Allied goal");
		set<string> afterAlliedGoalMsg = readJson("afterAlliedGoal");
		;		for (const auto& id : afterAlliedGoalMsg) {
			blacklist.erase(id);
		}
	}
	else { // Enemy goal
		LOG("[EVENT] Enemy goal");
		set<string> afterEnemyGoalMsg = readJson("afterEnemyGoal");
		for (const auto& id : afterEnemyGoalMsg) {
			blacklist.erase(id);
		}
	}
	if (assist) { // Save
		assist = false;
		set<string> afterPassMsg = readJson("afterPass");
		for (const auto& id : afterPassMsg) {
			blacklist.erase(id);
		}
	}
}

// Kick-off
void BetterChat::hitBall(CarWrapper car, void* params) {
	if (replayEnded == true) {
		replayEnded = false;
		LOG("[EVENT] Kick-off");
		resetBlacklist();
	}
}

// Replay end
void BetterChat::replayEnd() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	replayEnded = true;
	set<string> beforeKickoffMsg = readJson("beforeKickoff");
	for (const auto& id : beforeKickoffMsg) {
		blacklist.erase(id);
	}
	lastTouchTeam = -1;
	secondLastTouchTeam = -1;
}

// Game end
void BetterChat::gameEnd() {
	resetBlacklist();
	LOG("[EVENT] Game end");
	gameWrapper->RegisterDrawable(bind(&BetterChat::ShowToxicityScores, this, std::placeholders::_1));
}

/// <summary>
/// Display the table with the number of blocked messages per player during the game
/// </summary>
/// <param name="canvas"> Canvas Wrapper </param>
void BetterChat::ShowToxicityScores(CanvasWrapper canvas) {
	canvas.SetColor(LinearColor(255, 255, 255, 255));

	int x = cvarManager->getCvar("betterchat_score_X").getIntValue();
	int y = cvarManager->getCvar("betterchat_score_Y").getIntValue();

	canvas.SetPosition(Vector2({ x, y }));
	canvas.DrawString("Player names", 1, 1);

	canvas.SetPosition(Vector2({ x + 150, y }));
	canvas.DrawString("Messages blocked", 1, 1);

	canvas.SetPosition(Vector2({ x + 300, y }));
	canvas.DrawString("Ratio", 1, 1);

	LinearColor blueTeam = LinearColor(0, 75, 255, 255);
	LinearColor orangeTeam = LinearColor(255, 165, 0, 255);

	map<string, pInfos>::iterator player;
	for (player = playerInfo.begin(); player != playerInfo.end(); ++player) {

		if (player->second.teamNum == 0) {
			canvas.SetColor(blueTeam);
		}
		else {
			canvas.SetColor(orangeTeam);
		}

		canvas.SetPosition(Vector2({ x, y + 20 * (int)distance(playerInfo.begin(), playerInfo.find(player->first)) + 20 }));
		canvas.DrawString(player->first, 1, 1);

		string score;
		double percentage;
		if (player->second.numMsg != 0) {
			percentage = round((double)player->second.blockedMsg * 10000 / (double)player->second.numMsg) / 100;

			char number[4];
			sprintf(number, "%.2f%%", percentage);
			score = number;
		}
		else {
			percentage = 0;
			score = "0.00%";
		}

		if (percentage < 25) {
			canvas.SetColor(LinearColor(0, 255, 0, 255));
		}
		else if (percentage < 50) {
			canvas.SetColor(LinearColor(255, 255, 0, 255));
		}
		else if (percentage < 75) {
			canvas.SetColor(LinearColor(255, 165, 0, 255));
		}
		else {
			canvas.SetColor(LinearColor(255, 0, 0, 255));
		}

		canvas.SetPosition(Vector2({ x + 150, y + 20 * (int)distance(playerInfo.begin(), playerInfo.find(player->first)) + 20 }));
		canvas.DrawString(to_string(player->second.blockedMsg) + "/" + to_string(player->second.numMsg), 1, 1);

		canvas.SetPosition(Vector2({ x + 300, y + 20 * (int)distance(playerInfo.begin(), playerInfo.find(player->first)) + 20 }));
		canvas.DrawString(score, 1, 1);
	}
}

/// <summary>
/// Erase the table
/// </summary>
void BetterChat::gameDestroyed() {
	gameWrapper->UnregisterDrawables();
}

//
// Chat manipulation
//

struct FString
{
public:
	using ElementType = const wchar_t;
	using ElementPointer = ElementType*;

private:
	ElementPointer ArrayData;
	int32_t ArrayCount;
	int32_t ArrayMax;

public:
	FString()
	{
		ArrayData = nullptr;
		ArrayCount = 0;
		ArrayMax = 0;
	}

	FString(ElementPointer other)
	{
		ArrayData = nullptr;
		ArrayCount = 0;
		ArrayMax = 0;

		ArrayMax = ArrayCount = *other ? (wcslen(other) + 1) : 0;

		if (ArrayCount > 0)
		{
			ArrayData = other;
		}
	}

	~FString() {}

public:
	std::string ToString() const
	{
		if (!IsValid())
		{
			std::wstring wideStr(ArrayData);
			std::string str(wideStr.begin(), wideStr.end());
			return str;
		}

		return std::string("null");
	}

	bool IsValid() const
	{
		return !ArrayData;
	}

	FString operator=(ElementPointer other)
	{
		if (ArrayData != other)
		{
			ArrayMax = ArrayCount = *other ? (wcslen(other) + 1) : 0;

			if (ArrayCount > 0)
			{
				ArrayData = other;
			}
		}

		return *this;
	}

	bool operator==(const FString& other)
	{
		return (!wcscmp(ArrayData, other.ArrayData));
	}
};

FString FS(const std::string& s) {
	wchar_t* p = new wchar_t[s.size() + 1];
	for (std::string::size_type i = 0; i < s.size(); ++i)
		p[i] = s[i];

	p[s.size()] = '\0';
	return FString(p);
}

struct ChatMessage1
{
	void* PRI;
	void* Team;
	wchar_t* PlayerName;
	uint8_t PlayerNamePadding[0x8];
	wchar_t* Message;
	uint8_t MessagePadding[0x8];
	uint8_t ChatChannel;
	unsigned long bPreset : 1;
};

struct ChatMessage2 {
	int32_t Team;
	class FString PlayerName;
	class FString Message;
	uint8_t ChatChannel;
	bool bLocalPlayer : 1;
	unsigned char IDPadding[0x48];
	uint8_t MessageType;
};

/// <summary>
/// Erase a quickchat message
/// </summary>
void BetterChat::cancelMsg() {
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_Chat_TA.OnChatMessage", [this](ActorWrapper Caller, void* params, ...) {
		ChatMessage2* Params = (ChatMessage2*)params;
		Params->Message = FS("");
		Params->PlayerName = FS("");
		Params->ChatChannel = 0;
		});
}

/// <summary>
/// Determine if a quickchat message should be erased or not
/// </summary>
void BetterChat::chatMessageEvent(ActorWrapper caller, void* params) {
	if (params && cvarManager->getCvar("betterchat_enabled").getBoolValue()) {

		if (cvarManager->getCvar("betterchat_antispam").getBoolValue()) {
			delay = std::chrono::seconds(cvarManager->getCvar("betterchat_delay").getIntValue());
		}
		else {
			delay = std::chrono::seconds(0);
		}

		ChatMessage1* chatMessage = static_cast<ChatMessage1*>(params);
		if (chatMessage->PlayerName == nullptr) return;
		std::wstring player(chatMessage->PlayerName);
		std::string playerName(player.begin(), player.end());

		if (chatMessage->Message == nullptr) return;
		std::wstring message(chatMessage->Message);
		std::string msgID(message.begin(), message.end());

		playerInfo[playerName].numMsg += 1;

		if (msgID != playerInfo[playerName].previousMsg) { // Different message
			playerInfo[playerName].previousMsg = msgID;
			playerInfo[playerName].previousTime = chrono::system_clock::now();
			if (cvarManager->getCvar("betterchat_chatfilter").getBoolValue() && std::find(blacklist.begin(), blacklist.end(), msgID) != blacklist.end()) { // If message in blacklist
				cancelMsg();
				playerInfo[playerName].blockedMsg += 1;
			}
		}
		else { // Same message
			if (std::chrono::system_clock::now() < playerInfo[playerName].previousTime + delay) { // If it is spam
				cancelMsg();
				playerInfo[playerName].previousTime = chrono::system_clock::now();
				playerInfo[playerName].blockedMsg += 1;
			}
			else {
				playerInfo[playerName].previousMsg = msgID;
				playerInfo[playerName].previousTime = chrono::system_clock::now();
				if (cvarManager->getCvar("betterchat_chatfilter").getBoolValue() && std::find(blacklist.begin(), blacklist.end(), msgID) != blacklist.end()) { // Else, if message in blacklist
					cancelMsg();
					playerInfo[playerName].blockedMsg += 1;
				}
			}
		}
	}
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", [this](ActorWrapper caller, void* params, ...) {
		gameWrapper->UnhookEvent("Function TAGame.GFxData_Chat_TA.OnChatMessage");
	});
}