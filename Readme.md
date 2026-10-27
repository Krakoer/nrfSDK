# What's that?

I first wanted to learn more about BLE internals after viewing a SSTIC presentation about InjectaBLE [1][2].
I wanted to test their implementation [3], but the project did not seem to work in my NRF52840 dongle, so I opened an issue and did not have the time to investigate.

Recently, I managed to free some time, so I started to learn about C and NRF programming, and this is the SDK I'm building along the way.

# How to use

Copy one of the examples folder as a template, and run make/make send.

# Setup

Install `arm-none-eabi-gcc`, `arm-none-eabi-objcopy` using your fav package manager, and `nrfutil` using `uv`: `uv tool install nrfutil==6.1.7 --python 3.10`.

# AI use

As this project is only intened for me to learn how to program my NRF dongle, and as you can tell by the (very) bad code quality, I did not use AI to generate a single LoC in this project.

# References
[1] https://www.sstic.org/2021/presentation/injectable__injection_de_trafic_malveillant_dans_une_connexion_bluetooth_low_energy/
[2] https://ieeexplore.ieee.org/document/9505071
[3] https://github.com/whad-team/butterfly
[4] https://github.com/whad-team/whad-client/issues/257