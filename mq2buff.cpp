#include "MQ2Main.h"
#include <string>
#include <vector>
#include <map>
#include <fstream>

PreSetup("BuffRequestPlugin");

// Define a map to store the short spell name to full spell name mappings
std::map<std::string, std::string> spellNameMappings;

// Define a map to store class short names
std::map<std::string, std::string> classShortNames = {
    {"shaman", "shm"},
    {"warrior", "war"},
    {"cleric", "clr"},
    {"monk", "mnk"},
    {"Enchanter", "enc"},
    {"wizard", "wiz"},
    {"necromancer", "nec"},
    {"druid", "dru"},
    {"bard", "brd"},
    {"ranger", "rng"},
    {"rogue", "rog"},
    {"paladin", "pal"},
    {"shadow_knight", "SHD"},
    // Add more mappings as needed
};

// Function to load the short spell name mappings from "buffshortnames.ini"
void LoadShortSpellNames() {
    char iniFileName[MAX_STRING] = {0};
    sprintf_s(iniFileName, "%s\\buffshortnames.ini", gszINIPath);
    char sectionName[MAX_STRING] = "ShortSpellNames";

    // Clear the existing mappings
    spellNameMappings.clear();

    // Read the short spell names from the INI file
    for (std::map<std::string, std::string>::iterator it = spellNameMappings.begin(); it != spellNameMappings.end(); ++it) {
        GetPrivateProfileString(sectionName, it->first.c_str(), "", it->second, MAX_STRING, iniFileName);
    }
}

// Handle special class names based on class titles
void HandleSpecialClassNames(std::string& casterClass) {
    if (casterClass == "Reaver" || casterClass == "Revenant" || casterClass == "Grave Lord") {
        casterClass = "Shadow Knight"; // These titles are mapped to Shadow Knight
    } else if (casterClass == "Champion" || casterClass == "Myrmidon" || casterClass == "Warlord") {
        casterClass = "Warrior"; // These titles are mapped to Warrior
    } else if (casterClass == "Cavalier" || casterClass == "Knight" || casterClass == "Crusader") {
        casterClass = "Paladin"; // These titles are mapped to Paladin
    } else if (casterClass == "Rake" || casterClass == "Blackguard" || casterClass == "Assassin") {
        casterClass = "Rogue"; // These titles are mapped to Rogue
    } else if (casterClass == "Pathfinder" || casterClass == "Outrider" || casterClass == "Warder") {
        casterClass = "Ranger"; // These titles are mapped to Ranger
    } else if (casterClass == "Disciple" || casterClass == "Master" || casterClass == "Grandmaster") {
        casterClass = "Monk"; // These titles are mapped to Monk
    } else if (casterClass == "Minstrel" || casterClass == "Troubadour" || casterClass == "Virtuoso") {
        casterClass = "Bard"; // These titles are mapped to Bard
    } else if (casterClass == "Mystic" || casterClass == "Luminary" || casterClass == "Oracle") {
        casterClass = "Shaman"; // These titles are mapped to Shaman
    } else if (casterClass == "Wanderer" || casterClass == "Preserver" || casterClass == "Hierophant") {
        casterClass = "Druid"; // These titles are mapped to Druid
    } else if (casterClass == "Vicar" || casterClass == "Templar" || casterClass == "High Priest") {
        casterClass = "Cleric"; // These titles are mapped to Cleric
    } else if (casterClass == "Channeler" || casterClass == "Evoker" || casterClass == "Sorcerer") {
        casterClass = "Wizard"; // These titles are mapped to Wizard
    } else if (casterClass == "Heretic" || casterClass == "Defiler" || casterClass == "Warlock") {
        casterClass = "Necromancer"; // These titles are mapped to Necromancer
    } else if (casterClass == "Elementalist" || casterClass == "Conjurer" || casterClass == "Arch Mage") {
        casterClass = "Magician"; // These titles are mapped to Magician
    } else if (casterClass == "Illusionist" || casterClass == "Beguiler" || casterClass == "Phantasmist") {
        casterClass = "Enchanter"; // These titles are mapped to Enchanter
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
            std::string casterClass;
            int casterLevel;

            // Find the appropriate caster character and verify class and level
            if (FindCasterAndVerify(targetPlayer, casterCharacter, casterClass, casterLevel, buffName)) {
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
    bool FindCasterAndVerify(const std::string& targetPlayer, std::string& casterCharacter, std::string& casterClass, int& casterLevel, const std::string& spellName) {
        // Use the /who command to gather character information
        std::string whoOutput;
        if (EQWhos(targetPlayer.c_str(), whoOutput)) {
            // Parse the /who output to extract the character's class and level
            size_t classPos = whoOutput.find("Class: ");
            if (classPos != std::string::npos) {
                size_t levelPos = whoOutput.find("Level: ", classPos);
                if (levelPos != std::string::npos) {
                    // Extract class and level information
                    casterClass = whoOutput.substr(classPos + 7, levelPos - classPos - 8); // Extract class name
                    std::string levelStr = whoOutput.substr(levelPos + 7); // Extract level string

                    // Convert level string to an integer (assuming it's in the form "50 (whatever)")
                    size_t spacePos = levelStr.find(" ");
                    if (spacePos != std::string::npos) {
                        levelStr = levelStr.substr(0, spacePos); // Remove any additional information
                    }

                    casterLevel = atoi(levelStr.c_str());

                    // Map class names to short names
                    auto classShortNameIt = classShortNames.find(casterClass);
                    if (classShortNameIt != classShortNames.end()) {
                        casterClass = classShortNameIt->second;
                    }

                    // Handle special class names based on class titles
                    HandleSpecialClassNames(casterClass);

                    // Set the caster character name
                    casterCharacter = targetPlayer;

                    // Check if the caster is eligible to cast the spell based on class and level
                    // You can add your logic here
                    // For example:
                    // if (casterClass == "clr" && casterLevel >= 30) {
                    //     return true;
                    // }
                    // Adjust the condition based on your requirements

                    // For simplicity, allow any caster to cast any spell in this example
                    return true;
                }
            }
        }

        // If /who didn't return valid information or the caster is not eligible
        casterCharacter.clear();
        casterClass.clear();
        casterLevel = 0;
        return false;
    }
};

// Initialize the plugin
BOOL pluginLoaded = false;
BOOL InitializePlugin(PCHAR pName, PCHAR pVersion) {
    if (!pluginLoaded) {
        pluginLoaded = true;
        BuffRequestPlugin* plugin = new BuffRequestPlugin();
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
    // Check if the character is in combat
    bool inCombat = IsInCombat();

    // Check if another plugin is running (e.g., using MQ2Boxer)
    bool isOtherPluginRunning = IsOtherPluginRunning();

    // Check if the character is invisible
    bool isInvisible = IsCharacterInvisible();

    // Pause casting if any of the conditions are met
    if (AbleToBuff() && (inCombat || isOtherPluginRunning || isInvisible)) {
        PauseCasting();
    } else {
        ResumeCasting();
    }
}
