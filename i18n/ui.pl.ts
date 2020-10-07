<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pl">
<context>
    <name>FirstRunDialog</name>
    <message>
        <location filename="../src/ui/first-run.ui" line="+20"/>
        <source>First run</source>
        <translation>Pierwsze uruchomienie</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Welcome to AppImageLauncher!&lt;/span&gt;&lt;/p&gt;&lt;p&gt;This little helper is designed to improve your AppImage experience on your computer.&lt;/p&gt;&lt;p&gt;It appears you have never run AppImageLauncher before. Please take a minute and configure your preferences. You can always change these later on, using the control panel.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Witaj w AppImageLauncher!&lt;/span&gt;&lt;/p&gt;&lt;p&gt;Ten mały program został napisany, aby ułatwić korzystanie z plików AppImage na twoim komputerze.&lt;/p&gt;&lt;p&gt;Wygląda na to, że nigdy wcześniej nie korzystałeś z AppImageLauncher. Poświęć chwilę i skonfiguruj swoje preferencje. Zawsze możesz je później zmienić za pomocą panelu sterowania.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Ask me whether to move new AppImages into a central location</source>
        <translation>Pytaj czy przenieść nowe pliki AppImages do centralnej lokalizacji</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Integration target destination directory:</source>
        <translation>Docelowy katalog integracji:</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Customize</source>
        <translation>Dostosuj</translation>
    </message>
    <message>
        <location filename="../src/ui/first-run.cpp" line="+86"/>
        <source>Choose integration destination dir</source>
        <translation>Wybierz docelowy katalog do integracji</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>(default)</source>
        <translation>(domyślny)</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <location filename="../src/ui/main.cpp" line="+380"/>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>AppImage has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?
Choosing No will run the AppImage once, and leave the AppImage in its current directory.</source>
        <translation type="vanished">AppImage został już zintegrowany, ale nie ma go w bieżącym katalogu docelowym integracji.

Czy chcesz przenieść go do nowego miejsca docelowego?
Wybranie opcji Nie spowoduje uruchomienie AppImage jeden raz i pozostawienie AppImage w bieżącym katalogu.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AppImage %1 has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?

Choosing No will run the AppImage once, and leave the AppImage in its current directory.

</source>
        <translation>AppImage %1 został już zintegrowany, ale nie ma go w bieżącym docelowym katalogu integracji.

Czy chcesz przenieść go do nowego miejsca docelowego?

Wybranie opcji Nie spowoduje uruchomienie AppImage jeden raz i pozostawienie AppImage w bieżącym katalogu.

</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="+1232"/>
        <location line="+8"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="+23"/>
        <source>Failed to unregister AppImage before re-integrating it</source>
        <translation>Nie udało się wyrejestrować AppImage przed ponownym zintegrowaniem</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-485"/>
        <source>Could not parse desktop file translations:
Could not open file for reading:

%1</source>
        <translation>Nie można przeanalizować tłumaczeń plików pulpitu:
Nie można otworzyć pliku do odczytu:

%1</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Could not parse desktop file translations:
Invalid syntax:

%1</source>
        <translation>Nie można przeanalizować tłumaczeń plików pulpitu:
Niepoprawna składnia:

%1</translation>
    </message>
    <message>
        <location line="+409"/>
        <source>File %1 is owned by another user: %2</source>
        <translation>Plik %1 jest własnością innego użytkownika: %2</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Permissions problem</source>
        <translation>Problem z uprawnieniami</translation>
    </message>
    <message>
        <source>File %1 is owned by another user: %2

Relaunch with their permissions?</source>
        <translation type="vanished">Plik %1 jest własnością innego użytkownika: %2

