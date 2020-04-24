#include <math.h>
#include <limits.h>

#include "SDL2/SDL.h"

#include "global.h"
#include "settings.h"

#include "audio.h"

namespace audio
{

    SDL_AudioSpec outSpec;
    SDL_AudioDeviceID outDev;
    
    SDL_Window *window;
    
    track_t *musicTrack;
    track_t *oldMusic;
    track_t *playingSounds[MaxSfx];
    
    track_t *createTrack(sound_t *sound, double volume)
    {
        track_t *track = (track_t *)malloc(sizeof(*track));
        track->sound = sound;
        track->seek = 0;
        track->loop = 0;
        track->defVol = volume;
        track->volume = volume;
        int w;
        SDL_GetWindowSize(window, &w, NULL);
        dbgprint("w = %d", w);
        track->x_pos = w / 2;
        track->pitchShift = 0;
        track->volShift = 0;
        track->shiftFrames = 0;
        track->end = 0;
        return track;
    }
    void freeTrack(track_t *track)
    {
        free(track);
    }
    int enqueueSound(track_t *track)
    {
        for (int i = 0; i < MaxSfx; ++i)
        {
            if (playingSounds[i] == NULL)
            {
                playingSounds[i] = track;
                return i;
            }
        }
        return -1;
    }
    
    // Fades in music over the course of fadein frames, ending the previous music playing
    track_t *playMusic(sound_t *music, int fadein)
    {
        track_t *track = createTrack(music, MusicVolume);
        track->volume = 0.0;
        if (oldMusic != NULL)
            freeTrack(oldMusic);
        oldMusic = musicTrack;
        musicTrack = track;
        if (oldMusic != NULL)
        {
            rebalance(oldMusic, -oldMusic->volume, fadein);
            oldMusic->end = 1;
        }
    
        if (track->sound->endLoop > 0)
            track->loop = 1;
        return rebalance(track, track->defVol, fadein);
    }
    // Plays a sound effect pitchShifted by pitchShift semitones
    track_t *playSound(sound_t *sound, int x_pos, double pitchShift)
    {
        track_t *track = createTrack(sound, SfxVolume);
        track->pitchShift = pitchShift;
        track->x_pos = x_pos;
        enqueueSound(track);
        return track;
    }
    // Fades out the music over the course of fadeout frames
    void stopMusic(int fadeOut)
    {
        rebalance(musicTrack, -musicTrack->volume, fadeOut);
        musicTrack->end = 1;
    }
    // Fades out the sound effect pointed to by track over the course of fadeOut frames
    void stopSound(track_t *track, int fadeOut)
    {
        rebalance(track, -track->volume, fadeOut);
        track->end = 1;
    }
    // Cross fades between two sounds over the course of fadeTime frames
    track_t *crossFade(track_t *track, sound_t *newSound, int fadeTime)
    {
        track_t *newSfx = createTrack(newSound, SfxVolume);
        rebalance(track, -track->volume, fadeTime);
        track->end = 1;
        newSfx->volume = 0.0;
        rebalance(newSfx, newSfx->defVol, fadeTime);
        enqueueSound(newSfx);
        return newSfx;
    }
    // Shifts the volume by volShift percentage points (in decimal notation) of a track over transTime frames
    track_t *rebalance(track_t *track, double volShift, int transTime)
    {
        track->shiftFrames = transTime;
        track->volShift = volShift / (double)transTime;
        return track;
    }
    // resets the balance of a track to default over transTime frames
    track_t *resetBalance(track_t *track, int transTime)
    {
        return rebalance(track, track->defVol - track->volume, transTime);
    }
    
    // loads an audio file into a sound_t
    sound_t *loadAudio(char *filename)
    {
        sound_t *audio = new sound_t;
        SDL_AudioSpec *spec = SDL_LoadWAV(filename, &audio->spec, &audio->audio_buf,
                                          &audio->audio_len);
        if (spec == NULL)
        {
            free(audio);
            return NULL;
        }
    
        SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, audio->spec.format, audio->spec.channels, audio->spec.freq,
                          outSpec.format, 1, outSpec.freq);
    
        if (cvt.needed)
        {
            cvt.len = audio->audio_len;
            cvt.buf = (Uint8 *)realloc(audio->audio_buf, cvt.len * cvt.len_mult);
            SDL_ConvertAudio(&cvt);
            audio->spec = outSpec;
            audio->spec.channels = 1;
            audio->audio_buf = cvt.buf;
            audio->audio_len = cvt.len_cvt;
        }
    
