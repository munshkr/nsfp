#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "gme/gme.h"
#include <string>

typedef short sample_t;

class Player {
public:
  Player();
  ~Player();

  // Initialize player and set sample rate
  gme_err_t init(long sample_rate = 44100);

  // Load game music file. NULL on success, otherwise error string.
  gme_err_t load_file(const std::string &path);

  // (Re)start playing track. Tracks are numbered from 0 to track_count() - 1.
  gme_err_t start_track(int track);

  // Stop playing current file
  void stop();

  //
  // Optional functions
  //

  // Return currently loaded filename
  const std::string &filename() { return filename_; }

  // Number of tracks in current file, or 0 if no file loaded.
  int track_count() const;

  // Info for current track
  gme_info_t const &track_info() const { return *track_info_; }

  // Pause/resume playing current track.
  void pause(int);

  // True if track ended
  bool track_ended() const;

  // Pointer to emulator
  Music_Emu *emu() const { return emu_; }

  // Set stereo depth, where 0.0 = none and 1.0 = maximum
  void set_stereo_depth(double);

  // Enable accurate sound emulation
  void enable_accuracy(bool);

  // Set tempo, where 0.5 = half speed, 1.0 = normal, 2.0 = double speed
  void set_tempo(double);

  // Set voice muting bitmask
  void mute_voices(int);

  // Set buffer to copy samples from each buffer into, or NULL to disable
  void set_scope_buffer(sample_t *buf, int size) {
    scope_buf = buf;
    scope_buf_size = size;
  }

private:
  Music_Emu *emu_;
  sample_t *scope_buf;
  long sample_rate;
  int scope_buf_size;
  bool paused;
  gme_info_t *track_info_;
  std::string filename_;

  void suspend();
  void resume();
  static void fill_buffer(void *, sample_t *, int);
};

#endif // __PLAYER_H__
