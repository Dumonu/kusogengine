#ifndef AUDIO_H_
#define AUDIO_H_

namespace audio{
    typedef struct {
        SDL_AudioSpec spec;
        uint8_t* audio_buf;
        uint32_t audio_len;
        uint32_t startLoop;
        uint32_t endLoop;
    } sound_t;
    
    typedef struct {
        sound_t* sound;
        uint32_t seek;
        uint8_t loop;
        double defVol;
        double volume;
        int x_pos; // origin in screen space
        double pitchShift;
        double volShift;
        int shiftFrames;
        uint8_t end;
    } track_t;
    
    // Fades in music over the course of fadein frames, ending the previous music playing
    track_t* playMusic(sound_t* music, int fadein);
    // Plays a sound effect pitchShifted by pitchShift semitones
    track_t* playSound(sound_t* sound, int x_pos, double pitchShift);
    // Fades out the music over the course of fadeout frames
    void stopMusic(int fadeOut);
    // Fades out the sound effect pointed to by track over the course of fadeOut frames
    void stopSound(track_t* track, int fadeOut);
    // Cross fades between two sounds over the course of fadeTime frames
    track_t* crossFade(track_t* track, sound_t* newSound, int fadeTime);
    // Shifts the volume by volShift percentage points (in decimal notation) of a track over transTime frames
    track_t* rebalance(track_t* track, double volShift, int transTime);
    // resets the balance of a track to default over transTime frames
    track_t* resetBalance(track_t* track, int transTime);
    
    // loads an audio file into a sound_t
    sound_t* loadAudio(char* filename);
    // load looped audio, audio will be played from beginning,
    // then when the index endLoop is hit, will jump back to index startLoop
    sound_t* loadAudioLooped(char* filename, uint32_t startLoop, uint32_t endLoop);
    // frees a sound_t created by loadAudio
    void freeAudio(sound_t* sound);
    
    // Game Cycle functions
    int initAudio(SDL_Window * window);
    int updateAudio(void);
    int destroyAudio(void);
}
#endif
