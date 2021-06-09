require avs-device-sdk_1.22.0.inc

FILESEXTRAPATHS_prepend := "${THISDIR}/${P}:"

DEPENDS += "openssl"

SRC_URI += "file://0001-Enable-Hybrid-Router-build.patch"

EXTRA_OECMAKE += "-DENABLE_HR=ON"
EXTRA_OECMAKE_append_android = " -DOPENSSL_CRYPTO_LIBRARY=${STAGING_LIBDIR}/libcrypto.a \
                                 -DOPENSSL_INCLUDE_DIRS=${STAGING_INCDIR} \
                                 -DOPENSSL_SSL_LIBRARY=${STAGING_LIBDIR}/libssl.a"
