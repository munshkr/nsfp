#include "player.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

using namespace std;

void start_track(Player *player, int track, bool print_full_info = false) {
  // Start first track
  if (auto err = player->start_track(track)) {
    cerr << "Player error: " << err << endl;
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

  auto& info = player->track_info();

  if (print_full_info) {
    if (strcmp(info.game, "") != 0)
      cout << "Game:      " << info.game << "\n";
    if (strcmp(info.author, "") != 0)
      cout << "Author:    " << info.author << "\n";
    if (strcmp(info.copyright, "") != 0)
      cout << "Copyright: " << info.copyright << "\n";
    if (strcmp(info.comment, "") != 0)
      cout << "Comment:   " << info.comment << "\n";
    if (strcmp(info.dumper, "") != 0)
      cout << "Dumper:    " << info.dumper;
    cout << endl;
  }

  char title[512];
  sprintf(title, "%s: %d/%d %s (%ld:%02ld)", game, track, player->track_count(),
          player->track_info().song, seconds / 60, seconds % 60);

  cout << title << endl;
}

int main(int argc, char *argv[]) {
  cerr << "nsfp 0.1 - Simple command-line player of NSF/NSFE files" << endl;

  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " nsf_file" << endl;
    return 1;
  }

  const string filename = string(argv[1]);

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
  if (auto err = player->load_file(filename)) {
    cerr << "Player error: " << err << endl;
    return 1;
  }

  bool running = true;

  int track = 0;
  start_track(player, track, true);

  while (running) {
    SDL_Delay(1000);
  }

  delete player;

  return 0;
}
