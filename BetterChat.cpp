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
#include <regex>

BAKKESMOD_PLUGIN(BetterChat, "BetterChat Plugin", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

using namespace std;
using json = nlohmann::json;

list<string> whitelist;

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
unsigned char thirdLastTouchTeam;

int lastToucherID;

void BetterChat::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("betterchat_enabled", "1", "Enable BetterChat Plugin", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_antispam", "1", "Enable AntiSpam", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_chatfilter", "1", "Enable ChatFilter", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_delay", "5", "Delay between two same messages from the same player", true, true, 0, true, 10);
	cvarManager->registerCvar("betterchat_nowrittenmsg", "0", "Disable written messages", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_writtenmsgastoxic", "0", "Consider written messages as toxic", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_aftersavetime", "5", "Time the 'After save' messages are allowed after a save", true, true, 0, true, 20);
	cvarManager->registerCvar("betterchat_owngoal", "0", "Do not count the goal if it is an owngoal", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_unwanted_pass", "0", "Do not count the pass if an opponent touch it before the goal", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_toxicityscores", "1", "Enable Toxicity Scores", true, true, 0, true, 1);
	cvarManager->registerCvar("betterchat_score_X", "1530", "", true, true, 0, true, 1920);
	cvarManager->registerCvar("betterchat_score_Y", "45", "", true, true, 0, true, 1080);

	jsonFileExists();

	resetWhitelist();

	gameWrapper->HookEventWithCaller<ActorWrapper>("Function GameEvent_TA.Countdown.BeginState", bind(&BetterChat::gameBegin, this));
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&BetterChat::onStatTickerMessage, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", bind(&BetterChat::chatMessageEvent, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Replay_TA.StopPlayback", bind(&BetterChat::replayEnd, this));
	gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.Car_TA.EventHitBall", bind(&BetterChat::hitBall, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated", bind(&BetterChat::onTimerUpdate, this));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Ball_TA.OnHitGoal", bind(&BetterChat::onGoal, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated", std::bind(&BetterChat::onOvertimeStarted, this));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TA.Destroyed", bind(&BetterChat::gameDestroyed, this));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet", bind(&BetterChat::gameEnd, this));
	LOG("Plugin On");
}

void BetterChat::onUnload()
{
	LOG("Plugin Off");
	gameWrapper->UnhookEvent("Function GameEvent_TA.Countdown.BeginState");
	gameWrapper->UnhookEvent("Function TAGame.GFxHUD_TA.HandleStatTickerMessage");
	gameWrapper->UnhookEvent("Function TAGame.HUDBase_TA.OnChatMessage");
	gameWrapper->UnhookEvent("Function TAGame.Replay_TA.StopPlayback");
	gameWrapper->UnhookEvent("Function TAGame.Car_TA.EventHitBall");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnGameTimeUpdated");
	gameWrapper->UnhookEvent("Function TAGame.Ball_TA.OnHitGoal");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_TA.Destroyed");
	gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet");
}

/// <summary>
/// Reset the whitelist										 
/// </summary>
void BetterChat::resetWhitelist() {
	LOG("Messages have been reset");
	whitelist.clear();
	map<string, bool> defaultMsg = readJson("default");
	for (const auto& pair : defaultMsg) {
		if (pair.second) {
			whitelist.emplace_back(pair.first);
		}
	}
}

