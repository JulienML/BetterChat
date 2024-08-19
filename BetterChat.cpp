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

map<string, pInfos> playersInfos;
chrono::seconds delay;

chrono::system_clock::time_point lastSaveTime;
bool save;

unsigned char numTeam;
int blueScore;
int orangeScore;
bool gameInProgress;
bool waitingForKickoff;
bool goal;
bool assist;
unsigned char lastTouchTeam;
unsigned char secondLastTouchTeam;
unsigned char thirdLastTouchTeam;

int lastToucherID;

string gamemode;
string config;

void BetterChat::onLoad()
{
	_globalCvarManager = cvarManager;

	cvarManager->registerCvar("betterchat_enabled", "1", "Enable BetterChat Plugin", true, true, 0, true, 1);

	cvarManager->registerCvar("betterchat_score_X", "1530", "", true, true, 0, true, 1920);
	cvarManager->registerCvar("betterchat_score_Y", "45", "", true, true, 0, true, 1080);

	jsonFileExists();

	if (gameWrapper->IsInOnlineGame()) {
		setConfig();
		resetWhitelist();
		gameInProgress = true;
	}
	else {
		gameInProgress = false;
	}

	gameWrapper->HookEventWithCaller<ActorWrapper>("Function GameEvent_TA.Countdown.BeginState", bind(&BetterChat::gameBegin, this));
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", bind(&BetterChat::onStatTickerMessage, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", bind(&BetterChat::chatMessageEvent, this, std::placeholders::_1, std::placeholders::_2));
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Replay_TA.StopPlayback", bind(&BetterChat::addKickoffMessages, this));
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

// Reset the whitelist
void BetterChat::resetWhitelist() {
	LOG("Messages have been reset");
	whitelist.clear();
	map<string, bool> defaultMsg = readMapInJson(config, "default");
	for (const auto& pair : defaultMsg) {
		if (pair.second) {
			whitelist.emplace_back(pair.first);
		}
	}
}

