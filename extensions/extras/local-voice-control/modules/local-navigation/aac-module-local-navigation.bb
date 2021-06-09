SUMMARY = "Alexa Auto SDK - Local Navigation Module"
HOMEPAGE = "http://github.com/alexa"
LICENSE = "CLOSED"

DEPENDS = "local-poi-service aac-module-core aac-module-local-skill-service nlohmann"

PV = "0.99.0"

inherit aac-module devlibsonly

# OE Builds can't seem to cope up with INTERFACE (header only) libs
# It is forcing to link with the header only library and doesn't find
# the linked library. Temporarily disabling it.
EXTRA_OECMAKE += "-DOE_DISABLE_TESTS=ON"