/// <summary>
/// Check if the json file exists. If not, it create it
/// </summary>
void BetterChat::jsonFileExists() {
	if (!filesystem::exists(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json")) {

		ofstream NewFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		NewFile << setw(4) << "{\"afterAlliedGoal\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":true,\"Group3Message10\":true,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":true,\"Group3Message4\":false,\"Group3Message5\":true,\"Group3Message6\":true,\"Group3Message7\":false,\"Group3Message8\":true,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":true},\"afterEnemyGoal\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"afterPass\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":true,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"afterSave\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":true,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":true,\"Group2Message8\":true,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"beforeKickoff\":{\"Group1Message1\":true,\"Group1Message10\":true,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":false,\"Group2Message4\":false,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"default\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":false,\"Group2Message4\":false,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false}}" << endl;
		NewFile.close();

		ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		json jsonData;
		file >> jsonData;
		file.close();

		for (const auto& pair : BetterChat::idQuickchats) {
			if (jsonData["default"].find(pair.first) == jsonData["default"].end()) {
				jsonData["default"][pair.first] = true;
			}

			list<string> events = { "beforeKickoff", "afterAlliedGoal", "afterEnemyGoal", "afterPass", "afterSave" };
			for (const string event : events) {
				if (jsonData[event].find(pair.first) == jsonData[event].end()) {
					jsonData[event][pair.first] = false;
				}
			}
		}

		ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		outputFile << std::setw(4) << jsonData << endl;
		outputFile.close();
	}
}

/// <summary>
///	Return the message list of the specified category
/// </summary>
map<string, bool> BetterChat::readJson(string category) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;

	map<string, bool> messages = jsonData[category];
	return messages;
}

/// <summary>
/// Add a quickchat to the blacklist
/// </summary>
/// <param name="category"> The category to which the quickchat is to be added </param>
/// <param name="idMsg"> The id of the quickchat </param>
void BetterChat::toggleQuickchatInJson(string category, string idMsg) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();

	jsonData[category][idMsg] = !jsonData[category][idMsg];
	
	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
	LOG("\"" + BetterChat::idQuickchats[idMsg] + "\" is now " + (jsonData[category][idMsg] ? "allowed" : "forbidden") + " in '" + category + "' category.");
}

// Game begin
void BetterChat::gameBegin() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	gameWrapper->UnregisterDrawables();

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	int maxScore = server.GetTotalScore();
	if (maxScore > 0) { return; }
	resetWhitelist();
	playerInfo.clear();
	LOG("[EVENT] Game start");

	numTeam = gameWrapper->GetLocalCar().GetPRI().GetTeamNum();
	blueScore = 0;
	orangeScore = 0;
	replayEnded = true;
	replayEnd();
	goal = false;
	assist = false;
	lastTouchTeam = -1;
	secondLastTouchTeam = -1;
	thirdLastTouchTeam = -1;
	lastToucherID = -1;
	lastSaveTime = chrono::system_clock::now();
	save = false;
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
		whitelist.clear();
		LOG("[EVENT] Save");
		map<string, bool> afterSaveMsg = readJson("afterSave");
		for (const auto& pair : afterSaveMsg) {
			if (pair.second) {
				whitelist.emplace_back(pair.first);
			}
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
	whitelist.clear();
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

	if (scorerTeam == lastTouchTeam || !cvarManager->getCvar("betterchat_owngoal").getBoolValue()) {
		if (scorerTeam == numTeam) { // Allied goal
			LOG("[EVENT] Allied goal");
			map<string, bool> afterAlliedGoalMsg = readJson("afterAlliedGoal");
			for (const auto& pair : afterAlliedGoalMsg) {
				if (pair.second) {
					whitelist.emplace_back(pair.first);
				}
			}
		}
		else { // Enemy goal
			LOG("[EVENT] Enemy goal");
			map<string, bool> afterEnemyGoalMsg = readJson("afterEnemyGoal");
			for (const auto& pair : afterEnemyGoalMsg) {
				if (pair.second) {
					whitelist.emplace_back(pair.first);
				}
			}
		}
	}

	if ((lastTouchTeam == scorerTeam && secondLastTouchTeam == scorerTeam) || (lastTouchTeam != scorerTeam && thirdLastTouchTeam == scorerTeam) || !cvarManager->getCvar("betterchat_unwanted_pass").getBoolValue()) {
		if (assist) { // Assist
			assist = false;
			map<string, bool> afterPassMsg = readJson("afterPass");
			for (const auto& pair : afterPassMsg) {
				if (pair.second) {
					whitelist.emplace_back(pair.first);
				}
			}
		}
	}
}