// Check if the json file exists and updates it if necessary. If not, it creates it.
void BetterChat::jsonFileExists() {
	if (!filesystem::exists(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json")) {

		// Default config
		ofstream NewFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		NewFile << setw(4) << "{\"Default config\":{\"afterAlliedGoal\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":true,\"Group3Message10\":true,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":true,\"Group3Message4\":false,\"Group3Message5\":true,\"Group3Message6\":true,\"Group3Message7\":false,\"Group3Message8\":true,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":true},\"afterEnemyGoal\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"afterPass\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":true,\"Group2Message10\":false,\"Group2Message2\":true,\"Group2Message3\":true,\"Group2Message4\":false,\"Group2Message5\":true,\"Group2Message6\":true,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":true,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"afterSave\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":true,\"Group2Message4\":true,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":true,\"Group2Message8\":true,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"beforeKickoff\":{\"Group1Message1\":true,\"Group1Message10\":true,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":false,\"Group2Message4\":false,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false},\"default\":{\"Group1Message1\":true,\"Group1Message10\":false,\"Group1Message11\":true,\"Group1Message12\":true,\"Group1Message13\":true,\"Group1Message14\":true,\"Group1Message2\":true,\"Group1Message3\":true,\"Group1Message4\":true,\"Group1Message5\":true,\"Group1Message6\":true,\"Group1Message7\":true,\"Group1Message8\":true,\"Group1Message9\":true,\"Group2Message1\":false,\"Group2Message10\":false,\"Group2Message2\":false,\"Group2Message3\":false,\"Group2Message4\":false,\"Group2Message5\":false,\"Group2Message6\":false,\"Group2Message7\":false,\"Group2Message8\":false,\"Group2Message9\":false,\"Group3Message1\":false,\"Group3Message10\":false,\"Group3Message11\":false,\"Group3Message2\":false,\"Group3Message3\":false,\"Group3Message4\":false,\"Group3Message5\":false,\"Group3Message6\":false,\"Group3Message7\":false,\"Group3Message8\":false,\"Group3Message9\":false,\"Group4Message1\":false,\"Group4Message2\":true,\"Group4Message3\":true,\"Group4Message4\":true,\"Group4Message5\":true,\"Group4Message6\":true,\"Group4Message7\":true,\"Group5Message1\":true,\"Group5Message2\":true,\"Group5Message3\":true,\"Group5Message4\":true,\"Group5Message5\":true,\"Group5Message6\":true,\"Group5Message7\":true,\"Group5Message8\":true,\"Group6Message4\":false}}}" << endl;
		NewFile.close();

		ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		json jsonData;
		file >> jsonData;
		file.close();

		for (const auto& pair : BetterChat::idQuickchats) {
			if (jsonData["Default config"]["default"].find(pair.first) == jsonData["Default config"]["default"].end()) {
				jsonData["Default config"]["default"][pair.first] = true;
			}

			list<string> events = { "beforeKickoff", "afterAlliedGoal", "afterEnemyGoal", "afterPass", "afterSave" };
			for (const string event : events) {
				if (jsonData["Default config"][event].find(pair.first) == jsonData["Default config"][event].end()) {
					jsonData["Default config"][event][pair.first] = false;
				}
			}
		}

		// Parameters
		jsonData["Default config"]["params"]["antispam"] = true;
		jsonData["Default config"]["params"]["chatfilter"] = true;
		jsonData["Default config"]["params"]["antispam_delay"] = 5;
		jsonData["Default config"]["params"]["nowrittenmsg"] = false;
		jsonData["Default config"]["params"]["writtenmsgastoxic"] = false;
		jsonData["Default config"]["params"]["aftersavetime"] = 5;
		jsonData["Default config"]["params"]["owngoal"] = false;
		jsonData["Default config"]["params"]["unwanted_pass"] = false;
		jsonData["Default config"]["params"]["toxicityscores"] = true;

		// Config by gamemode
		jsonData["ConfigByGamemode"]["1v1"] = "Default config";
		jsonData["ConfigByGamemode"]["2v2"] = "Default config";
		jsonData["ConfigByGamemode"]["3v3"] = "Default config";
		jsonData["ConfigByGamemode"]["Tournament"] = "Default config";
		jsonData["ConfigByGamemode"]["Extra"] = "Default config";

		ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		outputFile << std::setw(4) << jsonData << endl;
		outputFile.close();
	}
	else {
		ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
		json jsonData;
		file >> jsonData;
		file.close();

		if (!jsonData.contains("Default config")) { // If the file version is older than BetterChat v2.2.1
			string path = gameWrapper->GetDataFolder().string() + "/BetterChat_config.json";
			remove(path.c_str());
			jsonFileExists();
		}

		if (!jsonData.contains("ConfigByGamemode")) { // If the file version is older than BetterChat v3.0.0
			jsonData["ConfigByGamemode"]["1v1"] = "Default config";
			jsonData["ConfigByGamemode"]["2v2"] = "Default config";
			jsonData["ConfigByGamemode"]["3v3"] = "Default config";
			jsonData["ConfigByGamemode"]["Tournament"] = "Default config";
			jsonData["ConfigByGamemode"]["Extra"] = "Default config";

			ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
			outputFile << std::setw(4) << jsonData << endl;
			outputFile.close();
		}
	}
	if (filesystem::exists(gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json")) { // Deletes the file used before BetterChat v2.0.0
		string path = gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json";
		remove(path.c_str());
	}
}

// Create a new config in the json file
void BetterChat::createConfigInJson(string configName) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;
	file.close();

	jsonData[configName] = jsonData["Default config"];

	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
}

// Delete a config in the json file
void BetterChat::deleteConfigInJson(string configName) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;
	file.close();

	jsonData.erase(configName);

	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
}

// Get the list of configs in the json file
list<string> BetterChat::getConfigsListInJson() {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;
	file.close();

	list<string> configs = { "Default config" };
	
	for (json::iterator it = jsonData.begin(); it != jsonData.end(); ++it) {
		if (it.key() != "ConfigByGamemode" && it.key() != "Default config") {
			configs.emplace_back(it.key());
		}
	}

	return configs;
}

map<string, string> BetterChat::getConfigByGamemodeInJson() {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;

	map<string, string> configByGamemode = jsonData["ConfigByGamemode"];
	return configByGamemode;
}

// Change the plugin config for a specific gamemode
void BetterChat::editConfigByGamemodeInJson(string gamemode, string config) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;
	file.close();

	jsonData["ConfigByGamemode"][gamemode] = config;

	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
}