        audio->startLoop = 0;
        audio->endLoop = 0;
        return audio;
    }
    // load looped audio, audio will be played from beginning,
    // then when the index endLoop is hit, will jump back to index startLoop
    sound_t *loadAudioLooped(char *filename, uint32_t startLoop, uint32_t endLoop)
    {
        sound_t *audio = loadAudio(filename);
        if (audio == NULL)
            return NULL;
    
        audio->startLoop = startLoop;
        audio->endLoop = endLoop;
    }
    // frees a sound_t created by loadAudio
    void freeAudio(sound_t *sound)
    {
        if (sound == NULL)
            return;
    
        SDL_FreeWAV(sound->audio_buf);
        free(sound);
    }
    
    int initAudio(SDL_Window* window_ref)
    {   window = window_ref;
        SDL_AudioSpec want;
        want.freq = AudioFrequency;
        want.format = AUDIO_S32SYS;
        want.channels = 2;
        want.samples = 4096;
        want.callback = NULL;
        want.userdata = NULL;
    
        outDev = SDL_OpenAudioDevice(NULL, 0, &want, &outSpec, 0);
        SDL_PauseAudioDevice(outDev, 0);
    }
    
    void updateVolume(track_t **track_p)
    {
        track_t *track = *track_p;
        if (track == NULL)
            return;
        if (track->end && track->shiftFrames == 0)
        {
            freeTrack(track);
            *track_p = NULL;
        }
        if (track->shiftFrames > 0)
        {
            track->volume += track->volShift;
            --track->shiftFrames;
        }
    }
    
    int32_t audLterm(track_t *track)
    {
        double tanpi_4 = tan(M_PI / 4);
        int w;
        SDL_GetWindowSize(window, &w, NULL);
        int x = track->x_pos;
        double theta = atan((2 * x * tanpi_4) / w - tanpi_4);
    
        double Lamp = (sqrt(2.0) / 2.0) * (cos(theta) - sin(theta));
        int32_t value = ((int32_t *)(track->sound->audio_buf))[track->seek];
        return (int32_t)(MasterVolume * track->volume * Lamp * value);
    }
    
    int32_t audRterm(track_t *track)
    {
        double tanpi_4 = tan(M_PI / 4);
        int w;
        SDL_GetWindowSize(window, &w, NULL);
        int x = track->x_pos;
        double theta = atan((2 * x * tanpi_4) / w - tanpi_4);
    
        double Ramp = (sqrt(2.0) / 2.0) * (cos(theta) + sin(theta));
        int32_t value = ((int32_t *)(track->sound->audio_buf))[track->seek];
        return (int32_t)(MasterVolume * track->volume * Ramp * value);
    }
    
    void mixAudio(void)
    {
        int32_t data[(outSpec.freq / FPS) * 2];
        for (int i = 0; i < outSpec.freq / FPS; ++i)
        {
            int32_t sumL = 0;
            int32_t sumR = 0;
            if (musicTrack && !musicTrack->end)
            {
                sumL += audLterm(musicTrack);
                sumR += audRterm(musicTrack);
                ++musicTrack->seek;
                if (musicTrack->loop && musicTrack->seek >= musicTrack->sound->endLoop)
                    musicTrack->seek = musicTrack->sound->startLoop;
                if (musicTrack->seek >= musicTrack->sound->audio_len / sizeof(int32_t))
                    musicTrack->end = 1;
            }
            if (oldMusic && !oldMusic->end)
            {
                sumL += audLterm(oldMusic);
                sumR += audRterm(oldMusic);
                ++oldMusic->seek;
                if (oldMusic->loop && oldMusic->seek >= oldMusic->sound->endLoop)
                    oldMusic->seek = oldMusic->sound->startLoop;
                if (oldMusic->seek >= oldMusic->sound->audio_len / sizeof(int32_t))
                    oldMusic->end = 1;
            }
    
            for (int j = 0; j < MaxSfx; ++j)
            {
                if (playingSounds[j] && !playingSounds[j]->end)
                {
                    sumL += audLterm(playingSounds[j]);
                    sumR += audRterm(playingSounds[j]);
                    ++playingSounds[j]->seek;
                    if (playingSounds[j]->loop &&
                        playingSounds[j]->seek >= playingSounds[j]->sound->endLoop)
                        playingSounds[j]->seek = playingSounds[j]->sound->startLoop;
                    if (playingSounds[j]->seek >=
                        playingSounds[j]->sound->audio_len / sizeof(int32_t))
                        playingSounds[j]->end = 1;
                }
            }
    
            data[2 * i] = sumL;
            data[2 * i + 1] = sumR;
        }
        if (SDL_QueueAudio(outDev, data, sizeof(data))){
            const char* err = SDL_GetError();
            errprint("%s\n", err);
        }
    }
    
    int updateAudio(void)
    {
        updateVolume(&oldMusic);
        updateVolume(&musicTrack);
        for (int i = 0; i < MaxSfx; ++i)
            updateVolume(&playingSounds[i]);
    
        mixAudio();
    }
    
    int destroyAudio(void)
    {
    }
} // namespace audio