// Kick-off
void BetterChat::hitBall(CarWrapper car, void* params) {
	if (replayEnded == true) {
		replayEnded = false;
		LOG("[EVENT] Kick-off");
		resetWhitelist();
	}

	unsigned char touchTeam = car.GetPRI().GetTeamNum();

	if (lastTouchTeam == -1) {
		lastTouchTeam = touchTeam;
		secondLastTouchTeam = touchTeam;
		thirdLastTouchTeam = touchTeam;
	}
	else if(lastToucherID != car.GetPRI().GetPlayerID()) {
		thirdLastTouchTeam = secondLastTouchTeam;
		secondLastTouchTeam = lastTouchTeam;
		lastTouchTeam = touchTeam;

		lastToucherID = car.GetPRI().GetPlayerID();
	}

	if (save && chrono::system_clock::now() > lastSaveTime + chrono::seconds(cvarManager->getCvar("betterchat_delay").getIntValue())) {
		save = false;
		resetWhitelist();
	}
}

void BetterChat::onTimerUpdate() {
	if (save && chrono::system_clock::now() > lastSaveTime + chrono::seconds(cvarManager->getCvar("betterchat_delay").getIntValue())) {
		save = false;
		resetWhitelist();
	}
}

// Overtime
void BetterChat::onOvertimeStarted() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	LOG("[EVENT] Overtime");
	replayEnd();
}

// Replay end
void BetterChat::replayEnd() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	replayEnded = true;
	whitelist.clear();
	map<string, bool> beforeKickoffMsg = readJson("beforeKickoff");
	for (const auto& pair : beforeKickoffMsg) {
		if (pair.second) {
			whitelist.emplace_back(pair.first);
		}
	}
	lastTouchTeam = -1;
	secondLastTouchTeam = -1;
	thirdLastTouchTeam = -1;
	lastToucherID = -1;
}

// Game end
void BetterChat::gameEnd() {
	resetWhitelist();
	LOG("[EVENT] Game end");
	gameWrapper->RegisterDrawable(bind(&BetterChat::ShowToxicityScores, this, std::placeholders::_1));
}

/// <summary>
/// Display the table with the number of blocked messages per player during the game
/// </summary>
/// <param name="canvas"> Canvas Wrapper </param>
void BetterChat::ShowToxicityScores(CanvasWrapper canvas) {
	if (cvarManager->getCvar("betterchat_toxicityscores").getBoolValue() && cvarManager->getCvar("betterchat_enabled").getBoolValue()) {
		canvas.SetColor(LinearColor(255, 255, 255, 255));

		int x = cvarManager->getCvar("betterchat_score_X").getIntValue();
		int y = cvarManager->getCvar("betterchat_score_Y").getIntValue();

		canvas.SetPosition(Vector2({ x, y }));
		canvas.DrawString("Player names", 1, 1);

		canvas.SetPosition(Vector2({ x + 150, y }));
		canvas.DrawString("Messages blocked", 1, 1);

		canvas.SetPosition(Vector2({ x + 300, y }));
		canvas.DrawString("Ratio", 1, 1);

		// LinearColor blueTeam = LinearColor(0, 75, 255, 255);
		// LinearColor orangeTeam = LinearColor(255, 165, 0, 255);

		map<string, pInfos>::iterator player;
		for (player = playerInfo.begin(); player != playerInfo.end(); ++player) {

			LinearColor linearTeamColor = gameWrapper->GetOnlineGame().GetTeams().Get(player->second.teamNum).GetFontColor();
			linearTeamColor = LinearColor((int)(linearTeamColor.R * 255), (int)(linearTeamColor.G * 255), (int)(linearTeamColor.B * 255), 255);
			canvas.SetColor(linearTeamColor);

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
}

/// <summary>
/// Erase the table
/// </summary>
void BetterChat::gameDestroyed() {
	gameWrapper->UnregisterDrawables();
}

/// <summary>
/// Erase a chat message
/// </summary>
void BetterChat::handleMsg(bool cancel, std::string playerName) {
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_Chat_TA.OnChatMessage", [this, cancel, playerName](ActorWrapper Caller, void* params, ...) {
		FGFxChatMessage* Params = (FGFxChatMessage*)params;
		if(cancel) { // If the message has to be cancelled
			Params->PlayerName = FS("");
			Params->Message = FS("");
			Params->ChatChannel = 0;
			Params->TimeStamp = FS("");
		}
		if (playerInfo[playerName].numMsg == 1) { // If it is the first message sent by this player
			playerInfo[playerName].teamNum = (unsigned char)Params->Team;
		}
	});
}

