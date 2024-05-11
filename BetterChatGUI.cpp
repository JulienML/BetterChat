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

	CVarWrapper noWrittenMsgCvar = cvarManager->getCvar("betterchat_nowrittenmsg");
	if (!noWrittenMsgCvar) { return; }
	bool noWrittenMsg = noWrittenMsgCvar.getBoolValue();

	CVarWrapper writtenMsgAsToxicCvar = cvarManager->getCvar("betterchat_writtenmsgastoxic");
	if (!writtenMsgAsToxicCvar) { return; }
	bool writtenMsgAsToxic = writtenMsgAsToxicCvar.getBoolValue();

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

	list<string> categories = {"quickchats" , "default", "beforeKickoff", "afterAlliedGoal", "afterEnemyGoal", "afterPass", "afterSave"};
	
	map<string, bool> defaultCheck = readJson("default");
	map<string, bool> beforeKickoffCheck = readJson("beforeKickoff");
	map<string, bool> afterAlliedGoalCheck = readJson("afterAlliedGoal");
	map<string, bool> afterEnemyGoalCheck = readJson("afterEnemyGoal");
	map<string, bool> afterPassCheck = readJson("afterPass");
	map<string, bool> afterSaveCheck = readJson("afterSave");
	
	map<string, map<string, bool>> maps = { {"default", defaultCheck}, {"beforeKickoff", beforeKickoffCheck}, { "afterAlliedGoal", afterAlliedGoalCheck }, {"afterEnemyGoal", afterEnemyGoalCheck}, {"afterPass", afterPassCheck}, {"afterSave", afterSaveCheck} };

	if (enabled) {
		ImGui::Text("\n");

		/*if (ImGui::BeginTabBar("tabBar")) {
			if (ImGui::BeginTabItem("1v1")) {*/
		ImGui::PushID("1v1");

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
			
			ImGui::BeginChild("Quickchats", ImVec2(755, 450), true, ImGuiWindowFlags_MenuBar); ;
			if (ImGui::BeginMenuBar())
			{
				ImGui::Text("Configuration");
				ImGui::EndMenuBar();
			}

			float headerOffsetX;			
			float headerOffsetY = ImGui::GetTextLineHeight() * 0.5f + 2;

			ImGui::Columns(7, nullptr);

			ImGui::SetColumnWidth(-1, 150);																						   
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("Quickchats").x) * 0.5f;								   
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
			ImGui::Text("Quickchats");																							   
			ImGui::NextColumn();																								   
			ImGui::SetColumnWidth(-1, 100);																						   
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("Default").x) * 0.5f;								   
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
			ImGui::Text("Default");																								   
			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100);
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("Before kickoff").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
			ImGui::Text("Before kickoff");
			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100);
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("After an allied").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::Text("After an allied");
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("goal").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::Text("goal");
			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100);
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("After an enemy").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::Text("After an enemy");
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("goal").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::Text("goal");
			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100);
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("After an assist").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
			ImGui::Text("After an assist");
			ImGui::NextColumn();
			ImGui::SetColumnWidth(-1, 100);
			headerOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize("After a save").x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + headerOffsetX - 6);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + headerOffsetY);
			ImGui::Text("After a save");
			ImGui::NextColumn();
			ImGui::Separator();

			for (const auto& chat : idQuickchats) {
				for (const string column : categories) {
					ImGui::PushID(column.c_str());
					if (column == "quickchats") {
						float textOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize(chat.second.c_str()).x) * 0.5f;
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX - 6);
						ImGui::Text(chat.second.c_str());
					}
					else {
						const string msg = chat.first;
						map<string, bool> map = maps[column];
						bool check = map[msg];
						string btn = check ? "Allowed" : "Forbidden";
						if (check) {
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.7f, 0.1f, 1.0f));
						}
						else {
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
						}

						ImGui::PushID(msg.c_str());
						float btnOffsetX = (ImGui::GetColumnWidth() - 80) * 0.5f;
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + btnOffsetX - 6);
						if (ImGui::Button(btn.c_str(), ImVec2(80, 18))) {
							toggleQuickchatInJson(column, msg);
							resetWhitelist();
						}
						ImGui::PopID();
						ImGui::PopStyleColor(3);
					}
					ImGui::PopID();
					ImGui::NextColumn();
				}
			}
			ImGui::EndChild();
			//ImGui::Separator();
			ImGui::Columns(1, nullptr);

			if (ImGui::Button("Reset table")) {
				string path = gameWrapper->GetDataFolder().string() + "/BetterChat_config.json";
				remove(path.c_str());
				jsonFileExists();
			}

			// Message filter options
			ImGui::Text("\nMessage filter options:");
			if (ImGui::Checkbox("Block written messages", &noWrittenMsg)) {
				noWrittenMsgCvar.setValue(noWrittenMsg);
			}
			if (ImGui::Checkbox("Count written messages in the toxicity scores", &writtenMsgAsToxic)) {
				writtenMsgAsToxicCvar.setValue(writtenMsgAsToxic);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("If enabled, written messages will be considered as toxic if 'Block written messages' option is enabled, and as normal if not.");
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
		ImGui::PopID();
				/*ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("2v2")) {
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("3v3")) {
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}*/
	}
}