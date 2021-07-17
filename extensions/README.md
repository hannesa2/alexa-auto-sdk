# Extras

Several preview extensions are available from the Amazon developer portal.

These need to be downloaded and built as well, for a full Alexa-coassistant experience.

## Download extensions

The following extensions are mirrored on the BetaLabs Teams space:
- [amazonlite](https://bmwgroup.sharepoint.com/teams/BETAlabsrats/Freigegebene%20Dokumente/Personal%20Assistant/aac-sdk-2.2/amazonlite-patched-2.2.zip) (not necessary, if amazonlite-coassistant is being used)
- [amazonlite-coassistant](https://bmwgroup.sharepoint.com/teams/BETAlabsrats/Freigegebene%20Dokumente/Personal%20Assistant/aac-sdk-2.2/amazonlite-coassistant-patched-2.2.zip)
- [coassistant](https://bmwgroup.sharepoint.com/teams/BETAlabsrats/Freigegebene%20Dokumente/Personal%20Assistant/aac-sdk-2.2/co-assistant-patched-2.2.zip)
- [local-voice-control](https://bmwgroup.sharepoint.com/teams/BETAlabsrats/Freigegebene%20Dokumente/Personal%20Assistant/aac-sdk-2.2/local-voice-control-coassistant-patched-2.2.zip)

Download them into the `extras` folder.
 
 **Access to BetaLabs Microsoft Teams is required.**

## Unzip the extensions

Run the `setup.sh` script to unpack the downloaded extensions automatically.

The extensions are already patched, to be compatible with modern gradle version. 

**Do NOT commit these extensions!**

## Build

Building the SDK with the extensions is documented [here](https://gitea.k8s.betalabs.rocks/BETALabs/alexa-assistant/wiki).


