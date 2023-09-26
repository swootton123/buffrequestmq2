#include "MQ2Main.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

PreSetup("BuffRequestPlugin");

// Define a map to store the short spell name to full spell name mappings
std::map<std::string, std::string> spellNameMappings;

// Function to load the short spell name mappings from "shortspellnames.txt"
void LoadShortSpellNames() {
    std::ifstream file("shortspellnames.txt");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            size_t pos = line.find("\t");
            if (pos != std::string::npos) {
                std::string shortName = line.substr(0, pos);
                std::string fullName = line.substr(pos + 1);
                spellNameMappings[shortName] = fullName;
            }
        }
        file.close();
    }
}

// Define the plugin class
class BuffRequestPlugin : public MQ2Plugin {
public:
    BuffRequestPlugin() {
        // Constructor
        LoadShortSpellNames();
    }

    virtual ~BuffRequestPlugin() {
        // Destructor
    }

    virtual void OnPluginInit() {
        DebugSpewAlways("BuffRequestPlugin::OnPluginInit()");
    }

    virtual void OnPluginShutdown() {
        DebugSpewAlways("BuffRequestPlugin::OnPluginShutdown()");
    }

    // Handle incoming /tell messages
    virtual void OnIncomingChat(PCHAR Line, DWORD Color) {
        if (Color == 0xFFFFFF && strstr(Line, "/tell ")) {
            // Parse the /tell message
            std::string message(Line);
            std::string::size_type pos = message.find(" ");
            if (pos != std::string::npos) {
                std::string command = message.substr(0, pos);
                std::string args = message.substr(pos + 1);

                if (command == "/tell") {
                    HandleTellCommand(args.c_str());
                }
            }
        }
    }

    // Handle the /tell command
    void HandleTellCommand(const char* args) {
        // Parse the /tell message for buff requests
        std::string request(args);
        std::vector<std::string> tokens;
        size_t pos = 0;
        while ((pos = request.find(" ")) != std::string::npos) {
            tokens.push_back(request.substr(0, pos));
            request.erase(0, pos + 1);
        }
        tokens.push_back(request);

        // Check if the request is valid and contains at least two arguments
        if (tokens.size() >= 3) {
            std::string targetPlayer = tokens[0]; // The player sending the /tell
            std::string spellName = tokens[1];    // The provided spell name (short or full)
            std::string buffName;

            // Look up the full spell name using the provided spell name
            auto it = spellNameMappings.find(spellName);
            if (it != spellNameMappings.end()) {
                buffName = it->second;
            } else {
                // Use the provided spell name as the full spell name
                buffName = spellName;
            }

            std::string casterCharacter;

            // Find the appropriate caster character and verify class and level
            if (FindCasterAndVerify(casterName, buffName, casterCharacter, requiredClass, requiredLevel)) {
                // Check if you have the spell in your spellbook
                if (MemorizeSpellByName(buffName.c_str(), 8)) {
                    // Successfully memorized the spell in gem 8

                    // Target the player who sent the tell
                    if (TargetPlayerByName(targetPlayer.c_str())) {
                        // Successfully targeted the player
                        Sleep(500); // Sleep for a moment to allow targeting to take effect

                        // Cast the spell on the targeted player
                        if (CastSpellByName(targetPlayer.c_str(), buffName.c_str())) {
                            WriteChatf("Casting %s on %s", buffName.c_str(), targetPlayer.c_str());
                        } else {
                            WriteChatf("Failed to cast %s on %s", buffName.c_str(), targetPlayer.c_str());
                        }
                    } else {
                        WriteChatf("Failed to target %s", targetPlayer.c_str());
                    }
                } else {
                    WriteChatf("I do not have the spell %s in my spellbook", buffName.c_str());
                }
            } else {
                // Caster is not valid
                WriteChatf("I am not the appropriate caster for %s", buffName.c_str());
            }
        } else {
            WriteChatf("Usage: /tell <player_name> <spell_name>");
        }
    }

    // Function to find the appropriate caster character and verify class and level
    bool FindCasterAndVerify(const std::string& casterName, const std::string& spellName, std::string& casterCharacter, const std::string& requiredClass, int requiredLevel) {
        // todo:
        // For example, you might query your server's database or use in-game data.
        // Replace the following placeholders with your implementation:

        // Placeholder for querying character data by name (casterName)
        // Example: casterCharacter = QueryCharacterName(casterName);

        // Placeholder for querying character class by name (casterCharacter)
        // Example: std::string casterClass = QueryCharacterClass(casterCharacter);

        // Placeholder for querying character level by name (casterCharacter)
        // Example: int casterLevel = QueryCharacterLevel(casterCharacter);

        // Sample checks (adjust as needed)
        if (casterCharacter == "MyCasterCharacter" && casterClass == "Wizard" && casterLevel >= 30) {
            return true;
        } else {
            return false;
        }
    }
};

// Initialize the plugin
BOOL pluginLoaded = false;
BOOL InitializePlugin(PCHAR pName, PCHAR pVersion) {
    if (!pluginLoaded) {
        pluginLoaded = true;
        BuffRequestPlugin *plugin = new BuffRequestPlugin();
    }
    return true;
}

// Shutdown the plugin
BOOL ShutdownPlugin() {
    if (pluginLoaded) {
        pluginLoaded = false;
        delete plugin;
    }
    return true;
}

PLUGIN_API void OnPulse() {
    // Check if the character is performing an action (e.g., casting a spell)
    if (IsCharacterPerformingAction()) {
        PauseCasting();
    } else {
        ResumeCasting();
    }

    // Check if another plugin is running (e.g., using MQ2Boxer)
    if (IsOtherPluginRunning()) {
        PauseCasting();
    } else {
        ResumeCasting();
    }
}
