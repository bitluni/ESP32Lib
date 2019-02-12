# bitluni's ESP32 Libraries
A collection of libraries for the ESP32 including VGA graphics, sound and game controllers.
More examples and documentation will follow with newer versions.
Check out https://youtube.com/bitlunislab for project updates

If you found it useful please consider supporting my work on

Patreon https://patreon.com/bitluni
Paypal https://paypal.me/bitluni

# Acknowledgements
Thanks to Ivan Grokhotkov & Jeroen Domburg (aka Sprite_tm) for their great work on I2S revealing some nitty-gritty details and quirks of the ESP32.

# License
bitluni 2019
Creative Commons Attribution ShareAlike 4.0
https://creativecommons.org/licenses/by-sa/4.0/

If you need another license, please feel free to contact me

# Documentation

## Pin configuration

There are limitation on which the VGA output an DACs can work:

I/O GPIO pads are 0-19, 21-23, 25-27, 32-39
Input/Output GPIOs are 0-19, 21-23, 25-27, 32-33. 
GPIO pads 34-39 are input-only.

Beware of pin 0. It selects the boot mode.
It can only be used as any of the syncs. (no resistors attached)

Pin 5 is often tied to the LED but can be used as hSync if you don't need the LED.
I²C is on 21 (SDA), 22(SCL)
DACs are 25, 26
ins 1(TX), 3(RX) sould only be used if really don't need to communicate anymore.

Here is an overview for your convenience:
(0), 2, 4, (5), 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 27, 32, 33


