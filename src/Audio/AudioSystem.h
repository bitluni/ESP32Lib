/*
	Author: bitluni 2019
	License: 
	Creative Commons Attribution ShareAlike 4.0
	https://creativecommons.org/licenses/by-sa/4.0/
	
	For further details check out: 
		https://youtube.com/bitlunislab
		https://github.com/bitluni
		http://bitluni.net
*/
#pragma once
#include "../Tools/Log.h"

class Sound
{
  public:
  const signed char *samples;
  int sampleCount;
  long long position;
  int positionIncrement;
  int volume;
  bool playing;
  int id;
  bool loop;
  Sound *next;

  void remove(Sound **prevNext)
  {
    *prevNext = next;
    delete this;
  }

  void insert(Sound **prevNext)
  {
    next = *prevNext;
    *prevNext = this;
  }
    
  void init(const signed char *samples, int sampleCount, float volume = 1, float speed = 1, bool loop = false)
  {
    next = 0;
    id = 0;
    this->samples = samples;
    this->sampleCount = sampleCount;
    this->loop = loop;
    position = 0;
    positionIncrement = int(65536 * speed);
    this->volume = volume * 256;
    playing = true;
  }

  ///returns the next rendered sample. 16bit since it's scaled by volume
  int nextSample()
  {
    if(!playing) return 0;
    int s = samples[position >> 16] * volume;
    position += positionIncrement;
    if((position >> 16) >= sampleCount)
    {
      if(loop)
        position = 0;
      else
        playing = false;
    }
    return s;
  }
};

class AudioSystem
{
  public:
  Sound *sounds;
  int samplingRate;
  unsigned char *buffer;
  int bufferSize;
  bool swapped;
  int currentId;
  volatile int readPosition;
  int writePosition;
  int volume;
  
  AudioSystem(int samplingRate, int bufferSize)
  {
    this->samplingRate = samplingRate;
    this->bufferSize = bufferSize;
    sounds = 0;
    buffer = (unsigned char*)malloc(bufferSize * sizeof(unsigned char));
	if(!buffer)
		ERROR("Not enough memory for audio buffer");
    for(int i = 0; i < bufferSize; i++)
      buffer[i] = 128;
    swapped = true;
    currentId = 0;
    readPosition = 0;
    writePosition = 0;
    volume = 256;
  }
  
  ///plays a sound, and returns an idividual id
  int play(Sound *sound)
  {
    sound->id = currentId;
    sound->insert(&sounds);
    return currentId++;
  }

  ///fills the buffer with all saounds that are currently playing
  void calcSamples()
  {
    int samples = readPosition - writePosition;
    if(samples < 0)
      samples += bufferSize;
    for(int i = 0; i < samples; i++)
    {
      int sample = 0;
      Sound **soundp = &sounds;
      while(*soundp)
      {
        sample += (*soundp)->nextSample();
        if(!(*soundp)->playing)
          (*soundp)->remove(soundp);
        else
          soundp = &(*soundp)->next;
      }
      if(sample >= (1 << 15)) 
        sample = (1 << 15) - 1;
      else if(sample < -(1 << 15)) 
        sample = -(1 << 15);
      sample = ((sample * volume) >> 16) + 128;
      buffer[writePosition] = sample;
      writePosition++;
      if(writePosition >= bufferSize)
        writePosition = 0;
    }
  }

  inline unsigned char nextSample()
  {
    unsigned char s = buffer[readPosition];
    readPosition++;
    if(readPosition >= bufferSize)
      readPosition = 0;
    return s;
  }

  ///stops playing a sound with an individual id
  void stop(int id)
  {
    Sound **soundp = &sounds;
    while(*soundp)
    {
      if((*soundp)->id == id)
      {
        (*soundp)->remove(soundp);
        return;
      }
      soundp = &(*soundp)->next;
    }
  }

  ///stops playing all sounds of an specific sample pointer
  void stopBySample(const signed char *samples)
  {
    Sound **soundp = &sounds;
    while(*soundp)
    {
      if((*soundp)->samples == samples)
        (*soundp)->remove(soundp);
      else
        soundp = &(*soundp)->next;
    }
  }  
};

class Wavetable
{
  public:
  const signed char *samples;
  int soundCount;
  const int *offsets;
  int samplingRate;
  
  Wavetable(const signed char *samples, int soundCount, const int *offsets, int samplingRate)
  {
    this->samples = samples;
    this->soundCount = soundCount;
    this->offsets = offsets;
    this->samplingRate = samplingRate;
  }

  int play(AudioSystem &audioSystem, int soundNumber, float amplitude = 1, float speed = 1, bool loop = false)
  {
    Sound *sound = new Sound();
    sound->init(&samples[offsets[soundNumber]], offsets[soundNumber + 1] - offsets[soundNumber], amplitude, speed * samplingRate / audioSystem.samplingRate, loop);
    return audioSystem.play(sound);
  }

  void stop(AudioSystem &audioSystem, int id)
  {
    audioSystem.stop(id);
  }

  void stop(AudioSystem &audioSystem)
  {
    for(int i = 0; i < soundCount; i++)
      audioSystem.stopBySample(&samples[offsets[i]]);
  }
};
