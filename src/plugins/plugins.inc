include(../make/config.inc)

TEMPLATE            = lib
CONFIG             += plugin
QT                 += network xml

greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
	CONFIG += c++11
} else {
	win32-g++:QMAKE_CXXFLAGS += -std=c++11
}

LIBS               += -L../../libs
LIBS               += -l$$EYECU_UTILS_NAME
DEPENDPATH         += ../..
INCLUDEPATH        += ../..
win32|os2 {
  DLLDESTDIR        = ..\\..\\..\\plugins
  QMAKE_DISTCLEAN  += $${DLLDESTDIR}\\$${TARGET}.dll
}
unix:DESTDIR        = ../../../plugins

#Install
target.path         = $$INSTALL_PLUGINS
INSTALLS           += target

#Translation
TRANS_BUILD_ROOT   = $${OUT_PWD}/../../..
TRANS_SOURCE_ROOT   = ../..
include(../translations/languages.inc)

#MacOS Install
macx {
  UTILS_LIB_NAME   = lib$${EYECU_UTILS_NAME}.$${EYECU_UTILS_ABI}.dylib
  name_tool.path    = $$INSTALL_PLUGINS
  name_tool.extra   = install_name_tool -change $$UTILS_LIB_NAME @executable_path/../Frameworks/$$UTILS_LIB_NAME $(INSTALL_ROOT)$$INSTALL_PLUGINS/lib$${TARGET}.dylib
  INSTALLS         += name_tool
}

symbian: include(../symbianpkgrules.pri)
