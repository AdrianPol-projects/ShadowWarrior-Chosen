// The game this build of the Chosen engine is for: names, folders and the file that shows the
// game data is there (used by the Android, iPad and web platform code).

#pragma once

#define CHOSEN_GAME_NAME "SW Chosen"
#define CHOSEN_GAME_FOLDER "ShadowWarrior"     // Android: main storage/ShadowWarrior, iPad: On My iPad > SW Chosen
#define CHOSEN_GAME_LOG "swchosen.log"
#define CHOSEN_GAME_EXE "swchosen"
#define CHOSEN_GAME_PK3 "swchosen.pk3"       // shipped with the app when there is one
#define CHOSEN_GAME_DATAFILE "SW.GRP"      // the game data is there when this file is
#define CHOSEN_GAME_DATAFILE_LOWER "sw.grp"
#define CHOSEN_GAME_DATAFILE_MIXED "ShadowWarrior.grp"
#define CHOSEN_GAME_FILES "Shadow Warrior game files"
#define CHOSEN_GAME_README_NAME "PUT SW FILES HERE.txt"
#define CHOSEN_GAME_SOURCES "your Shadow Warrior folder (GOG/Steam Shadow Warrior Classic Complete or Classic Redux, the CD, or the free shareware version)"
#define CHOSEN_GAME_README_FILES \
    "Required:\n" \
    "  SW.GRP  (Shadow Warrior Classic Complete / Classic Redux from GOG or Steam, the CD,\n" \
    "          or the free shareware version)\n\n" \
    "Optional:\n" \
    "  SW.RTS (the voice macros), the add-ons WT.GRP (Wanton Destruction) and TD.GRP (Twin Dragon),\n" \
    "  user maps (*.MAP), CD music as trackXX.ogg\n\n"
