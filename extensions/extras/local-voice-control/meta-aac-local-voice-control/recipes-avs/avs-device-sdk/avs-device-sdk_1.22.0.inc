SRC_URI += "file://${LOCALVOICECONTROL_PACKAGES}/hr-avs-1.22-0e25bceb.tar.gz"

do_configure_prepend() {
	cp -R ${WORKDIR}/HybridRouter ${S}
}

do_install_append() {
	# Copy header only files
	mkdir -p ${D}${AAC_PREFIX}/include
	cp -R ${S}/HybridRouter/ThirdParty/ahe_commons/* ${D}${AAC_PREFIX}/include
	cp -R ${S}/HybridRouter/ThirdParty/ahe_imports/mpark ${D}${AAC_PREFIX}/include
	cp -R ${S}/HybridRouter/ThirdParty/ahe_imports/nonstd ${D}${AAC_PREFIX}/include
}
