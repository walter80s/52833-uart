nrfutil settings generate --family NRF52 --application nrf52833_xxaa.hex --application-version 3 --bootloader-version 2 --bl-settings-version 1 settings.hex

mergehex.exe -m nrf52833_xxaa.hex s140_nrf52_7.2.0_softdevice.hex bootloader.hex -o merged_all.hex

mergehex.exe -m merged_all.hex settings.hex -o all.hex

rm -rf settings.hex

rm -rf merged_all.hex
