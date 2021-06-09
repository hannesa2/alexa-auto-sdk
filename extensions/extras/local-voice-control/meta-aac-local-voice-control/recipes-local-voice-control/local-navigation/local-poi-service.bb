SUMMARY = "Local Search POI Service libraries"
HOMEPAGE = "https://github.com/alexa/auto-sdk"
LICENSE = "CLOSED"

DEPENDS = "local-voice-control"

SRC_URI = "file://${LOCALVOICECONTROL_PACKAGES}/local-poi-service-release-2020-11-24.tar.gz;subdir=${P}"

# Libraries in source tarball are un-versioned. Skip PN-dev and package directly in PN.
SOLIBS = ".so"
FILES_SOLIBSDEV = ""

def get_local_search_arch(d):
    binary_arch = aac.get_external_binary_arch(d)
    # Note: Arch dir names in the source tarball match the AAC build platform-arch format already
    if binary_arch == "android-armv7a":
        return "android-armv7a"
    elif binary_arch == "android-aarch64":
        return "android-aarch64"
    elif binary_arch == "android-x86":
        return "android-x86"
    elif binary_arch == "android-x86_64":
        return "android-x86_64"
    elif binary_arch == "linux-armv7a":
        return "linux-armv7a"
    elif binary_arch == "linux-armv7ahf":
        return "linux-armv7ahf"
    elif binary_arch == "linux-aarch64":
        return "linux-aarch64"
    elif binary_arch == "linux-x86_64":
        return "linux-x86_64_gcc54"
    else:
        raise bb.parse.SkipPackage("Arch %s is not supported" % binary_arch)

LOCAL_SEARCH_ARCH ?= "${@get_local_search_arch(d)}"

do_install() {
    mkdir -p ${D}${libdir}
    localsearch_libdir_src=${S}/${LOCAL_SEARCH_ARCH}/lib

    cp ${localsearch_libdir_src}/libAlexaHybridPOIERService.so ${D}${libdir}
    cp ${localsearch_libdir_src}/libAlexaHybridPOISearchLibrary.so ${D}${libdir}
    cp ${localsearch_libdir_src}/libahe_external_er_client.so ${D}${libdir}
    cp ${localsearch_libdir_src}/libahe_external_er_common.so ${D}${libdir}
    cp ${localsearch_libdir_src}/libahe_external_er_server.so ${D}${libdir}

    # This package also depends on the following common libs provided by the local-voice-control package:
    #   libahe_configuration.so
    #   libahe_logger.so
    #   libahe_platform_interfaces_ipc.so
    #   libboost_system.so
    #   libboost_thread.so
    #   libboost_filesystem.so
    #   libnanomsg.so
    #   libboost_program_options.so

    chmod -R 755 ${D}${libdir}/lib*

    mkdir -p ${D}${includedir}
    cp -R ${S}/include/* ${D}${includedir}
}