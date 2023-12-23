<h1 align="center">ml_synth_sampler_example</h1>
<h3 align="center"Arduino synthesizer sampler with loader for wav and sf2 soundfont files</h3>  
<p align="center"> 
  <img src="img/splash.jpg" alt="project picture" width="480px" height="270px"><br>
  <a href="https://youtu.be/G6Go3KIiWPE">link to the video ESP32</a>
</p>



The required library can be found here: https://github.com/marcel-licence/ML_SynthTools
Version 1.2.0 will be required (will be released soon)

Supported versions of board library
---
This project is an example supporting the following platforms:
- ESP32, RP2040: for more details look <a href="doc/board_info.md">here</a>


I would recommend to take a look at the loaddata_examples.ino
Here you would find different possibilities to load data into the sampler:
- single wav file to preset (all keys used for playback of the same sample with different pitch)
- wav files to different keys (each key can playback another sample)
- folder to different keys (wav files contained in a folder mapped to different keys)
- folder to presets (each file found in a folder will be used as a new sound)
- samples from soundfont (ignoring the rest within the soundfont the samples only will be loaded)
- instruments from soundfont (this allows to get key mapping: notes and velocity)
- load complete soundfont (the sample data will be copied completely and loading the properties)

Some features of soundfonts are not supported (like modulators etc.)

More information will be available in future