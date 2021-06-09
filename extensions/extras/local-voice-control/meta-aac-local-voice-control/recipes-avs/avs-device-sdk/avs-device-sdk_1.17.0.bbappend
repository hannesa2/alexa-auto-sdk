FILESEXTRAPATHS_prepend := "${THISDIR}/${P}:"

# Alexa Auto modifications
# * Hybrid Proxy
SRC_URI += "file://0001-LVC-support-for-AVS-Device-SDK-1.17.patch"
SRC_URI += "file://0002-Remove-SetAgent-directive-from-log-output.patch"
