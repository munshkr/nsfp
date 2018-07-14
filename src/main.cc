/*
 * Copyright 2018 Damián Silvani
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <SDL2/SDL.h>
#include <iostream>
#include <string>

#ifdef CURSES
#include <ncurses.h>
#endif

#ifdef CURSES
#define PRINTF(...) printw(__VA_ARGS__)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

#include "cxxopts.h"
#include "player.h"

using namespace std;

void start_track(Player *player, int track, bool dry_run = false) {
#ifdef CURSES
  move(0, 0);
#endif

  // Start first track
  if (auto err = player->start_track(track, dry_run)) {
    PRINTF("Player error: %s\n", err);
    exit(1);
  }

  // Get track information
  long seconds = player->track_info().length / 1000;
  const char *game = player->track_info().game;
  if (!*game) {
    // extract filename
    game = strrchr(player->filename().c_str(), '\\'); // DOS
    if (!game)
      game = strrchr(player->filename().c_str(), '/'); // UNIX
    if (!game)
      game = player->filename().c_str();
    else
      game++; // skip path separator
  }

  auto &info = player->track_info();

  if (strcmp(info.game, "") != 0)
    PRINTF("Game:      %s\n", info.game);
  if (strcmp(info.author, "") != 0)
    PRINTF("Author:    %s\n", info.author);
  if (strcmp(info.copyright, "") != 0)
    PRINTF("Copyright: %s\n", info.copyright);
  if (strcmp(info.comment, "") != 0)
    PRINTF("Comment:   %s\n", info.comment);
  if (strcmp(info.dumper, "") != 0)
    PRINTF("Dumper:    %s\n", info.dumper);

  char title[512];
  sprintf(title, "%s: %d/%d %s (%ld:%02ld)", game, track, player->track_count() - 1,
          player->track_info().song, seconds / 60, seconds % 60);

  PRINTF("%s\n\n", title);

#ifdef CURSES
  refresh();
#endif
}

int main(int argc, const char *argv[]) {
  try {
    cxxopts::Options options(argv[0], "nsfp 0.1 - NSF/NSFE player");

    options.positional_help("INPUT").show_positional_help();

    options.add_options()
      ("input", "Input file", cxxopts::value<string>())
      ("i,info", "Only show info")
      ("t,track", "Start playing from a specific track",
        cxxopts::value<int>()->default_value("0"))
      ("s,single", "Stop after playing current track")
      ("h,help", "Print this message");

    options.parse_positional({"input"});

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      cout << options.help({""}) << endl;
      return 0;
    }

    if (!result.count("input")) {
      cerr << options.help({""}) << endl;
      return 1;
    }

    // Read options
    const string input = result["input"].as<string>();
    bool show_info = result["info"].as<bool>();
    int track = result["track"].as<int>();
    bool single = result["single"].as<bool>();

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
      cerr << "Failed to initialize SDL" << endl;
      return 1;
    }
    atexit(SDL_Quit);

    // Create player
    Player *player = new Player;
    if (!player) {
      cerr << "Out of memory" << endl;
      return 1;
    }

    // Initialize
    if (auto err = player->init()) {
      cerr << "Player error: " << err << endl;
      return 1;
    }

    // Load file
    if (auto err = player->load_file(input)) {
      cerr << "Player error: " << err << endl;
      return 1;
    }

    //
    // Main loop
    //

#ifdef CURSES
    initscr();
    noecho();
    keypad(stdscr, TRUE);
#endif

    bool running = true;
    bool playing = true;
    start_track(player, track, show_info);

    // If only printing info, do not keep running and exit
    if (show_info) {
      running = false;
    }

    while (running) {
      // Handle input
      int ch = getch();
      switch (ch) {
        case 'q':
          running = false;
          break;
        case KEY_RIGHT:
          track++;
          start_track(player, track);
          break;
        case KEY_LEFT:
          if (track > 0) {
            track--;
          }
          start_track(player, track);
          break;
        case ' ':
          player->pause(playing);
          if (playing) {
#ifdef CURSES
            move(0, 60);
#endif
            PRINTF("[Paused]\n");
          } else {
#ifdef CURSES
            move(0, 60);
            PRINTF("         \n");
#else
            PRINTF("[Playing]\n");
#endif
          }
          playing = !playing;
          break;
      }

      // If track ended, play the next track
      if (player->track_ended()) {
        // If all tracks have been played, exit
        if (single || track == player->track_count() - 1) {
          running = false;
          break;
        }

        track++;
        start_track(player, track);
      }
    }

    delete player;

#ifdef CURSES
    endwin();
#endif

    return 0;

  } catch (const cxxopts::OptionException &e) {
    cerr << "error parsing options: " << e.what() << endl;
    return 1;
  }
}