Uruchomić ponownie z jego uprawnieniami?</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Failed to run permissions helper, exited with return code %1</source>
        <translation>Nie udało się uruchomić pomocnika uprawnień, zakończono z błędem %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Could not find suitable permissions helper, aborting</source>
        <translation>Nie można znaleźć odpowiedniego pomocnika uprawnień, przerywam</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location line="-985"/>
        <location line="+680"/>
        <location filename="../src/ui/remove_main.cpp" line="+106"/>
        <location line="+12"/>
        <location line="+12"/>
        <location filename="../src/ui/update_main.cpp" line="+119"/>
        <location line="+42"/>
        <source>Error</source>
        <translation>Błąd</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="-358"/>
        <source>AppImageLauncher does not support type %1 AppImages at the moment.</source>
        <translation>AppImageLauncher obecnie nie obsługuje plików AppImage typu %1.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Could not make AppImage executable: %1</source>
        <translation>Nie udało się nadać plikowi AppImage: %1 uprawnień do wykonania</translation>
    </message>
    <message>
        <source>Failed to open AppImage for reading: %1</source>
        <translation type="vanished">Nie można otworzyć AppImage do odczytu: %1</translation>
    </message>
    <message>
        <source>Failed to create temporary directory</source>
        <translation type="vanished">Nie udało się utworzyć katalogu tymczasowego</translation>
    </message>
    <message>
        <source>Failed to create temporary copy of type 1 AppImage</source>
        <translation type="vanished">Nie udało się utworzyć tymczasowej kopii AppImage typu 1</translation>
    </message>
    <message>
        <source>Failed to open temporary AppImage copy for writing</source>
        <translation type="vanished">Nie udało się otworzyć tymczasowej kopii AppImage do zapisu</translation>
    </message>
    <message>
        <source>Failed to remove magic bytes from temporary AppImage copy</source>
        <translation type="vanished">Nie udało się usunąć sygnatury z tymczasowej kopii AppImage</translation>
    </message>
    <message>
        <source>execv() failed: %1</source>
        <comment>error message</comment>
        <translation type="vanished">execv() zakończył się błędem: %1</translation>
    </message>
    <message>
        <source>runtime not found: no such file or directory: %1</source>
        <translation type="vanished">nie znaleziono środowiska wykonawczego: brak takiego pliku albo katalogu: %1</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>execv() failed: %1</source>
        <translation>execv() zakończył się błędem: %1</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Usage: %1 [options] &lt;path&gt;</source>
        <translation>Zastosowanie: %1 [opcje] &lt;ścieżka&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Desktop integration helper for AppImages, for use by Linux distributions.</source>
        <translation>Pomocnik integracji pulpitu dla plików AppImage dla dystrybucji Linuksa.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Options:</source>
        <translation>Opcje:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display this help and exit</source>
        <translation>Wyświetl tę pomoc i zakończ</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display version and exit</source>
        <translation>Wyświetl wersję i zakończ</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Arguments:</source>
        <translation>Argumenty:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Path to AppImage (mandatory)</source>
        <translation>Ścieżka do AppImage (obowiązkowa)</translation>
    </message>
    <message>
        <location line="-21"/>
        <source>Failed to clean up old desktop files</source>
        <translation>Nie udało się wykasować starych plików pulpitu</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: error while trying to start appimagelauncherfs.service</source>
        <translation type="vanished">Nie udało się zarejestrować AppImage w AppImageLauncherFS: błąd podczas próby uruchomienia appimagelauncherfs.service</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: failed to register AppImage path %1</source>
        <translation type="vanished">Nie udało się zarejestrować AppImage w AppImageLauncherFS: rejestracja ścieżki AppImage %1 nie powiodła się</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not open map file</source>
        <translation type="vanished">Nie udało się zarejestrować AppImage w AppImageLauncherFS: nie można otworzyć pliku map</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not find virtual file for AppImage</source>
        <translation type="vanished">Nie udało się zarejestrować AppImage w AppImageLauncherFS: nie można znaleźć pliku wirtualnego dla AppImage</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: unknown failure</source>
        <translation type="vanished">Nie udało się zarejestrować AppImage w AppImageLauncherFS: nieznana przyczyna</translation>
    </message>
    <message>
        <location line="+8"/>
        <location filename="../src/ui/remove_main.cpp" line="+1"/>
        <source>Failed to clean up AppImage trash bin: %1</source>
        <translation>Nie udało się wyczyścić kosza aplikacji AppImage: %1</translation>
    </message>
    <message>
        <location line="+48"/>
        <source>Unknown AppImageLauncher option: %1</source>
        <translation>Nieznana opcja AppImageLauncher: %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/ui/remove_main.cpp" line="-74"/>
        <location filename="../src/ui/update_main.cpp" line="-99"/>
        <source>Error: no such file or directory: %1</source>
        <translation>Błąd: brak takiego pliku lub katalogu: %1</translation>
    </message>
    <message>
        <location line="+13"/>
        <location filename="../src/ui/update_main.cpp" line="+7"/>
        <source>Not an AppImage: %1</source>
        <translation>To nie jest AppImage: %1</translation>
    </message>
    <message>
        <location line="+159"/>
        <source>The directory the integrated AppImages are stored in is currently set to:
%1</source>
        <translation>Katalogiem, w którym przechowywane są zintegrowane pliki AppImage, jest obecnie:
%1</translation>
    </message>
    <message>
        <source>The directory the integrated AppImages are stored in is currently set to: %1</source>
        <translation type="vanished">Katalogiem, w którym przechowywane są zintegrowane pliki AppImage, jest obecnie: %1</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Integrating it will move the AppImage into a predefined location, and include it in your application launcher.</source>
        <translation>Integracja tego pliku spowoduje przeniesienie AppImage do predefiniowanego katalogu i dodanie go do menu aplikacji.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>To remove or update the AppImage, please use the context menu of the application icon in your task bar or launcher.</source>
        <translation>Aby usunąć lub zaktualizować AppImage, użyj menu kontekstowego ikony aplikacji na pasku zadań lub w menu aplikacji.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The directory the integrated AppImages are stored in is currently set to:</source>
        <translation>Katalogiem, w którym przechowywane są zintegrowane pliki AppImage, jest obecnie:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>%1 has not been integrated into your system.</source>
        <translation>%1 nie został zintegrowany z twoim systemem.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Desktop Integration</source>
        <translation>Integracja z pulpitem</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Integrate and run</source>
        <translation>Zintegruj i uruchom</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run once</source>
        <translation>Uruchom jeden raz</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Cancel</source>
        <translation>Anuluj</translation>
    </message>
    <message>
        <source>Removes AppImages after desktop integration, for use by Linux distributions</source>
        <translation type="vanished">Usuwa pliki AppImage po integracji z komputerem, do użytku w systemach Linuks</translation>
    </message>
    <message>
        <source>AppImageLauncher remove</source>
        <comment>remove helper app name</comment>
        <translation type="vanished">Usuń AppImageLauncher</translation>
    </message>
    <message>
        <source>Helper to delete integrated AppImages easily, e.g., from the application launcher&apos;s context menuz</source>
        <translation type="vanished">Program do łatwego usuwania zintegrowanych plików AppImage, np. z menu kontekstowego aplikacji</translation>
    </message>
    <message>
        <location filename="../src/ui/remove_main.cpp" line="-28"/>
        <source>Helper to delete integrated AppImages easily, e.g., from the application launcher&apos;s context menu</source>
        <translation>Program do łatwego usuwania zintegrowanych plików AppImage, np. z menu kontekstowego aplikacji</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Path to AppImage</source>
        <translation>Ścieżka do AppImage</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>&lt;path&gt;</source>
        <translation>&lt;ścieżka&gt;</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>AppImage delete helper error</source>
        <translation>Błąd programu kasowania AppImage</translation>
    </message>
    <message>
        <source>AppImage remove helper error</source>
        <translation type="vanished">Błąd programu usuwania AppImage</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Not an AppImage:

%1</source>
        <translation>To nie jest AppImage:

%1</translation>
    </message>
    <message>
        <source>Refusing to work on non-integrated AppImage:

%1</source>
        <translation type="vanished">Odmowa pracy z niezintegrowanym plikiem AppImage:

%1</translation>
    </message>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">Proszę potwierdzić</translation>
    </message>
    <message>
        <source>Are you sure you want to remove this AppImage?</source>
        <translation type="vanished">Czy na pewno chcesz usunąć ten plik AppImage?</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Failed to unregister AppImage: %1</source>
        <translation>Nie udało się wyrejestrować AppImage: %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to move AppImage into trash bin directory</source>
        <translation>Nie udało się przenieść AppImage do katalogu kosza na śmieci</translation>
    </message>
    <message>
        <source>Failed to remove AppImage: %1</source>
        <translation type="vanished">Nie udało się usunąć pliku AppImage: %1</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-27"/>
        <source>AppImage with same filename has already been integrated.</source>
        <translation>AppImage z tą samą nazwą pliku został już zintegrowany.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you wish to overwrite the existing AppImage?</source>
        <translation>Czy chcesz zastąpić istniejący AppImage?</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Choosing No will run the AppImage once, and leave the system in its current state.</source>
        <translation>Wybranie opcji Nie spowoduje uruchomienie AppImage jeden raz i pozostawienie systemu bez zmian.</translation>
    </message>
    <message>
        <location line="-652"/>
        <location line="+656"/>
        <source>Warning</source>
        <translation>Ostrzeżenie</translation>
    </message>
    <message>
        <source>Failed to move AppImage to target location</source>
        <translation type="vanished">Nie udało się przenieść AppImage do lokalizacji docelowej</translation>
    </message>
    <message>
        <location line="-296"/>
        <source>Failed to register AppImage in system via libappimage</source>
        <translation>Nie udało się zarejestrować AppImage w systemie via libappimage</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to find integrated desktop file</source>
        <translation>Nie udało się znaleźć zintegrowanego pliku pulpitu</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Couldn&apos;t find integrated AppImage&apos;s desktop file</source>
        <translation>Nie można znaleźć zintegrowanego pliku pulpitu AppImage</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Failed to load desktop file:</source>
        <translation>Nie udało się załadować pliku pulpitu:</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>AppImage has invalid desktop file</source>
        <translation>AppImage plik pulpitu jest nieprawidłowy</translation>
    </message>
    <message>
        <location line="+271"/>
        <source>Failed to move AppImage to target location.
Try to copy AppImage instead?</source>
        <translation>Nie udało się przenieść AppImage do lokalizacji docelowej.
Czy zamiast tego spróbować skopiować AppImage?</translation>
    </message>
    <message>
        <location line="+181"/>
        <source>Failed to call stat() on path:

%1</source>
        <translation>Nie można wywołać stat () na ścieżce:

%1</translation>
    </message>
    <message>
        <source>Failed to copy AppImage to target location</source>
        <translation type="vanished">Nie udało się skopiować AppImage do lokalizacji docelowej</translation>
    </message>
    <message>
        <location filename="../src/ui/update_main.cpp" line="-40"/>
        <source>Updates AppImages after desktop integration, for use by Linux distributions</source>
        <translation>Aktualizuje AppImages po integracji z komputerem, do użytku przez dystrybucje Linuksa</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>AppImageLauncher update</source>
        <comment>update helper app name</comment>
        <translation>Aktualizacja AppImageLauncher</translation>
    </message>
    <message>
        <source>The AppImage hasn&apos;t been integrated before. This tool will, however, integrate the updated AppImage.</source>
        <translation type="vanished">AppImage nie był wcześniej zintegrowany. To narzędzie zintegruje zaktualizowany AppImage.</translation>
    </message>
    <message>
        <source>Do you wish to continue?</source>
        <translation type="vanished">Czy chcesz kontynuować?</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>No updates found</source>
        <translation>Nie znaleziono aktualizacji</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find updates for AppImage %1</source>
        <translation>Nie można znaleźć aktualizacji dla AppImage %1</translation>
    </message>
    <message>
        <source>Failed to check for updates. Please check the command line output for details.</source>
        <translation type="vanished">Nie udało się sprawdzić aktualizacji. Szczegóły sprawdź w wierszu poleceń.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No update information found</source>
        <translation>Nie znaleziono informacji o aktualizacji</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set update information to allow for easy updating.</source>
        <translation type="vanished">Nie można znaleźć informacji o aktualizacji w AppImage:
