TRANSLATIONS = $$PWD/app_ru.ts $$PWD/app_en.ts

QMAKE_EXTRA_TARGETS += translations
translations.target = lrelease
translations.commands = lrelease $$TRANSLATIONS
translations.CONFIG += no_link