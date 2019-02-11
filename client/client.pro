TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.c

LIBS += -ljson-c -lubox -lubus -lblobmsg_json
