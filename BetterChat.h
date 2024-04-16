#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"

#include <set>
#include <string>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

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

		{"Group3Message1", "OMG!"}, // Oh mon dieu !
		{"Group3Message2", "Noooo!"}, // Noooon !
		{"Group3Message3", "Wow!"}, // Wow !
		{"Group3Message4", "Close one..."}, // C'était pas loin...
		{"Group3Message5", "No way!"}, // Pas possible !
		{"Group3Message6", "Holy cow!"}, // Sérieux ?!
		{"Group3Message7", "Whew."}, // Waouh.
		{"Group3Message8", "Siiiick!"}, // Truc de ouf !
		{"Group3Message9", "Calculated."}, // C'est prévu.
		{"Group3Message10", "Savage!"}, // Sauvage !
		{"Group3Message11", "Okay."}, // Ok.

		{"Group4Message1", "$#@%!"}, // $#@%!
		{"Group4Message2", "No problem."}, // Pas de problèmes.
		{"Group4Message3", "Whoops..."}, // Oups...
		{"Group4Message4", "Sorry!"}, // Désolé !
		{"Group4Message5", "My bad..."}, // Pardon...
		{"Group4Message6", "Oops!"}, // Oups !
		{"Group4Message7", "My fault."}, // Ma faute.

		{"Group5Message1", "gg"}, // gg
		{"Group5Message2", "Well played."}, // Bien joué.
		{"Group5Message3", "That was fun!"}, // C'était cool !
		{"Group5Message4", "Rematch!"}, // On remet ça !
		{"Group5Message5", "One. More. Game."}, // Encore. Une. Partie.
		{"Group5Message6", "What a game!"}, // Quelle partie !
		{"Group5Message7", "Nice moves!"}, // Super déplacements !
		{"Group5Message8", "Everybody dance!"}, // Que tout le monde dance !

		{"Group6Message4", "This is Rocket League!"}, // Ça c'est Rocket League !
	};

	//Functions

	void resetWhitelist();

	void jsonFileExists();
	std::map<std::string, bool> readJson(std::string category);
	void toggleQuickchatInJson(std::string category, std::string idMsg);

	virtual void onLoad();
	virtual void onUnload();

	void handleMsg(bool cancel, std::string playerName);

	void gameBegin();
	void gameEnd();
	void chatMessageEvent(ActorWrapper caller, void* args);
	void onStatTickerMessage(ServerWrapper caller, void* args);
	void hitBall(CarWrapper car, void* params);
	void replayEnd();
	void onTimerUpdate();
	void onGoal();

	void gameDestroyed();
	void ShowToxicityScores(CanvasWrapper canvas);

	struct StatTickerParams {
		// person who got a stat
		uintptr_t Receiver;
		// person who is victim of a stat (only exists for demos afaik)
		uintptr_t Victim;
		// wrapper for the stat event
		uintptr_t StatEvent;
	};

	// structure of a stat event
	struct StatEventParams {
		// always primary player
		uintptr_t PRI;
		// wrapper for the stat event
		uintptr_t StatEvent;
	};

	//Interface
	void RenderSettings() override;
	std::string GetPluginName() override;
	void SetImGuiContext(uintptr_t ctx) override;
};