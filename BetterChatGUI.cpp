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

	BetterChatParams pluginParams = getParamsInJson("Default config");

	CVarWrapper toxicityScoreXCvar = cvarManager->getCvar("betterchat_score_X");
	if (!toxicityScoreXCvar) { return; }
	int toxicityScoreX = toxicityScoreXCvar.getIntValue();

	CVarWrapper toxicityScoreYCvar = cvarManager->getCvar("betterchat_score_Y");
	if (!toxicityScoreYCvar) { return; }
	int toxicityScoreY = toxicityScoreYCvar.getIntValue();

	list<string> categories = {"quickchats" , "default", "beforeKickoff", "afterAlliedGoal", "afterEnemyGoal", "afterPass", "afterSave"};
	
	map<string, bool> defaultCheck = readMapInJson("default");
	map<string, bool> beforeKickoffCheck = readMapInJson("beforeKickoff");
	map<string, bool> afterAlliedGoalCheck = readMapInJson("afterAlliedGoal");
	map<string, bool> afterEnemyGoalCheck = readMapInJson("afterEnemyGoal");
	map<string, bool> afterPassCheck = readMapInJson("afterPass");
	map<string, bool> afterSaveCheck = readMapInJson("afterSave");
	
	map<string, map<string, bool>> maps = { {"default", defaultCheck}, {"beforeKickoff", beforeKickoffCheck}, { "afterAlliedGoal", afterAlliedGoalCheck }, {"afterEnemyGoal", afterEnemyGoalCheck}, {"afterPass", afterPassCheck}, {"afterSave", afterSaveCheck} };

	if (enabled) {
		ImGui::Text("\n");

		ImGui::PushID("1v1");

		// AntiSpam button
		if (ImGui::Checkbox("AntiSpam", &pluginParams.antispam)) {
			editParamInJson("Default config", "antispam", pluginParams.antispam);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enable/Disable AntiSpam");
		}

		// AntiSpam delay slider
		if (pluginParams.antispam) {
			if (ImGui::SliderInt("Delay", &pluginParams.antispam_delay, 0, 10)) {
				editParamInJson("Default config", "antispam_delay", pluginParams.antispam_delay);
			}
			if (ImGui::IsItemHovered()) {
				std::string hoverText = "Delay between two similar messages : " + std::to_string(pluginParams.antispam_delay) + " seconds";
				ImGui::SetTooltip(hoverText.c_str());
			}
			if (ImGui::Button("Reset delay value")) {
				pluginParams.antispam_delay = 5;
				editParamInJson("Default config", "antispam_delay", pluginParams.antispam_delay);
			}
		}
		else {
			ImGui::Text("\n\n");
		}

		// Message Filter Button
		ImGui::Text("\n");
		if (ImGui::Checkbox("Message Filter", &pluginParams.chatfilter)) {
			editParamInJson("Default config", "chatfilter", pluginParams.chatfilter);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("Enable/Disable Message Filter");
		}

		if (pluginParams.chatfilter) {
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
							toggleQuickchatInJson("Default config", column, msg);
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
			ImGui::Columns(1, nullptr);

			if (ImGui::Button("Reset table")) {
				string path = gameWrapper->GetDataFolder().string() + "/BetterChat_config.json";
				remove(path.c_str());
				jsonFileExists();
			}

			// Message filter options
			ImGui::Text("\nMessage filter options:");
			if (ImGui::Checkbox("Block written messages", &pluginParams.nowrittenmsg)) {
				editParamInJson("Default config", "nowrittenmsg", pluginParams.nowrittenmsg);
			}
			if (ImGui::Checkbox("Count written messages in the toxicity scores", &pluginParams.writtenmsgastoxic)) {
				editParamInJson("Default config", "writtenmsgastoxic", pluginParams.writtenmsgastoxic);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("If enabled, written messages will be considered as toxic if 'Block written messages' option is enabled, and as normal if not.");
			}
			if (ImGui::SliderInt("Time during which 'after a save' messages are allowed after a save.", &pluginParams.aftersavetime, 0, 20)) {
				editParamInJson("Default config", "aftersavetime", pluginParams.aftersavetime);
			}
			if (ImGui::Checkbox("Do not count a goal if it is an owngoal", &pluginParams.owngoal)) {
				editParamInJson("Default config", "owngoal", pluginParams.owngoal);
			}
			if (ImGui::Checkbox("Do not count a pass if an opponent touch it", &pluginParams.unwanted_pass)) {
				editParamInJson("Default config", "unwanted_pass", pluginParams.unwanted_pass);
			}

			// Toxicity Scores
			ImGui::Text("\n");
			if (ImGui::Checkbox("Toxicity scores", &pluginParams.toxicityscores)) {
				editParamInJson("Default config", "toxicityscores", pluginParams.toxicityscores);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Enable/Disable Toxicity Scores (at the end of the game)");
			}

			if (pluginParams.toxicityscores) {
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
	}
}