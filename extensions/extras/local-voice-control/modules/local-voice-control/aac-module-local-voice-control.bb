SUMMARY = "Alexa Automotive Core - Local Voice Control Module"
HOMEPAGE = "http://github.com/alexa"
LICENSE = "CLOSED"

DEPENDS = "local-voice-control aac-module-core aac-module-alexa  aac-module-local-skill-service"

PV = "0.99.0"

inherit aac-module devlibsonly

# OE Builds can't seem to cope up with INTERFACE (header only) libs
# It is forcing to link with the header only library and doesn't find
# the linked library. Temporarily disabling it.
EXTRA_OECMAKE += "-DOE_DISABLE_TESTS=ON"