%1

AppImage nie obsługuje aktualizacji. Poproś autorów o ich dodanie, aby umożliwić łatwą aktualizację.</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set upupdate information to allow for easy updating.</source>
        <translation type="vanished">Nie można znaleźć informacji o aktualizacji w AppImage:
%1

AppImage nie obsługuje aktualizacji. Poproś autorów o ich dodanie, aby umożliwić łatwą aktualizację.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to embed update information to allow for easy updating.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to check for updates:

%1</source>
        <translation>Nie udało się sprawdzić aktualizacji:

%1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>An update has been found for the AppImage %1</source>
        <translation>Znaleziono aktualizację dla AppImage %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you want to perform the update?</source>
        <translation>Czy chcesz zaktualizować?</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Remove old AppImage after successful update</source>
        <translation>Usuń starą aplikację AppImage po udanej aktualizacji</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Failed to update AppImage:

%1</source>
        <translation>Nie udało się zaktualizować AppImage:

%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>File reported as updated does not exist: %1</source>
        <translation>Plik zgłoszony jako zaktualizowany nie istnieje: %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to register updated AppImage in system</source>
        <translation>Nie udało się zarejestrować zaktualizowanego AppImage w systemie</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Failed to unregister old AppImage in system</source>
        <translation>Nie udało się wyrejestrować starej aplikacji AppImage z systemu</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Failed to remove old AppImage</source>
        <translation>Nie udało się usunąć starego AppImage</translation>
    </message>
    <message>
        <location filename="../src/daemon/main.cpp" line="+101"/>
        <source>Tracks AppImages in applications directories (user&apos;s, system and other ones). Automatically integrates AppImages moved into those directories and unintegrates ones removed from them.</source>
        <translation>Śledzi pliki AppImage w katalogach aplikacji (użytkownika, systemu i innych). Automatycznie integruje pliki AppImage przeniesione do tych katalogów i wyrejestrowuje pliki z nich usunięte.</translation>
    </message>
    <message>
        <source>Search for AppImages in /Applications directories in suitable mounted filesystems</source>
        <translation type="vanished">Wyszukaj pliki AppImage w katalogach /Aplikacje w odpowiednio zamontowanych systemach plików</translation>
    </message>
    <message>
        <source>Search for AppImages in /Applications directories suitable mounted filesystems</source>
        <translation type="vanished">Wyszukaj pliki AppImage w katalogach /Aplikacje odpowiednio zamontowanych systemach plików</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Lists directories watched by this daemon and exit</source>
        <translation>Wyświetl listę katalogów śledzonych przez proces demona i zakończ działanie</translation>
    </message>