// Return the message/boolean map of the specified category
map<string, bool> BetterChat::readMapInJson(string config, string category) {
	ifstream file(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	file >> jsonData;

	map<string, bool> messages = jsonData[config][category];
	return messages;
}

// Allow/Forbid a quickchat message in the json file
void BetterChat::toggleQuickchatInJson(string config, string category, string idMsg) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();

	jsonData[config][category][idMsg] = !jsonData[config][category][idMsg];
	
	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
	LOG("\"" + BetterChat::idQuickchats[idMsg] + "\" is now " + (jsonData[config][category][idMsg] ? "allowed" : "forbidden") + " in '" + category + "' category of the '" + config + "' configuration.");
}

// Get a parameter in the json file
BetterChat::BetterChatParams BetterChat::getParamsInJson(string config) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();

	BetterChatParams params{};
	params.antispam = jsonData[config]["params"]["antispam"];
	params.chatfilter = jsonData[config]["params"]["chatfilter"];
	params.antispam_delay = jsonData[config]["params"]["antispam_delay"];
	params.nowrittenmsg = jsonData[config]["params"]["nowrittenmsg"];
	params.writtenmsgastoxic = jsonData[config]["params"]["writtenmsgastoxic"];
	params.aftersavetime = jsonData[config]["params"]["aftersavetime"];
	params.owngoal = jsonData[config]["params"]["owngoal"];
	params.unwanted_pass = jsonData[config]["params"]["unwanted_pass"];
	params.toxicityscores = jsonData[config]["params"]["toxicityscores"];

	return params;
}

// Edit a parameter in the json file
void BetterChat::editParamInJson(string config, string param, std::variant<bool, int> value) {
	ifstream inputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	json jsonData;
	inputFile >> jsonData;
	inputFile.close();
	
	if (value.index() == 0) {
		jsonData[config]["params"][param] = std::get<bool>(value);
	}
	else if (value.index() == 1) {
		jsonData[config]["params"][param] = std::get<int>(value);
	}

	ofstream outputFile(gameWrapper->GetDataFolder().string() + "/BetterChat_config.json");
	outputFile << std::setw(4) << jsonData << endl;
	outputFile.close();
}

void BetterChat::refreshConfig() {
	config = getConfigByGamemodeInJson()[gamemode];
	LOG("Config changed: " + config);
}

// Check the gamemode and apply the corresponding plugin configuration
void BetterChat::setConfig() {
	if (!gameWrapper->IsInOnlineGame()) { return; }

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	GameSettingPlaylistWrapper playlistWrapper = server.GetPlaylist();
	UnrealStringWrapper playlistTitle = playlistWrapper.GetTitle();

	if (!playlistTitle) {
		LOG("Gamemode not recognized, set to 'Extra' by default");
		gamemode = "Extra";
	}
	else {
		string playlist = playlistTitle.ToString();
		if (playlist == "Duel" || playlist == "Duel solo") {
			gamemode = "1v1";
		}
		else if (playlist == "Double") {
			gamemode = "2v2";
		}
		else if (playlist == "Standard") {
			gamemode = "3v3";
		}
		else if (playlist == "Tournament") {
			gamemode = "Tournament";
		}
		else {
			gamemode = "Extra";
		}
	}

	config = getConfigByGamemodeInJson()[gamemode];

	LOG("Gamemode: " + gamemode);
	LOG("Config: " + config);
}

// Game begin
void BetterChat::gameBegin() {
	if (!gameWrapper->IsInOnlineGame()) { return; }
	gameWrapper->UnregisterDrawables();

	if (gameInProgress) { return; }
	else {
		gameInProgress = true;

		setConfig();

		resetWhitelist();
		playersInfos.clear();
		LOG("[EVENT] Game start");

		numTeam = gameWrapper->GetLocalCar().GetPRI().GetTeamNum();
		blueScore = 0;
		orangeScore = 0;
		waitingForKickoff = true;
		addKickoffMessages();
		goal = false;
		assist = false;
		lastTouchTeam = -1;
		secondLastTouchTeam = -1;
		thirdLastTouchTeam = -1;
		lastToucherID = -1;
		lastSaveTime = chrono::system_clock::now();
		save = false;
	}
}

