DESCRIPTION = "Burn key for sn&mac"
SECTION = "libs"
LICENSE = "MIT"
PV = "3"
PR = "r0"

SRC_URI = " \
          file://burnkey.c \
          file://makefile \
          "
TARGET_CC_ARCH += "${LDFLAGS}"

LIC_FILES_CHKSUM = "file://burnkey.c;md5=8492de9e45939e5494b464f93a495078"
S = "${WORKDIR}"
do_compile () {
    make
}

do_install () {
    install -d ${D}${bindir}/
    install -m 0755 ${S}/burn_key ${D}${bindir}/
}

FILES_${PN} = "${bindir}/burn_key"
