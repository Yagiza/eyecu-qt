#Get coreect path to lrelease binary
LRELEASE = $$[QT_INSTALL_BINS]/lrelease
win32|os2: LRELEASE = $$replace(LRELEASE, \\\\, \\\\)
win32|os2: LRELEASE = $$replace(LRELEASE, /, \\\\)
  

#Building Translations
qm_build_all.target = build_qm
for(LANG, TRANS_LANGS) {
  TS_FILE = $${_PRO_FILE_PWD_}/$${TRANS_SOURCE_ROOT}/translations/$${LANG}/$${TRANS_SOURCE}.ts
  exists($${TS_FILE}) {
  QM_FILE = $${TRANS_BUILD_ROOT}/src/translations/$${LANG}/$${TRANS_TARGET}.qm
  QM_DIR = $${TRANS_BUILD_ROOT}/src/translations/$${LANG}
  win32|os2: QM_DIR = $$replace(QM_DIR, /, \\\\)

    QM_COMMAND = $(MKDIR) $${QM_DIR} && $${LRELEASE} -compress $${TS_FILE} -qm $${QM_FILE}
    win32|os2: QM_COMMAND = ( if not exist $${QM_DIR} ( $(MKDIR) $${QM_DIR} ) ) & $${LRELEASE} $${TS_FILE} -qm $${QM_FILE}

    eval(qm_$${TRANS_TARGET}_build_$${LANG}.target   = $${QM_FILE})
    eval(qm_$${TRANS_TARGET}_build_$${LANG}.depends  = $${TS_FILE})
    eval(qm_$${TRANS_TARGET}_build_$${LANG}.commands = $${QM_COMMAND})

    qm_build_all.depends += qm_$${TRANS_TARGET}_build_$${LANG}
    QMAKE_EXTRA_TARGETS  += qm_$${TRANS_TARGET}_build_$${LANG}

	win32|os2: QM_FILE = $$replace(QM_FILE, /, \\\\)
    QMAKE_DISTCLEAN += $${QM_FILE}
  }
}
QMAKE_EXTRA_TARGETS  += qm_build_all
POST_TARGETDEPS += $${qm_build_all.target}


#Install Translations
for(LANG, TRANS_LANGS) {
  INST_DIR  = $${INSTALL_TRANSLATIONS}/$${LANG}
	win32|os2: INST_DIR = $$replace(INST_DIR, /, \\\\)
	
  INST_FILE = $${TRANS_BUILD_ROOT}/src/translations/$${LANG}/$${TRANS_TARGET}.qm
	win32|os2: INST_FILE = $$replace(INST_FILE, /, \\\\)
	
  eval(qm_$${TRANS_TARGET}_inst_$${LANG}.path  = $${INST_DIR})
  eval(qm_$${TRANS_TARGET}_inst_$${LANG}.files = $${INST_FILE})

  #If file is not exits right now, qmake will not install it
  !exists($${INST_FILE}): eval(qm_$${TRANS_TARGET}_inst_$${LANG}.extra = "-$(INSTALL_FILE)" \\\"$${INST_FILE}\\\" \\\"$(INSTALL_ROOT)$${INST_DIR}\\\")

  INSTALLS += qm_$${TRANS_TARGET}_inst_$${LANG}
}