</context>
<context>
    <name>RemoveDialog</name>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">Proszę potwierdzić</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to remove this AppImage?&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Czy na pewno chcesz usunąć ten AppImage?&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/remove.ui" line="+17"/>
        <source>Delete AppImage</source>
        <translation>Wykasuj AppImage</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to delete this AppImage?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Czy na pewno chcesz wykasować ten AppImage?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Uncheck to only remove the desktop integration, but leave the file on the system.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Odznacz, aby usunąć tylko integrację pulpitu, ale pozostaw plik w systemie.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Remove AppImage file from system</source>
        <translation type="vanished">Usuń plik AppImage z systemu</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="../src/ui/settings_dialog.ui" line="+14"/>
        <source>AppImageLauncher Settings</source>
        <translation>Ustawienia AppImageLauncher</translation>
    </message>
    <message>
        <source>AppImageLauncher</source>
        <translation type="vanished">AppImageLauncher</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Launcher Dialog</source>
        <translation>Okno dialogowe uruchamiania</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ask whether to move AppImage files into the applications directory</source>
        <translation>Zapytaj, czy przenieść pliki AppImage do katalogu aplikacji</translation>
    </message>
    <message>
        <source>appimagelauncherd</source>
        <translation type="vanished">appimagelauncherd</translation>
    </message>
    <message>
        <location line="+119"/>
        <source>General settings</source>
        <translation>Ustawienia główne</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;When this box is checked, AppImageLauncher automatically starts a daemon called appimagelauncherd.&lt;/p&gt;&lt;p&gt;This daemon automatically integrates AppImages you copy into the &amp;quot;Applications directory&amp;quot; and the additional directories you configured. When the files are deleted, the daemon will clean up the integration data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Gdy to pole jest zaznaczone, AppImageLauncher automatycznie uruchamia proces demona o nazwie appimagelauncherd.&lt;/p&gt;&lt;p&gt;Ten demon automatycznie integruje skopiowane AppImages do &amp;quot;katalogu aplikacji&amp;quot; i skonfigurowanych dodatkowych katalogów. Po usunięciu plików demon wyczyści dane integracji.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Auto start auto-integration daemon</source>
        <translation>Automatycznie uruchamiaj demona integracji</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Additional directories to watch</source>
        <translation>Dodatkowe katalogi do śledzenia</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Add new directory to list</source>
        <translation>Dodaj nowy katalog do listy</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove selected directory from list</source>
        <translation>Usuń wybrany katalog z listy</translation>
    </message>
    <message>
        <source>-</source>
        <translation type="vanished">-</translation>
    </message>
    <message>
        <source>Applications directory</source>
        <translation type="vanished">Katalog aplikacji</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;When enabled, the daemon automatically integrates AppImages you copy into the &amp;quot;Applications directory&amp;quot;. When the files are deleted, the daemon will clean up the integration data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Po włączeniu procesu demona automatycznie integruje pliki AppImage, które kopiujesz do &amp;quot;katalogu aplikacji&amp;quot;. Po usunięciu plików demon wyczyści dane integracji.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="-144"/>
        <source>Applications directory path</source>
        <translation>Ścieżka do katalogu aplikacji</translation>
    </message>
    <message>
        <location line="-11"/>
        <source>Location where to store your AppImage files to ease their management</source>
        <translation>Miejsce przechowywania plików AppImage w celu ułatwienia zarządzania nimi</translation>
    </message>
    <message>
        <source>Location where to store your AppImage files to ease their management using the file manager.</source>
        <translation type="vanished">Miejsce przechowywania plików AppImage w celu ułatwienia zarządzania nimi za pomocą menedżera plików.</translation>
    </message>
    <message>
        <source>/path</source>
        <translation type="vanished">/ścieżka</translation>
    </message>
    <message>
        <source>Enable auto-integration daemon</source>
        <translation type="vanished">Włącz proces demona automatycznej integracji</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Available Features</source>
        <translation>Dostępne funkcje</translation>
    </message>
    <message>
        <location filename="../src/ui/settings_dialog.cpp" line="+40"/>
        <source>updater available for AppImages supporting AppImageUpdate</source>
        <translation>aktualizator dostępny dla plików AppImage obsługujących AppImageUpdate</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>updater unavailable</source>
        <translation>aktualizator niedostępny</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&lt;strong&gt;Note: this is an AppImageLauncher Lite build, only supports a limited set of features&lt;/strong&gt;&lt;br /&gt;Please install the full version via the provided native packages to enjoy the full AppImageLauncher experience</source>
        <translation>&lt;strong&gt;Uwaga: jest to wersja AppImageLauncher Lite, obsługująca tylko ograniczony zestaw funkcji&lt;/strong&gt;&lt;br /&gt;Zainstaluj pełną wersję korzystając z dostępnych programów instalacyjnych, aby cieszyć się pełną funkcjonalnością AppImageLauncher</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Select Applications directory</source>
        <translation>Wybierz katalog aplikacji</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Select additional directory to watch</source>
        <translation>Wybierz dodatkowy katalog do śledzenia</translation>
    </message>
</context>
</TS>