/// <summary>
/// Determine if a quickchat message should be erased or not
/// </summary>
void BetterChat::chatMessageEvent(ActorWrapper caller, void* params) {
	if (params) {

		FHUDChatMessage* chatMessageParams = static_cast<FHUDChatMessage*>(params);
		if (chatMessageParams->PlayerName == nullptr) return;
		std::wstring player(chatMessageParams->PlayerName);
		std::string playerName(player.begin(), player.end());

		if (chatMessageParams->Message == nullptr) return;
		std::wstring message(chatMessageParams->Message);
		std::string msgID(message.begin(), message.end());

		if (!cvarManager->getCvar("betterchat_enabled").getBoolValue()) {
			handleMsg(false, playerName);
			return;
		}

		if (cvarManager->getCvar("betterchat_antispam").getBoolValue()) {
			delay = std::chrono::seconds(cvarManager->getCvar("betterchat_delay").getIntValue());
		}
		else {
			delay = std::chrono::seconds(0);
		}

		bool cancel = false;

		regex quickchat_pattern("^Group\\dMessage\\d\\d?$");
		if (regex_match(msgID, quickchat_pattern)) // If it is a quickchat
		{
			playerInfo[playerName].numMsg += 1;
			if (msgID != playerInfo[playerName].previousMsg) { // Different message
				playerInfo[playerName].previousMsg = msgID;
				playerInfo[playerName].previousTime = chrono::system_clock::now();
				if (cvarManager->getCvar("betterchat_chatfilter").getBoolValue() && std::find(whitelist.begin(), whitelist.end(), msgID) == whitelist.end()) { // If message not in whitelist
					cancel = true;
					playerInfo[playerName].blockedMsg += 1;
				}
			}
			else { // Same message
				if (std::chrono::system_clock::now() < playerInfo[playerName].previousTime + delay) { // If it is spam
					cancel = true;
					playerInfo[playerName].previousTime = chrono::system_clock::now();
					playerInfo[playerName].blockedMsg += 1;
				}
				else {
					playerInfo[playerName].previousTime = chrono::system_clock::now();
					if (cvarManager->getCvar("betterchat_chatfilter").getBoolValue() && std::find(whitelist.begin(), whitelist.end(), msgID) == whitelist.end()) { // Else, if message not in whitelist
						cancel = true;
						playerInfo[playerName].blockedMsg += 1;
					}
				}
			}
		}
		else {
			if (cvarManager->getCvar("betterchat_nowrittenmsg").getBoolValue()) {
				cancel = true;
			}
			if(cvarManager->getCvar("betterchat_writtenmsgastoxic").getBoolValue()) {
				if (cancel) playerInfo[playerName].blockedMsg += 1;
				playerInfo[playerName].numMsg += 1;
			}
		}
		handleMsg(cancel, playerName);
	}
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", [this](ActorWrapper caller, void* params, ...) {
		gameWrapper->UnhookEvent("Function TAGame.GFxData_Chat_TA.OnChatMessage");
	});
}