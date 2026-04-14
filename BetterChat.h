#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"

#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <variant>
#include <Windows.h>

using json = nlohmann::json;

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH);

class BetterChat: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow
{
	// Quickchats id <=> Quickchats texts
	std::map<std::string, std::string> idQuickchats = {
		{"Group1Message1", "I got it!"}, // Je l'ai !
		{"Group1Message2", "Need boost!"}, // Besoin de turbo !
		{"Group1Message3", "Take the shot!"}, // Prends-le !
		{"Group1Message4", "Defending."}, // Je défends.
		{"Group1Message5", "Go for it!"}, // Vas-y !
		{"Group1Message6", "Centering!"}, // Centre !
		{"Group1Message7", "All yours."}, // Il est pour toi.
		{"Group1Message8", "In position."}, // En position.
		{"Group1Message9", "Incoming!"}, // En approche !
		{"Group1Message10", "Faking."}, // La feinte.
		{"Group1Message11", "Bumping!"}, // Impact !
		{"Group1Message12", "On your left!"}, // Sur ta gauche !
		{"Group1Message13", "On your right!"}, // Sur ta droite !
		{"Group1Message14", "Passing!"}, // La passe !
		{"Group1Message15", "Rotating Up!"}, // Je monte !
		{"Group1Message16", "Rotating back!"}, // Je recule !
		{"Group1Message17", "You have time!"}, // Tu as le temps !
		{"Group1Message18", "One more"}, // Encore une !
		{"Group1Message19", "Back Post"}, // Poste arrière !
		{"Group1Message20", "Leaving"}, // Je pars.
		{"Group1Message21", "Oops, wrong quickchat!"}, // Oups, mauvais message !

		{"Group2Message1", "Nice shot!"}, // Beau tir !
		{"Group2Message2", "Great pass!"}, // Belle passe !
		{"Group2Message3", "Thanks!"}, // Merci !
		{"Group2Message4", "What a save!"}, // Quel arrêt !
		{"Group2Message5", "Nice one!"}, // Bien vu !
		{"Group2Message6", "What a play!"}, // Quelle intensité !
		{"Group2Message7", "Great clear!"}, // Beau dégagement !
		{"Group2Message8", "Nice block!"}, // Super blocage !
		{"Group2Message9", "Nice bump!"}, // Bel impact !
		{"Group2Message10", "Nice demo!"}, // Jolie démo !
		{"Group2Message11", "We got this."}, // On assure !

		{"Group3Message1", "OMG!"}, // Oh mon dieu !
		{"Group3Message2", "Noooo!"}, // Noooon !
		{"Group3Message3", "Wow!"}, // Wow !
		{"Group3Message4", "Close one..."}, // C'était pas loin...
		{"Group3Message5", "No way!"}, // Pas possible !
		{"Group3Message6", "Holy cow!"}, // Sérieux ?!
		{"Group3Message7", "Whew."}, // Waouh.
		{"Group3Message8", "Siiiick!"}, // Truc de ouf !
		{"Group3Message9", "Calculated."}, // C'est prévu.
		{"Group3Message10", "Brutal!"}, // Brutal !
		{"Group3Message11", "Okay."}, // Ok.
		{"Group3Message12", "Yes!"}, // Oui !
		{"Group3Message13", "Unlucky!"}, // Pas de chance !
		{"Group3Message14", "This isn't Rocket League!"}, // C'est pas Rocket League ça !
		{"Group3Message15", "Oof."}, // Oof.
		{"Group3Message16", "..."}, // ...
		{"Group3Message17", "Be careful!"}, // Attention !
		{"Group3Message18", "Let's lock in"}, // On se concentre.

		{"Group4Message1", "$#@%!"}, // $#@%!
		{"Group4Message2", "No problem."}, // Pas de problèmes.
		{"Group4Message3", "Whoops..."}, // Oups...
		{"Group4Message4", "Sorry!"}, // Désolé !
		{"Group4Message5", "My bad..."}, // Pardon...
		{"Group4Message6", "Oops!"}, // Oups !
		{"Group4Message7", "My fault."}, // Ma faute.
		{"Group4Message8", "All good!"}, // Tout va bien !

		{"Group5Message1", "gg"}, // gg
		{"Group5Message2", "Well played."}, // Bien joué.
		{"Group5Message3", "That was fun!"}, // C'était cool !
		{"Group5Message4", "Rematch!"}, // On remet ça !
		{"Group5Message5", "One. More. Game."}, // Encore. Une. Partie.
		{"Group5Message6", "What a game!"}, // Quelle partie !
		{"Group5Message7", "Nice moves!"}, // Super déplacements !
		{"Group5Message8", "Everybody dance!"}, // Que tout le monde dance !
		{"Group5Message9", "Party Up?"}, // On groupe ?

		{"Group6Message4", "This is Rocket League!"}, // Ça c'est Rocket League !
	};

