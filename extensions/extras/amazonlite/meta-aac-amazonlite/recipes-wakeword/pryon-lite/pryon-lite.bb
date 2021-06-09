python () {
    if d.getVar('PV') == "1.0":
        pv = aac.find_latest_version("pryon_lite_", d.getVar('AMAZONLITE_PACKAGES') + "/*.zip")
        if pv:
            d.setVar('PV', pv)
        else:
            bb.fatal("Could not find a suitable pryon_lite package!")
}

python check_pryon_lite_package_version() {
    pv = d.getVar('PV')
    dv = d.getVar('PRYON_LITE_DEFAULT_VERSION')
    if pv != dv:
        bb.warn("Using pryon_lite package version " + pv + " instead of default version " + dv)
}

do_fetch[prefuncs] += "check_pryon_lite_package_version "
do_fetch[vardepsexclude] += "check_pryon_lite_package_version "

PV[vardepvalue] = "1.0"

SUMMARY = "Low Power Wake Word Engine"
HOMEPAGE = "https://developer.amazon.com/avs/sdk"
LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://LICENSE.txt;md5=ea1633002b92669fa132d44dd155705b"

SRC_URI = "file://${AMAZONLITE_PACKAGES}/pryon_lite_${PV}-*.zip;subdir=src"

S = "${WORKDIR}/src"

INSANE_SKIP_${PN} += "already-stripped"

def get_pryon_lite_arch(d):
    binary_arch = aac.get_external_binary_arch(d)
    if binary_arch == "android-armv7a":
        return "android-armv7a"
    elif binary_arch == "android-aarch64":
        return "android-armv8a"
    elif binary_arch == "android-x86":
        return "android-x86"
    elif binary_arch == "android-x86_64":
        return "android-x64"
    elif binary_arch == "qnx7-aarch64":
        return "qnx-arm-aarch64"
    elif binary_arch == "qnx7-x86_64":
        return "qnx-x86-64"
    elif binary_arch == "linux-x86_64":
        return "x86"
    elif binary_arch == "linux-armv7ahf":
        return "armv7a-linaro731"
    elif binary_arch == "linux-armv7a":
        return "armv7a-softfp-linaro731"
    elif binary_arch == "linux-aarch64":
        return "armv8a-linaro731"
    else:
        raise bb.parse.SkipPackage("Arch %s is not supported" % binary_arch)

def get_pryon_lite_model_format_mapping(d):
    features = d.getVar('TUNE_FEATURES').split(' ')
    if ("aarch64" in features) or ("arm" in features):
        return "f8"
    else:
        return "common"

PRYON_LITE_FORMAT_MAPPING ?= "${@get_pryon_lite_model_format_mapping(d)}"

PRYON_LITE_ARCH ?= "${@get_pryon_lite_arch(d)}"
PRYON_LITE_DEFAULT_VERSION = "1.13.5"

do_install() {
	libname="pryon_lite"
	mkdir -p ${D}${libdir}
	cp ${S}/${PRYON_LITE_ARCH}/lib${libname}.a ${D}${libdir}
	cp ${S}/${PRYON_LITE_ARCH}/lib${libname}${SOLIBS} ${D}${libdir}
	# Note: It is safe to create a symlink manually here because sometime the
	#       zip package contains hardcopy rather than symlink
	if [ ! -f ${D}${libdir}/lib${libname}.so ]; then
		soname=$(basename $(find ${D}${libdir} -maxdepth 1 -name "lib${libname}${SOLIBS}"))
		ln -s ${soname} ${D}${libdir}/lib${libname}.so
	fi

	mkdir -p ${D}${includedir}
	if [ -f ${S}/${PRYON_LITE_ARCH}/pryon_lite.h ]; then
		cp ${S}/${PRYON_LITE_ARCH}/*.h ${D}${includedir}
	else
		cp ${S}/pryon_lite.h ${D}${includedir}
	fi
	mkdir -p ${D}${datadir}/${BPN}

	# Copying locale model file
	mkdir -p ${D}${datadir}/${BPN}/models
	cp -R ${S}/models/${PRYON_LITE_FORMAT_MAPPING}/* ${D}${datadir}/${BPN}/models
	cp ${S}/${PRYON_LITE_ARCH}/localeToModels.json ${D}${datadir}/${BPN}/models
}
