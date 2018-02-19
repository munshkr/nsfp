#include "player.h"
#include <cstring>
#include <string>

using namespace std;

#define RETURN_ERR(expr)                                                       \
  do {                                                                         \
    gme_err_t err_ = (expr);                                                   \
    if (err_)                                                                  \
      return err_;                                                             \
  } while (0)

// Number of audio buffers per second. Adjust if you encounter audio skipping.
const int fill_rate = 45;

// Simple sound driver using SDL
typedef void (*sound_callback_t)(void *data, short *out, int count);
static const char *sound_init(long sample_rate, int buf_size, sound_callback_t,
                              void *data);
static void sound_start();
static void sound_stop();
static void sound_cleanup();

Player::Player() {
  emu_ = 0;
  paused = false;
  track_info_ = nullptr;
}

gme_err_t Player::init(long rate) {
  sample_rate = rate;

  int min_size = sample_rate * 2 / fill_rate;
  int buf_size = 512;
  while (buf_size < min_size)
    buf_size *= 2;

  return sound_init(sample_rate, buf_size, fill_buffer, this);
}

void Player::stop() {
  sound_stop();
  gme_delete(emu_);
  emu_ = nullptr;
}

Player::~Player() {
  stop();
  sound_cleanup();
  gme_free_info(track_info_);
}

gme_err_t Player::load_file(const string &path) {
  filename_ = path;

  stop();

  RETURN_ERR(gme_open_file(path.c_str(), &emu_, sample_rate));

  char m3u_path[256 + 5];
  strncpy(m3u_path, path.c_str(), 256);
  m3u_path[256] = 0;
  char *p = strrchr(m3u_path, '.');
  if (!p)
    p = m3u_path + strlen(m3u_path);
  strcpy(p, ".m3u");
  if (gme_load_m3u(emu_, m3u_path)) {
  } // ignore error

  return 0;
}

int Player::track_count() const { return emu_ ? gme_track_count(emu_) : false; }

gme_err_t Player::start_track(int track) {
  if (emu_) {
    gme_free_info(track_info_);
    track_info_ = nullptr;
    RETURN_ERR(gme_track_info(emu_, &track_info_, track));

    // Sound must not be running when operating on emulator
    sound_stop();
    RETURN_ERR(gme_start_track(emu_, track));

    // Calculate track length
    if (track_info_->length <= 0)
      track_info_->length =
          track_info_->intro_length + track_info_->loop_length * 2;

    if (track_info_->length <= 0)
      track_info_->length = (long)(2.5 * 60 * 1000);
    gme_set_fade(emu_, track_info_->length);

    paused = false;
    sound_start();
  }
  return 0;
}

void Player::pause(int b) {
  paused = b;
  if (b)
    sound_stop();
  else
    sound_start();
}

void Player::suspend() {
  if (!paused)
    sound_stop();
}

void Player::resume() {
  if (!paused)
    sound_start();
}

bool Player::track_ended() const {
  return emu_ ? gme_track_ended(emu_) : false;
}

void Player::set_stereo_depth(double tempo) {
  suspend();
  gme_set_stereo_depth(emu_, tempo);
  resume();
}

void Player::enable_accuracy(bool b) {
  suspend();
  gme_enable_accuracy(emu_, b);
  resume();
}

void Player::set_tempo(double tempo) {
  suspend();
  gme_set_tempo(emu_, tempo);
  resume();
}

void Player::mute_voices(int mask) {
  suspend();
  gme_mute_voices(emu_, mask);
  gme_ignore_silence(emu_, mask != 0);
  resume();
}

void Player::fill_buffer(void *data, sample_t *out, int count) {
  Player *self = (Player *)data;
  if (self->emu_) {
    if (gme_play(self->emu_, count, out)) {
    } // ignore error
  }
}

// Sound output driver using SDL

#include "SDL2/SDL.h"

static sound_callback_t sound_callback;
static void *sound_callback_data;

static void sdl_callback(void *data, Uint8 *out, int count) {
  if (sound_callback)
    sound_callback(sound_callback_data, (short *)out, count / 2);
}

static const char *sound_init(long sample_rate, int buf_size,
                              sound_callback_t cb, void *data) {
  sound_callback = cb;
  sound_callback_data = data;

  static SDL_AudioSpec as; // making static clears all fields to 0
  as.freq = sample_rate;
  as.format = AUDIO_S16SYS;
  as.channels = 2;
  as.callback = sdl_callback;
  as.samples = buf_size;
  if (SDL_OpenAudio(&as, 0) < 0) {
    const char *err = SDL_GetError();
    if (!err)
      err = "Couldn't open SDL audio";
    return err;
  }

  return 0;
}

static void sound_start() { SDL_PauseAudio(false); }

static void sound_stop() {
  SDL_PauseAudio(true);

  // be sure audio thread is not active
  SDL_LockAudio();
  SDL_UnlockAudio();
}

static void sound_cleanup() {
  sound_stop();
  SDL_CloseAudio();
}
