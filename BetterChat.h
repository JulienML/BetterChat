#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"

#include <set>
#include <string>
#include <variant>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH);

class BetterChat: public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow
{
	// Quickchats id <=> Quickchats texts
	std::map<std::string, std::string> idQuickchats = {
		{"Group1Message1", "I got it!"}, // Je l'ai !
		{"Group1Message2", "Need boost!"}, // Besoin de turbo !
		{"Group1Message3", "Take the shot!"}, // Prends-le !
		{"Group1Message4", "Defending."}, // Je d�fends.
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

		{"Group2Message1", "Nice shot!"}, // Beau tir !
		{"Group2Message2", "Great pass!"}, // Belle passe !
		{"Group2Message3", "Thanks!"}, // Merci !
		{"Group2Message4", "What a save!"}, // Quel arr�t !
		{"Group2Message5", "Nice one!"}, // Bien vu !
		{"Group2Message6", "What a play!"}, // Quelle intensit� !
		{"Group2Message7", "Great clear!"}, // Beau d�gagement !
		{"Group2Message8", "Nice block!"}, // Super blocage !
		{"Group2Message9", "Nice bump!"}, // Bel impact !
		{"Group2Message10", "Nice demo!"}, // Jolie d�mo !
		{"Group2Message11", "We got this."}, // On assure !

		{"Group3Message1", "OMG!"}, // Oh mon dieu !
		{"Group3Message2", "Noooo!"}, // Noooon !
		{"Group3Message3", "Wow!"}, // Wow !
		{"Group3Message4", "Close one..."}, // C'�tait pas loin...
		{"Group3Message5", "No way!"}, // Pas possible !
		{"Group3Message6", "Holy cow!"}, // S�rieux ?!
		{"Group3Message7", "Whew."}, // Waouh.
		{"Group3Message8", "Siiiick!"}, // Truc de ouf !
		{"Group3Message9", "Calculated."}, // C'est pr�vu.
		{"Group3Message10", "Savage!"}, // Sauvage !
		{"Group3Message11", "Okay."}, // Ok.
		{"Group3Message12", "Yes!"}, // Oui !

		{"Group4Message1", "$#@%!"}, // $#@%!
		{"Group4Message2", "No problem."}, // Pas de probl�mes.
		{"Group4Message3", "Whoops..."}, // Oups...
		{"Group4Message4", "Sorry!"}, // D�sol� !
		{"Group4Message5", "My bad..."}, // Pardon...
		{"Group4Message6", "Oops!"}, // Oups !
		{"Group4Message7", "My fault."}, // Ma faute.

		{"Group5Message1", "gg"}, // gg
		{"Group5Message2", "Well played."}, // Bien jou�.
		{"Group5Message3", "That was fun!"}, // C'�tait cool !
		{"Group5Message4", "Rematch!"}, // On remet �a !
		{"Group5Message5", "One. More. Game."}, // Encore. Une. Partie.
		{"Group5Message6", "What a game!"}, // Quelle partie !
		{"Group5Message7", "Nice moves!"}, // Super d�placements !
		{"Group5Message8", "Everybody dance!"}, // Que tout le monde dance !
		{"Group5Message9", "Party Up?"}, // On groupe ?

		{"Group6Message4", "This is Rocket League!"}, // �a c'est Rocket League !
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
		bool unwanted_pass;
		bool toxicityscores;
	};

	struct StatTickerParams {
		uintptr_t Receiver;
		uintptr_t Victim;
		uintptr_t StatEvent;
	};

	struct FHUDChatMessage
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

	virtual void onLoad();
	virtual void onUnload();

	void handleMsg(bool cancel, std::string playerName);
	
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

	void gameDestroyed();
	void ShowToxicityScores(CanvasWrapper canvas);

	// Interface
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};