	// Default config
	json defaultFilterConfig = {
		{"Default config", {
			{"afterAlliedGoal", {
				{"Group1Message1", true},
				{"Group1Message10", false},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", false},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", true},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", false},
				{"Group2Message3", true},
				{"Group2Message4", false},
				{"Group2Message5", true},
				{"Group2Message6", true},
				{"Group2Message7", false},
				{"Group2Message8", false},
				{"Group2Message9", true},
				{"Group3Message1", true},
				{"Group3Message10", true},
				{"Group3Message11", false},
				{"Group3Message12", true},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", true},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", true},
				{"Group3Message4", false},
				{"Group3Message5", true},
				{"Group3Message6", true},
				{"Group3Message7", false},
				{"Group3Message8", true},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", true}
			}},
			{"afterEnemyGoal", {
				{"Group1Message1", true},
				{"Group1Message10", false},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", false},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", true},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", false},
				{"Group2Message3", true},
				{"Group2Message4", false},
				{"Group2Message5", true},
				{"Group2Message6", true},
				{"Group2Message7", false},
				{"Group2Message8", false},
				{"Group2Message9", true},
				{"Group3Message1", false},
				{"Group3Message10", false},
				{"Group3Message11", false},
				{"Group3Message12", false},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", false},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", false},
				{"Group3Message4", false},
				{"Group3Message5", false},
				{"Group3Message6", false},
				{"Group3Message7", false},
				{"Group3Message8", false},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", false}
			}},
			{"afterPass", {
				{"Group1Message1", true},
				{"Group1Message10", false},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", false},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", true},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", true},
				{"Group2Message3", true},
				{"Group2Message4", false},
				{"Group2Message5", true},
				{"Group2Message6", true},
				{"Group2Message7", false},
				{"Group2Message8", false},
				{"Group2Message9", true},
				{"Group3Message1", false},
				{"Group3Message10", false},
				{"Group3Message11", false},
				{"Group3Message12", true},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", true},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", false},
				{"Group3Message4", false},
				{"Group3Message5", false},
				{"Group3Message6", false},
				{"Group3Message7", false},
				{"Group3Message8", false},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", false}
			}},
			{"afterSave", {
				{"Group1Message1", true},
				{"Group1Message10", false},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", true},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", false},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", false},
				{"Group2Message3", true},
				{"Group2Message4", true},
				{"Group2Message5", false},
				{"Group2Message6", false},
				{"Group2Message7", true},
				{"Group2Message8", true},
				{"Group2Message9", false},
				{"Group3Message1", false},
				{"Group3Message10", false},
				{"Group3Message11", false},
				{"Group3Message12", true},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", false},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", false},
				{"Group3Message4", false},
				{"Group3Message5", false},
				{"Group3Message6", false},
				{"Group3Message7", false},
				{"Group3Message8", false},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", false}
			}},
			{"beforeKickoff", {
				{"Group1Message1", true},
				{"Group1Message10", true},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", true},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", false},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", false},
				{"Group2Message3", false},
				{"Group2Message4", false},
				{"Group2Message5", false},
				{"Group2Message6", false},
				{"Group2Message7", false},
				{"Group2Message8", false},
				{"Group2Message9", false},
				{"Group3Message1", false},
				{"Group3Message10", false},
				{"Group3Message11", false},
				{"Group3Message12", true},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", false},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", false},
				{"Group3Message4", false},
				{"Group3Message5", false},
				{"Group3Message6", false},
				{"Group3Message7", false},
				{"Group3Message8", false},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", false}
			}},
			{"default", {
				{"Group1Message1", true},
				{"Group1Message10", false},
				{"Group1Message11", true},
				{"Group1Message12", true},
				{"Group1Message13", true},
				{"Group1Message14", true},
				{"Group1Message15", true},
				{"Group1Message16", true},
				{"Group1Message17", true},
				{"Group1Message18", true},
				{"Group1Message19", true},
				{"Group1Message20", true},
				{"Group1Message21", true},
				{"Group1Message2", true},
				{"Group1Message3", true},
				{"Group1Message4", true},
				{"Group1Message5", true},
				{"Group1Message6", true},
				{"Group1Message7", true},
				{"Group1Message8", true},
				{"Group1Message9", true},
				{"Group2Message1", false},
				{"Group2Message10", false},
				{"Group2Message11", true},
				{"Group2Message2", false},
				{"Group2Message3", false},
				{"Group2Message4", false},
				{"Group2Message5", false},
				{"Group2Message6", false},
				{"Group2Message7", false},
				{"Group2Message8", false},
				{"Group2Message9", false},
				{"Group3Message1", false},
				{"Group3Message10", false},
				{"Group3Message11", false},
				{"Group3Message12", true},
				{"Group3Message13", false},
				{"Group3Message14", true},
				{"Group3Message15", false},
				{"Group3Message16", false},
				{"Group3Message17", true},
				{"Group3Message18", true},
				{"Group3Message2", false},
				{"Group3Message3", false},
				{"Group3Message4", false},
				{"Group3Message5", false},
				{"Group3Message6", false},
				{"Group3Message7", false},
				{"Group3Message8", false},
				{"Group3Message9", false},
				{"Group4Message1", false},
				{"Group4Message2", true},
				{"Group4Message3", true},
				{"Group4Message4", true},
				{"Group4Message5", true},
				{"Group4Message6", true},
				{"Group4Message7", true},
				{"Group4Message8", true},
				{"Group5Message1", true},
				{"Group5Message2", true},
				{"Group5Message3", true},
				{"Group5Message4", true},
				{"Group5Message5", true},
				{"Group5Message6", true},
				{"Group5Message7", true},
				{"Group5Message8", true},
				{"Group5Message9", true},
				{"Group6Message4", false}
			}}
		}}
	};

	// Structs
	struct BetterChatParams {
		bool antispam;
		bool chatfilter;
		int antispam_delay;
		bool nowrittenmsg;
		bool writtenmsgastoxic;
		int aftersavetime;
		bool owngoal;
		bool save_logs;
		bool unwanted_pass;
		bool block_custom_msg;
		bool toxicityscores;
	};

	struct StatTickerParams {
		uintptr_t Receiver;
		uintptr_t Victim;
		uintptr_t StatEvent;
	};

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
				int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(), nullptr, 0, nullptr, nullptr);
				std::string str(size_needed, 0);
				WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(), &str[0], size_needed, nullptr, nullptr);
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

	struct FSceNpOnlineId
	{
		uint64_t                                           Data[0x2];                                        		// 0x0000 (0x0010) [0x0000000000000000]               
		uint8_t                                            Term;                                             		// 0x0010 (0x0001) [0x0000000000000000]               
		uint8_t                                            Dummy[0x3];                                       		// 0x0011 (0x0003) [0x0000000000000000]               
	};

	struct FSceNpId
	{
		struct FSceNpOnlineId                              Handle;                                           		// 0x0000 (0x0018) [0x0000000000000002] (CPF_Const)   
		uint64_t                                           Opt;                                              		// 0x0018 (0x0008) [0x0000000000000002] (CPF_Const)   
		uint64_t                                           Reserved;                                         		// 0x0020 (0x0008) [0x0000000000000002] (CPF_Const)   
	};

	struct FUniqueNetId
	{
		uint64_t                                           Uid;                                              		// 0x0000 (0x0008) [0x0000000000000000]               
		struct FSceNpId                                    NpId;                                             		// 0x0008 (0x0028) [0x0000000000000000]               
		struct FString                                     EpicAccountId;                                    		// 0x0030 (0x0010) [0x0000000000400000] (CPF_NeedCtorLink)
		uint8_t                                            Platform;                                         		// 0x0040 (0x0001) [0x0000000000000000]               
		uint8_t                                            SplitscreenID;                                    		// 0x0041 (0x0001) [0x0000000000000000]               
	};

	struct FHUDChatMessage
	{
		class APlayerReplicationInfo* PRI;
		class ATeam_TA* Team;               
		struct FString PlayerName;
		struct FString Message;
		uint8_t ChatChannel;               
		unsigned long bPreset : 1; 
		struct FUniqueNetId Recipient;
	};

	struct FGFxChatMessage {
		int32_t Team;
		class FString PlayerName;
		class FString Message;
		uint8_t ChatChannel;
		bool bLocalPlayer : 1;
		struct FUniqueNetId SenderID;
		uint8_t MessageType;
		class FString TimeStamp;
	};

	//Functions

	void resetWhitelist();

	// -- JSON functions --
	void jsonFileExists();
	void createConfigInJson(std::string configName);
	void deleteConfigInJson(std::string configName);
	std::list<std::string> getConfigsListInJson();
	std::map<std::string, std::string> getConfigByGamemodeInJson();
	void editConfigByGamemodeInJson(std::string gamemode, std::string config);
	std::map<std::string, bool> readMapInJson(std::string config, std::string category);
	void toggleQuickchatInJson(std::string config, std::string category, std::string idMsg);
	BetterChatParams getParamsInJson(std::string config);
	void editParamInJson(std::string config, std::string param, std::variant<bool, int> value);
	std::list<std::string> getBannedWordsInJson(std::string config);
	void addBannedWordInJson(std::string config, std::string word);
	void removeBannedWordInJson(std::string config, std::string word);

	virtual void onLoad();
	virtual void onUnload();
	
	void onNewGame();
	void setConfig();
	void refreshConfig();
	void gameBegin();
	void gameEnd();
	void chatMessageEvent(ActorWrapper caller, void* args);
	void onStatTickerMessage(ServerWrapper caller, void* args);
	void hitBall(CarWrapper car, void* params);
	void addKickoffMessages();
	void onTimerUpdate();
	void onGoal();
	void onOvertimeStarted();

	void onMessage(ActorWrapper Caller, void* params);

	void gameDestroyed();
	void ShowToxicityScores(CanvasWrapper canvas);

	// Interface
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};