#include "player.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {
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
  if (auto err = player->load_file(filename.c_str())) {
    cerr << "Player error: " << err << endl;
    return 1;
  }

  bool running = true;

  // Start first track
  if (auto err = player->start_track(0)) {
    cerr << "Player error: " << err << endl;
    return 1;
  }

  while (running) {
    SDL_Delay(1000);
  }

  delete player;

  return 0;
}
