#include "pch.h"
#include "BetterChat.h"
#include <nlohmann/json.hpp>
#include <set>
#include <unordered_set>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using json = nlohmann::json;

bool newConfigButtonClicked = false;
string newConfigName = "";

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

	CVarWrapper toxicityScoreXCvar = cvarManager->getCvar("betterchat_score_X");
	if (!toxicityScoreXCvar) { return; }
	int toxicityScoreX = toxicityScoreXCvar.getIntValue();

	CVarWrapper toxicityScoreYCvar = cvarManager->getCvar("betterchat_score_Y");
	if (!toxicityScoreYCvar) { return; }
	int toxicityScoreY = toxicityScoreYCvar.getIntValue();

	list<string> categories = { "quickchats" , "default", "beforeKickoff", "afterAlliedGoal", "afterEnemyGoal", "afterPass", "afterSave" };

	// On/Off Plugin Button
	if (ImGui::Checkbox("Enable plugin", &enabled)) {
		enabledCvar.setValue(enabled);
	}
	if (ImGui::IsItemHovered()) {
		ImGui::SetTooltip("Enable/Disable BetterChat Plugin");
	}

	if (enabled)
	{
		ImGui::Separator();

		// Configuration by gamemode
		ImGui::Text("Gamemode configuration:");
		ImGui::Text("");
		
		list<string> configsSet = getConfigsListInJson();
		const char** configsArray = new const char* [configsSet.size()];
		configsArray[0] = "Default config";
		int i = 1;
		for (const string& config : configsSet) {
			if (config != "Default config") {
				configsArray[i] = config.c_str();
				i++;
			}
		}

		map<string, string> configByGamemode = getConfigByGamemodeInJson();
		map<string, int> configIndexByGamemode;
		for (auto& pair : configByGamemode) {
			configIndexByGamemode[pair.first] = 0;
			for (int i = 1; i < configsSet.size(); i++) {
				if (pair.second == configsArray[i]) {
					configIndexByGamemode[pair.first] = i;
					break;
				}
			}
		}

		// Configurations combo for each gamemode
		ImGui::BeginGroup();
		ImGui::Columns(2, nullptr, false);
		ImGui::SetColumnWidth(-1, 100);
		float textOffset;

		for (auto& pair : configIndexByGamemode) {
			ImGui::PushID(pair.first.c_str());
			textOffset = (ImGui::GetColumnWidth() - ImGui::CalcTextSize(pair.first.c_str()).x) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset - 6);
			ImGui::Text(pair.first.c_str());
			ImGui::NextColumn();
			ImGui::PushItemWidth(300);
			if (ImGui::Combo("", &pair.second, configsArray, configsSet.size())) {
				editConfigByGamemodeInJson(pair.first, configsArray[pair.second]);
				LOG("'" + pair.first + "' gamemode is now using '" + configsArray[pair.second] + "' config.");
				if (gameWrapper->IsInOnlineGame()) {
					refreshConfig();
					resetWhitelist();
				}
			}
			ImGui::PopItemWidth();
			ImGui::PopID();
			ImGui::NextColumn();
		}
		ImGui::Columns(1, nullptr);
		ImGui::Dummy(ImVec2(0, 10));
		ImGui::EndGroup();

		ImGui::SameLine(0, 600);

		// New config button
		ImGui::BeginGroup();
		ImGui::Text("\n");
		if (ImGui::Button("New config", ImVec2(175, 60))) {
			newConfigButtonClicked = !newConfigButtonClicked;
		}
		if (newConfigButtonClicked) {
			ImGui::PushItemWidth(175);
			ImGui::Text("Config name:");
			ImGui::InputText("", &newConfigName);
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("Create")) {
				if (newConfigName != "" && newConfigName != "ConfigByGamemode" && find(configsSet.begin(), configsSet.end(), newConfigName) == configsSet.end()) {
					createConfigInJson(newConfigName);
					LOG("'" + newConfigName + "' config has been created.");
					newConfigButtonClicked = false;
					newConfigName = "";
				}
				else {
					ImGui::OpenPopup("Error");
				}
			}
		}
		ImGui::EndGroup();

		ImGui::Separator();

		ImGui::Text("\n");

		// Configurations tabs
		if (ImGui::BeginTabBar("tab_bar")) {
			for (const string& config : configsSet) {

				if (ImGui::BeginTabItem(config.c_str())) {
					ImGui::PushID(config.c_str());

					BetterChatParams pluginParams = getParamsInJson(config);

					map<string, bool> defaultCheck = readMapInJson(config, "default");
					map<string, bool> beforeKickoffCheck = readMapInJson(config, "beforeKickoff");
					map<string, bool> afterAlliedGoalCheck = readMapInJson(config, "afterAlliedGoal");
					map<string, bool> afterEnemyGoalCheck = readMapInJson(config, "afterEnemyGoal");
					map<string, bool> afterPassCheck = readMapInJson(config, "afterPass");
					map<string, bool> afterSaveCheck = readMapInJson(config, "afterSave");

					map<string, map<string, bool>> maps = { {"default", defaultCheck}, {"beforeKickoff", beforeKickoffCheck}, { "afterAlliedGoal", afterAlliedGoalCheck }, {"afterEnemyGoal", afterEnemyGoalCheck}, {"afterPass", afterPassCheck}, {"afterSave", afterSaveCheck} };

					// AntiSpam button
					if (ImGui::Checkbox("AntiSpam", &pluginParams.antispam)) {
						editParamInJson(config, "antispam", pluginParams.antispam);
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Enable/Disable AntiSpam");
					}

					// AntiSpam delay slider
					if (pluginParams.antispam) {
						ImGui::PushItemWidth(500);
						if (ImGui::SliderInt("Delay", &pluginParams.antispam_delay, 0, 10)) {
							editParamInJson(config, "antispam_delay", pluginParams.antispam_delay);
						}
						ImGui::PopItemWidth();
						if (ImGui::IsItemHovered()) {
							std::string hoverText = "Delay between two similar messages : " + std::to_string(pluginParams.antispam_delay) + " seconds";
							ImGui::SetTooltip(hoverText.c_str());
						}
					}
					else {
						ImGui::Text("\n\n");
					}

					// Message Filter Button
					ImGui::Text("\n");
					if (ImGui::Checkbox("Message Filter", &pluginParams.chatfilter)) {
						editParamInJson(config, "chatfilter", pluginParams.chatfilter);
					}
					if (ImGui::IsItemHovered()) {
						ImGui::SetTooltip("Enable/Disable Message Filter");
					}

					// Message Filter Table
					if (pluginParams.chatfilter) {
						ImGui::Text("\n");

						// Menu Bar
						ImGui::BeginChild("Config Table", ImVec2(775, 450), true, ImGuiWindowFlags_MenuBar);
						if (ImGui::BeginMenuBar())
						{
							ImGui::Text("Configuration");
							ImGui::EndMenuBar();
						}

						// Headers child
						ImGui::BeginChild("Headers", ImVec2(0, 35), false);

						ImGui::Columns(7, nullptr);

						float headerOffsetX;
						float headerOffsetY = ImGui::GetTextLineHeight() * 0.5f + 2;

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

						ImGui::Columns(1, nullptr);
						ImGui::EndChild();

						ImGui::Separator();
						ImGui::Dummy(ImVec2(0, 3));

						// Table content

						ImGui::BeginChild("TableContent", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
						ImGui::Columns(7, nullptr);

						for (const auto& chat : idQuickchats) {
							for (const string column : categories) {
								ImGui::PushID(column.c_str());
								if (column == "quickchats") {
									ImGui::SetColumnWidth(-1, 150);
									float textOffsetX = (ImGui::GetColumnWidth() - ImGui::CalcTextSize(chat.second.c_str()).x) * 0.5f;
									ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffsetX - 6);
									ImGui::Text(chat.second.c_str());
								}
								else {
									ImGui::SetColumnWidth(-1, 100);
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
										toggleQuickchatInJson(config, column, msg);
										if (gameWrapper->IsInOnlineGame()) {
											resetWhitelist();
										}
									}
									ImGui::PopID();
									ImGui::PopStyleColor(3);
								}
								ImGui::PopID();
								ImGui::NextColumn();
							}
						}
						ImGui::Columns(1, nullptr);
						ImGui::EndChild();

						ImGui::EndChild();

						// Message filter options
						ImGui::Text("\nMessage filter options:");
						if (ImGui::Checkbox("Block written messages", &pluginParams.nowrittenmsg)) {
							editParamInJson(config, "nowrittenmsg", pluginParams.nowrittenmsg);
						}
						if (ImGui::Checkbox("Count written messages in the toxicity scores", &pluginParams.writtenmsgastoxic)) {
							editParamInJson(config, "writtenmsgastoxic", pluginParams.writtenmsgastoxic);
						}
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("If enabled, written messages will be considered as toxic if 'Block written messages' option is enabled, and as normal if not.");
						}
						ImGui::PushItemWidth(500);
						if (ImGui::SliderInt("Time during which 'after a save' messages are allowed after a save.", &pluginParams.aftersavetime, 0, 20)) {
							editParamInJson(config, "aftersavetime", pluginParams.aftersavetime);
						}
						ImGui::PopItemWidth();
						if (ImGui::Checkbox("Do not count a goal if it is an owngoal", &pluginParams.owngoal)) {
							editParamInJson(config, "owngoal", pluginParams.owngoal);
						}
						if (ImGui::Checkbox("Do not count a pass if an opponent touch it", &pluginParams.unwanted_pass)) {
							editParamInJson(config, "unwanted_pass", pluginParams.unwanted_pass);
						}
					}

					// Toxicity scores
					if (pluginParams.antispam || pluginParams.chatfilter) {
						ImGui::Text("\n");
						if (ImGui::Checkbox("Toxicity scores", &pluginParams.toxicityscores)) {
							editParamInJson(config, "toxicityscores", pluginParams.toxicityscores);
						}
						if (ImGui::IsItemHovered()) {
							ImGui::SetTooltip("Enable/Disable Toxicity Scores (at the end of the game)");
						}
					}

					// Delete config button
					if (config != "Default config") {
						ImGui::Text("\n");
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
						if (ImGui::Button("Delete config")) {
							for (auto& pair : configByGamemode) {
								if (pair.second == config) {
									editConfigByGamemodeInJson(pair.first, "Default config");
									LOG("'" + pair.first + "' gamemode is now using 'Default config' config.");
								}
							}
							if (gameWrapper->IsInOnlineGame()) {
								refreshConfig();
								resetWhitelist();
							}
							deleteConfigInJson(config);
							LOG("'" + config + "' config has been deleted.");
						}
						ImGui::PopStyleColor(3);
					}
					ImGui::EndTabItem();
					ImGui::PopID();
				}
			}
			ImGui::EndTabBar();
		}
		ImGui::Separator();

		// Toxicity scores position
		ImGui::Text("Toxicity scores options:");
		if (ImGui::SliderInt("X", &toxicityScoreX, 0, 1920)) {
			toxicityScoreXCvar.setValue(toxicityScoreX);
		}

		if (ImGui::SliderInt("Y", &toxicityScoreY, 0, 1080)) {
			toxicityScoreYCvar.setValue(toxicityScoreY);
		}
	}

	ImGui::Text("\n\n");

	ImGui::Separator();

	ImGui::Text("Plugin version: %s", plugin_version);

	if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("This config name is invalid\nor already exists.");
		if (ImGui::Button("OK", ImVec2(50, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}