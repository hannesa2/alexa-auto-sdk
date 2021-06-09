python () {
    if d.getVar('PV') == "1.0":
        pv = aac.find_latest_version("local-voice-engine-", d.getVar('LOCALVOICECONTROL_PACKAGES') + "/*.tar.gz")
        if pv:
            d.setVar('PV', pv)
        else:
            bb.fatal("Could not find a suitable Local Voice Engine archive!")
}

SUMMARY = "Local Voice Engine Platform interface libraries"
HOMEPAGE = "https://github.com/alexa/auto-sdk"
LICENSE = "CLOSED"

SRC_URI = "file://${LOCALVOICECONTROL_PACKAGES}/local-voice-engine-${PV}.tar.gz;subdir=lve"

S = "${WORKDIR}/lve"

# Some libraries are non-versioning ELF
FILES_SOLIBSDEV = ""
FILES_${PN} += "${libdir}/lib*${SOLIBSDEV}"

INSANE_SKIP_${PN} += "already-stripped"

LVE_PREFIX ?= "${@aac.get_external_binary_arch(d)}"

do_install() {
	mkdir -p ${D}${libdir}
	lve_libdir=${S}/${LVE_PREFIX}/lib
	cp ${lve_libdir}/libahe_platform_interfaces_ipc.so ${D}${libdir}
	cp ${lve_libdir}/libahe_logger.so ${D}${libdir}
	cp ${lve_libdir}/libboost_system${SOLIBS} ${D}${libdir}
	cp ${lve_libdir}/libboost_thread${SOLIBS} ${D}${libdir}
	cp ${lve_libdir}/libboost_filesystem${SOLIBS} ${D}${libdir}
	cp ${lve_libdir}/libnanomsg${SOLIBS} ${D}${libdir}

	# XXX: Those libraries are only required for Android
	cp ${lve_libdir}/libboost_program_options${SOLIBS} ${D}${libdir}
	cp ${lve_libdir}/libboost_chrono${SOLIBS} ${D}${libdir}
	if [ -f ${lve_libdir}/libboost_atomic${SOLIBS} ]; then
		cp ${lve_libdir}/libboost_atomic${SOLIBS} ${D}${libdir}
	fi
	if [ -f ${lve_libdir}/libboost_date_time${SOLIBS} ]; then
		cp ${lve_libdir}/libboost_date_time${SOLIBS} ${D}${libdir}
	fi

	mkdir -p ${D}${includedir}
	cp -R ${S}/include/* ${D}${includedir}
        chmod -R 755 ${D}${libdir}/lib*
}