// Event
void BetterChat::onStatTickerMessage(ServerWrapper caller, void* params) {
	StatTickerParams* pstats = (StatTickerParams*)params;
	StatEventWrapper event = StatEventWrapper(pstats->StatEvent);

	std::string name = event.GetEventName();

	if (!gameWrapper->IsInOnlineGame() || !gameInProgress) { return; }

	ServerWrapper server = gameWrapper->GetCurrentGameState();

	if (name == "Goal") { // Goal
		goal = true;
		save = false;
	}
	else if (name == "Save" || name == "EpicSave") { // Save
		whitelist.clear();
		LOG("[EVENT] Save");
		map<string, bool> afterSaveMsg = readMapInJson(config, "afterSave");
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
	if (!gameWrapper->IsInOnlineGame() || !goal || !gameInProgress) { return; }
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

	BetterChatParams pluginParams = getParamsInJson(config);

	if (scorerTeam == lastTouchTeam || !pluginParams.owngoal) {
		if (scorerTeam == numTeam) { // Allied goal
			LOG("[EVENT] Allied goal");
			map<string, bool> afterAlliedGoalMsg = readMapInJson(config, "afterAlliedGoal");
			for (const auto& pair : afterAlliedGoalMsg) {
				if (pair.second) {
					whitelist.emplace_back(pair.first);
				}
			}
		}
		else { // Enemy goal
			LOG("[EVENT] Enemy goal");
			map<string, bool> afterEnemyGoalMsg = readMapInJson(config, "afterEnemyGoal");
			for (const auto& pair : afterEnemyGoalMsg) {
				if (pair.second) {
					whitelist.emplace_back(pair.first);
				}
			}
		}
	}

	if ((lastTouchTeam == scorerTeam && secondLastTouchTeam == scorerTeam) || (lastTouchTeam != scorerTeam && thirdLastTouchTeam == scorerTeam) || !pluginParams.unwanted_pass) {
		if (assist) { // Assist
			assist = false;
			map<string, bool> afterPassMsg = readMapInJson(config, "afterPass");
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
	if (!gameWrapper->IsInOnlineGame() || !gameInProgress) { return; }

	if (waitingForKickoff == true) {
		waitingForKickoff = false;
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

	if (save && chrono::system_clock::now() > lastSaveTime + chrono::seconds(getParamsInJson(config).aftersavetime)) {
		save = false;
		resetWhitelist();
	}
}

void BetterChat::onTimerUpdate() {
	if (!gameWrapper->IsInOnlineGame() || !gameInProgress) { return; }
	if (save && chrono::system_clock::now() > lastSaveTime + chrono::seconds(getParamsInJson(config).aftersavetime)) {
		save = false;
		resetWhitelist();
	}
}

// Overtime
void BetterChat::onOvertimeStarted() {
	if (!gameWrapper->IsInOnlineGame() || !gameInProgress) { return; }
	LOG("[EVENT] Overtime");
	whitelist.clear();
	addKickoffMessages();
}

// Replay end
void BetterChat::addKickoffMessages() {
	if (!gameWrapper->IsInOnlineGame() || !gameInProgress) { return; }
	waitingForKickoff = true;
	map<string, bool> beforeKickoffMsg = readMapInJson(config, "beforeKickoff");
	for (const auto& pair : beforeKickoffMsg) {
		if (std::find(whitelist.begin(), whitelist.end(), pair.first) == whitelist.end()) {
			if (pair.second) whitelist.emplace_back(pair.first);
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
	gameInProgress = false;
	LOG("[EVENT] Game end");
	gameWrapper->RegisterDrawable(bind(&BetterChat::ShowToxicityScores, this, std::placeholders::_1));
}

// Display the table with the number of blocked messages per player during the game
void BetterChat::ShowToxicityScores(CanvasWrapper canvas) {
	if (getParamsInJson(config).chatfilter && getParamsInJson(config).toxicityscores && cvarManager->getCvar("betterchat_enabled").getBoolValue()) {
		canvas.SetColor(LinearColor(255, 255, 255, 255));

		int x = cvarManager->getCvar("betterchat_score_X").getIntValue();
		int y = cvarManager->getCvar("betterchat_score_Y").getIntValue();

		canvas.SetPosition(Vector2({ x, y }));
		canvas.DrawString("Player names", 1, 1);

		canvas.SetPosition(Vector2({ x + 150, y }));
		canvas.DrawString("Blocked messages", 1, 1);

		canvas.SetPosition(Vector2({ x + 300, y }));
		canvas.DrawString("Ratio", 1, 1);

		map<string, pInfos>::iterator player;
		for (player = playersInfos.begin(); player != playersInfos.end(); ++player) {

			LinearColor linearTeamColor = gameWrapper->GetOnlineGame().GetTeams().Get(player->second.teamNum).GetFontColor();
			linearTeamColor = LinearColor((int)(linearTeamColor.R * 255), (int)(linearTeamColor.G * 255), (int)(linearTeamColor.B * 255), 255);
			canvas.SetColor(linearTeamColor);

			canvas.SetPosition(Vector2({ x, y + 20 * (int)distance(playersInfos.begin(), playersInfos.find(player->first)) + 20 }));
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

			canvas.SetPosition(Vector2({ x + 150, y + 20 * (int)distance(playersInfos.begin(), playersInfos.find(player->first)) + 20 }));
			canvas.DrawString(to_string(player->second.blockedMsg) + "/" + to_string(player->second.numMsg), 1, 1);

			canvas.SetPosition(Vector2({ x + 300, y + 20 * (int)distance(playersInfos.begin(), playersInfos.find(player->first)) + 20 }));
			canvas.DrawString(score, 1, 1);
		}
	}
}

// Erase the table
void BetterChat::gameDestroyed() {
	gameInProgress = false;
	gameWrapper->UnregisterDrawables();
}

// Erase a message if necessary
void BetterChat::handleMsg(bool cancel, std::string playerName) {
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_Chat_TA.OnChatMessage", [this, cancel, playerName](ActorWrapper Caller, void* params, ...) {
		FGFxChatMessage* Params = (FGFxChatMessage*)params;
		if(cancel) { // If the message has to be cancelled
			Params->PlayerName = FS("");
			Params->Message = FS("");
			Params->ChatChannel = 0;
			Params->TimeStamp = FS("");
		}
		if (playersInfos[playerName].numMsg == 1) { // If it is the first message sent by this player
			playersInfos[playerName].teamNum = (unsigned char)Params->Team;
		}
	});
}

// Determine if a quickchat message should be erased or not
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

		BetterChatParams pluginParams = getParamsInJson(config);

		if (pluginParams.antispam) {
			delay = std::chrono::seconds(pluginParams.antispam_delay);
		}
		else {
			delay = std::chrono::seconds(0);
		}

		bool cancel = false;

		regex quickchat_pattern("^Group\\dMessage\\d\\d?$");
		if (regex_match(msgID, quickchat_pattern)) // If it is a quickchat
		{
			playersInfos[playerName].numMsg += 1;
			if (msgID != playersInfos[playerName].previousMsg) { // Different message
				playersInfos[playerName].previousMsg = msgID;
				playersInfos[playerName].previousTime = chrono::system_clock::now();
				if (pluginParams.chatfilter && std::find(whitelist.begin(), whitelist.end(), msgID) == whitelist.end()) { // If message not in whitelist
					cancel = true;
					playersInfos[playerName].blockedMsg += 1;
				}
			}
			else { // Same message
				if (std::chrono::system_clock::now() < playersInfos[playerName].previousTime + delay) { // If it is spam
					cancel = true;
					playersInfos[playerName].previousTime = chrono::system_clock::now();
					playersInfos[playerName].blockedMsg += 1;
				}
				else {
					playersInfos[playerName].previousTime = chrono::system_clock::now();
					if (pluginParams.chatfilter && std::find(whitelist.begin(), whitelist.end(), msgID) == whitelist.end()) { // Else, if message not in whitelist
						cancel = true;
						playersInfos[playerName].blockedMsg += 1;
					}
				}
			}
		}
		else {
			if (pluginParams.nowrittenmsg) {
				cancel = true;
			}
			if(pluginParams.writtenmsgastoxic) {
				if (cancel) playersInfos[playerName].blockedMsg += 1;
				playersInfos[playerName].numMsg += 1;
			}
		}
		handleMsg(cancel, playerName);
	}

	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage", [this](ActorWrapper caller, void* params, ...) {
		gameWrapper->UnhookEvent("Function TAGame.GFxData_Chat_TA.OnChatMessage");
	});
}