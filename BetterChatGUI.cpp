#include "pch.h"
#include "BetterChat.h"
#include <set>
#include <map>
#include <string>

using namespace std;

//Plugin Settings Window code here

void BetterChat::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string BetterChat::GetPluginName() {
	return "BetterChat Plugin";
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> BetterChat
void BetterChat::RenderSettings() {
	CVarWrapper enabledCvar = cvarManager->getCvar("betterchat_enabled");
	if (!enabledCvar) { return; }
	bool enabled = enabledCvar.getBoolValue();

	// On/Off Plugin Button
	if (ImGui::Checkbox("Enable plugin", &enabled)) {
		enabledCvar.setValue(enabled);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Enable/Disable BetterChat Plugin");
	}

	CVarWrapper antiSpamCvar = cvarManager->getCvar("betterchat_antispam");
	if (!antiSpamCvar) { return; }
	bool antiSpam = antiSpamCvar.getBoolValue();

	CVarWrapper chatFilterCvar = cvarManager->getCvar("betterchat_chatfilter");
	if (!chatFilterCvar) { return; }
	bool chatFilter = chatFilterCvar.getBoolValue();

	CVarWrapper delayCvar = cvarManager->getCvar("betterchat_delay");
	if (!delayCvar) { return; }
	int delay = delayCvar.getIntValue();

	CVarWrapper writtenMsgCvar = cvarManager->getCvar("betterchat_writtenmsg");
	if (!writtenMsgCvar) { return; }
	bool writtenMsg = writtenMsgCvar.getBoolValue();

	CVarWrapper afterSaveTimeCvar = cvarManager->getCvar("betterchat_aftersavetime");
	if (!afterSaveTimeCvar) { return; }
	int afterSaveTime = afterSaveTimeCvar.getIntValue();

	CVarWrapper owngoalCvar = cvarManager->getCvar("betterchat_owngoal");
	if (!owngoalCvar) { return; }
	bool owngoalDetection = owngoalCvar.getBoolValue();

	CVarWrapper unwantedPassCvar = cvarManager->getCvar("betterchat_unwanted_pass");
	if (!unwantedPassCvar) { return; }
	bool unwantedPassDetection = unwantedPassCvar.getBoolValue();

	CVarWrapper toxicityScoresCvar = cvarManager->getCvar("betterchat_toxicityscores");
	if (!toxicityScoresCvar) { return; }
	bool toxicityScores = toxicityScoresCvar.getBoolValue();

	CVarWrapper toxicityScoreXCvar = cvarManager->getCvar("betterchat_score_X");
	if (!toxicityScoreXCvar) { return; }
	int toxicityScoreX = toxicityScoreXCvar.getIntValue();

	CVarWrapper toxicityScoreYCvar = cvarManager->getCvar("betterchat_score_Y");
	if (!toxicityScoreYCvar) { return; }
	int toxicityScoreY = toxicityScoreYCvar.getIntValue();

	if (enabled) {
		ImGui::Text("\n");

		// AntiSpam button
		if (ImGui::Checkbox("AntiSpam", &antiSpam)) {
			antiSpamCvar.setValue(antiSpam);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enable/Disable AntiSpam");
		}

		// AntiSpam delay slider
		if (antiSpam) {
			if (ImGui::SliderInt("Delay", &delay, 0, 10)) {
				delayCvar.setValue(delay);
			}
			if (ImGui::IsItemHovered()) {
				std::string hoverText = "Delay between two similar messages : " + std::to_string(delay) + " seconds";
				ImGui::SetTooltip(hoverText.c_str());
			}
			if (ImGui::Button("Reset delay value")) {
				delay = 5;
				delayCvar.setValue(delay);
			}
		}
		else {
			ImGui::Text("\n\n");
		}

		// Message Filter Button
		ImGui::Text("\n");
		if (ImGui::Checkbox("Message Filter", &chatFilter)) {
			chatFilterCvar.setValue(chatFilter);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enable/Disable Message Filter");
		}

		if (chatFilter) {
			ImGui::Text("\n");

			// 1st header
			ImGui::Columns(2, nullptr);

			ImGui::Separator();
			ImGui::Text("These quickchats are");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("Except in these situations:");
			ImGui::SetColumnWidth(-1, 825);
			ImGui::NextColumn();

			// 2nd header
			ImGui::Columns(6, nullptr);

			ImGui::Text("forbidden:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("Before kickoff:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("After an allied goal:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("After an enemy goal:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("After an assist:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Text("After a save:");
			ImGui::SetColumnWidth(-1, 165);
			ImGui::NextColumn();
			ImGui::Separator();

			// Default

			map<string, bool> defaultMsgCheck;
			set<string> defaultMsg = readJson("default");
			for (const auto& pair : BetterChat::idQuickchats) {
				const string& id = pair.first;
				bool check = defaultMsg.find(id) != defaultMsg.end();
				defaultMsgCheck.emplace(id, check);
			}

			ImGui::PushID("defaultColumn");
			for (const auto& pair : defaultMsgCheck) {
				const string msg = pair.first;
				bool defaultCheckbox = defaultMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &defaultCheckbox)) {
					if (defaultCheckbox) {
						addToJson("default", msg);
					}
					else {
						removeFromJson("default", msg);
					}
					resetBlacklist();
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();

			// Before kick-off

			map<string, bool> beforeKickoffMsgCheck;
			set<string> beforeKickoffMsg = readJson("beforeKickoff");
			for (const string& id : defaultMsg) {
				bool check = beforeKickoffMsg.find(id) != beforeKickoffMsg.end();
				beforeKickoffMsgCheck.emplace(id, check);
			}

			ImGui::PushID("beforeKickoffColumn");
			for (const auto& pair : beforeKickoffMsgCheck) {
				const string msg = pair.first;
				bool checkbox = beforeKickoffMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &checkbox)) {
					if (checkbox) {
						addToJson("beforeKickoff", msg);
					}
					else {
						removeFromJson("beforeKickoff", msg);
					}
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();

			// After an allied goal

			map<string, bool> afterAlliedGoalMsgCheck;
			set<string> afterAlliedGoalMsg = readJson("afterAlliedGoal");
			for (const string& id : defaultMsg) {
				bool check = afterAlliedGoalMsg.find(id) != afterAlliedGoalMsg.end();
				afterAlliedGoalMsgCheck.emplace(id, check);
			}

			ImGui::PushID("afterAlliedGoalColumn");
			for (const auto& pair : afterAlliedGoalMsgCheck) {
				const string msg = pair.first;
				bool checkbox = afterAlliedGoalMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &checkbox)) {
					if (checkbox) {
						addToJson("afterAlliedGoal", msg);
					}
					else {
						removeFromJson("afterAlliedGoal", msg);
					}
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();

			// After an enemy goal

			map<string, bool> afterEnemyGoalMsgCheck;
			set<string> afterEnemyGoalMsg = readJson("afterEnemyGoal");
			for (const string& id : defaultMsg) {
				bool check = afterEnemyGoalMsg.find(id) != afterEnemyGoalMsg.end();
				afterEnemyGoalMsgCheck.emplace(id, check);
			}

			ImGui::PushID("afterEnemyGoalColumn");
			for (const auto& pair : afterEnemyGoalMsgCheck) {
				const string msg = pair.first;
				bool checkbox = afterEnemyGoalMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &checkbox)) {
					if (checkbox) {
						addToJson("afterEnemyGoal", msg);
					}
					else {
						removeFromJson("afterEnemyGoal", msg);
					}
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();

			// After an assist

			map<string, bool> afterPassMsgCheck;
			set<string> afterPassMsg = readJson("afterPass");
			for (const string& id : defaultMsg) {
				bool check = afterPassMsg.find(id) != afterPassMsg.end();
				afterPassMsgCheck.emplace(id, check);
			}

			ImGui::PushID("afterPassColumn");
			for (const auto& pair : afterPassMsgCheck) {
				const string msg = pair.first;
				bool checkbox = afterPassMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &checkbox)) {
					if (checkbox) {
						addToJson("afterPass", msg);
					}
					else {
						removeFromJson("afterPass", msg);
					}
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();

			// After a save

			map<string, bool> afterSaveMsgCheck;
			set<string> afterSaveMsg = readJson("afterSave");
			for (const string& id : defaultMsg) {
				bool check = afterSaveMsg.find(id) != afterSaveMsg.end();
				afterSaveMsgCheck.emplace(id, check);
			}

			ImGui::PushID("afterSaveColumn");
			for (const auto& pair : afterSaveMsgCheck) {
				const string msg = pair.first;
				bool checkbox = afterSaveMsgCheck[msg];
				if (ImGui::Checkbox(BetterChat::idQuickchats[msg].c_str(), &checkbox)) {
					if (checkbox) {
						addToJson("afterSave", msg);
					}
					else {
						removeFromJson("afterSave", msg);
					}
				}
			}
			ImGui::PopID();
			ImGui::NextColumn();
			ImGui::Separator();
			ImGui::Columns(1, nullptr);

			if (ImGui::Button("Reset table")) {
				string path = gameWrapper->GetDataFolder().string() + "/BetterChat_Blacklist.json";
				remove(path.c_str());
				jsonFileExists();
			}

			// Message filter options
			ImGui::Text("\nMessage filter options:");
			if (ImGui::Checkbox("Disable written messages", &writtenMsg)) {
				writtenMsgCvar.setValue(writtenMsg);
			}
			if (ImGui::SliderInt("Time during which 'after a save' messages are allowed after a save.", &afterSaveTime, 0, 20)) {
				afterSaveTimeCvar.setValue(afterSaveTime);
			}
			if (ImGui::Checkbox("Do not count a goal if it is an owngoal", &owngoalDetection)) {
				owngoalCvar.setValue(owngoalDetection);
			}
			if (ImGui::Checkbox("Do not count a pass if an opponent touch it", &unwantedPassDetection)) {
				unwantedPassCvar.setValue(unwantedPassDetection);
			}

			// Toxicity Scores
			ImGui::Text("\n");
			if (ImGui::Checkbox("Toxicity scores", &toxicityScores)) {
				toxicityScoresCvar.setValue(toxicityScores);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Enable/Disable Toxicity Scores (at the end of the game)");
			}

			if (toxicityScores) {
				ImGui::Text("Toxicity scores options:");
				if (ImGui::SliderInt("X", &toxicityScoreX, 0, 1920)) {
					toxicityScoreXCvar.setValue(toxicityScoreX);
				}

				if (ImGui::SliderInt("Y", &toxicityScoreY, 0, 1080)) {
					toxicityScoreYCvar.setValue(toxicityScoreY);
				}
			}
		}
	}
}