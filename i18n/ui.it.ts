<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it">
<context>
    <name>FirstRunDialog</name>
    <message>
        <location filename="../src/ui/first-run.ui" line="+20"/>
        <source>First run</source>
        <translation>Primo avvio</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Welcome to AppImageLauncher!&lt;/span&gt;&lt;/p&gt;&lt;p&gt;This little helper is designed to improve your AppImage experience on your computer.&lt;/p&gt;&lt;p&gt;It appears you have never run AppImageLauncher before. Please take a minute and configure your preferences. You can always change these later on, using the control panel.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Benvenuto su AppImageLauncher!&lt;/span&gt;&lt;/p&gt;&lt;p&gt;Questo piccolo assistente è progettato per migliorare la tua esperienza con le AppImage sul tuo computer.&lt;/p&gt;&lt;p&gt;Sembra che tu non abbia mai eseguito AppImageLauncher prima. Ti preghiamo di dedicare un minuto e configurare le tue preferenze. Puoi sempre modificarle in seguito, utilizzando il pannello di controllo.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Ask me whether to move new AppImages into a central location</source>
        <translation>Chiedimi se spostare le nuove AppImage in una posizione centrale</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Integration target destination directory:</source>
        <translation>Directory di destinazione per l&apos;integrazione:</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Customize</source>
        <translation>Personalizza</translation>
    </message>
    <message>
        <location filename="../src/ui/first-run.cpp" line="+86"/>
        <source>Choose integration destination dir</source>
        <translation>Scegliere la directory di destinazione per l&apos;integrazione</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>(default)</source>
        <translation>(predefinito)</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <location filename="../src/ui/main.cpp" line="+380"/>
        <source>Warning</source>
        <translation>Avviso</translation>
    </message>
    <message>
        <source>AppImage has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?
Choosing No will run the AppImage once, and leave the AppImage in its current directory.</source>
        <translation type="vanished">AppImage è già stato integrato, ma non si trova nella directory di destinazione dell&apos;integrazione corrente.

Vuoi spostarlo nella nuova destinazione?
Scegliendo No verrà eseguito l&apos;AppImage una volta e lascerà l&apos;AppImage nella sua directory corrente.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AppImage %1 has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?

Choosing No will run the AppImage once, and leave the AppImage in its current directory.

</source>
        <translation>L&apos;AppImage %1 è già stata integrata, ma non si trova nella directory di integrazione corrente.

Vuoi spostarla nella nuova destinazione?

Scegliendo No verrà eseguita l&apos;AppImage una volta e verrà lasciata l&apos;AppImage nella sua directory corrente.

</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="+1232"/>
        <location line="+8"/>
        <source>Error</source>
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="+23"/>
        <source>Failed to unregister AppImage before re-integrating it</source>
        <translation>Impossibile annullare la registrazione dell&apos;AppImage per reintegrarla</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-485"/>
        <source>Could not parse desktop file translations:
Could not open file for reading:

%1</source>
        <translation>Impossibile analizzare le traduzioni dei file desktop:
Impossibile aprire il file per la lettura:

%1</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Could not parse desktop file translations:
Invalid syntax:

%1</source>
        <translation>Impossibile analizzare le traduzioni dei file desktop:
Sintassi non valida:

%1</translation>
    </message>
    <message>
        <location line="+409"/>
        <source>File %1 is owned by another user: %2</source>
        <translation>Il file %1 è di proprietà di un altro utente: %2</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Permissions problem</source>
        <translation>Problema con i permessi</translation>
    </message>
    <message>
        <source>File %1 is owned by another user: %2

Relaunch with their permissions?</source>
        <translation type="vanished">Il file %1 è di proprietà di un altro utente: %2

Rilanciare con i loro permessi?</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Failed to run permissions helper, exited with return code %1</source>
        <translation>Impossibile eseguire l&apos;assistente dei permessi, terminato con il codice di ritorno %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Could not find suitable permissions helper, aborting</source>
        <translation>Impossibile trovare un assistente dei permessi adeguato, interrotto</translation>
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
        <translation>Errore</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="-358"/>
        <source>AppImageLauncher does not support type %1 AppImages at the moment.</source>
        <translation>AppImageLauncher non supporta AppImage di tipo %1 al momento.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Could not make AppImage executable: %1</source>
        <translation>Impossibile rendere eseguibile l&apos;AppImage: %1</translation>
    </message>
    <message>
        <source>Failed to open AppImage for reading: %1</source>
        <translation type="vanished">Impossibile aprire AppImage per la lettura: %1</translation>
    </message>
    <message>
        <source>Failed to create temporary directory</source>
        <translation type="vanished">Impossibile creare la directory temporanea</translation>
    </message>
    <message>
        <source>Failed to create temporary copy of type 1 AppImage</source>
        <translation type="vanished">Impossibile creare una copia temporanea di AppImage di tipo 1</translation>
    </message>
    <message>
        <source>Failed to open temporary AppImage copy for writing</source>
        <translation type="vanished">Impossibile aprire la copia temporanea di AppImage per la scrittura</translation>
    </message>
    <message>
        <source>Failed to remove magic bytes from temporary AppImage copy</source>
        <translation type="vanished">Impossibile rimuovere i byte magici dalla copia temporanea di AppImage</translation>
    </message>
    <message>
        <source>execv() failed: %1</source>
        <comment>error message</comment>
        <translation type="vanished">execv() fallito: %1</translation>
    </message>
    <message>
        <source>runtime not found: no such file or directory: %1</source>
        <translation type="vanished">runtime non trovato: nessun file o directory:% 1</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>execv() failed: %1</source>
        <translation>execv() fallito: %1</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Usage: %1 [options] &lt;path&gt;</source>
        <translation>Uso: %1 [opzioni] &lt;percorso&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Desktop integration helper for AppImages, for use by Linux distributions.</source>
        <translation>Assistente di integrazione desktop per AppImage, per l&apos;utilizzo da parte di distribuzioni Linux.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Options:</source>
        <translation>Opzioni:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display this help and exit</source>
        <translation>Visualizza questa guida ed esce</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display version and exit</source>
        <translation>Visualizza la versione ed esce</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Arguments:</source>
        <translation>Argomenti:</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Path to AppImage (mandatory)</source>
        <translation>Percorso dell&apos;AppImage (obbligatorio)</translation>
    </message>
    <message>
        <location line="-21"/>
        <source>Failed to clean up old desktop files</source>
        <translation>Impossibile ripulire i vecchi file desktop</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: error while trying to start appimagelauncherfs.service</source>
        <translation type="vanished">Impossibile registrare AppImage in AppImageLauncherFS: errore durante il tentativo di avviare appimagelauncherfs.service</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: failed to register AppImage path %1</source>
        <translation type="vanished">Impossibile registrare AppImage in AppImageLauncherFS: impossibile registrare il percorso AppImage %1</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not open map file</source>
        <translation type="vanished">Impossibile registrare AppImage in AppImageLauncherFS: impossibile aprire la mappa del file</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not find virtual file for AppImage</source>
        <translation type="vanished">Impossibile registrare AppImage in AppImageLauncherFS: impossibile trovare il file virtuale per AppImage</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: unknown failure</source>
        <translation type="vanished">Impossibile registrare AppImage in AppImageLauncherFS: errore sconosciuto</translation>
    </message>
    <message>
        <location line="+8"/>
        <location filename="../src/ui/remove_main.cpp" line="+1"/>
        <source>Failed to clean up AppImage trash bin: %1</source>
        <translation>Impossibile ripulire il cestino delle AppImage: %1</translation>
    </message>
    <message>
        <location line="+48"/>
        <source>Unknown AppImageLauncher option: %1</source>
        <translation>Opzione AppImageLauncher sconosciuta: %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/ui/remove_main.cpp" line="-74"/>
        <location filename="../src/ui/update_main.cpp" line="-99"/>
        <source>Error: no such file or directory: %1</source>
        <translation>Errore: file o directory non esistente: %1</translation>
    </message>
    <message>
        <location line="+13"/>
        <location filename="../src/ui/update_main.cpp" line="+7"/>
        <source>Not an AppImage: %1</source>
        <translation>Non un&apos;AppImage: %1</translation>
    </message>
    <message>
        <location line="+159"/>
        <source>The directory the integrated AppImages are stored in is currently set to:
%1</source>
        <translation>La directory in cui sono archiviate le AppImage integrate è attualmente impostata su:
%1</translation>
    </message>
    <message>
        <source>The directory the integrated AppImages are stored in is currently set to: %1</source>
        <translation type="vanished">La directory in cui sono archiviati gli AppImages integrati è attualmente impostata: %1</translation>
    </message>
    <message>
        <location line="+28"/>
        <source>Integrating it will move the AppImage into a predefined location, and include it in your application launcher.</source>
        <translation>Integrandola, l&apos;AppImage verrà spostata in una posizione predefinita e verrà inclusa nel launcher delle applicazioni.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>To remove or update the AppImage, please use the context menu of the application icon in your task bar or launcher.</source>
        <translation>Per rimuovere o aggiornare un&apos;AppImage, utilizzare il menu di scelta rapida sull&apos;icona dell&apos;applicazione nella barra delle applicazioni o nel launcher delle applicazioni.</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>The directory the integrated AppImages are stored in is currently set to:</source>
        <translation>La directory in cui sono archiviate le AppImage integrate è attualmente impostata su:</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>%1 has not been integrated into your system.</source>
        <translation>%1 non è stata integrata nel tuo sistema.</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Desktop Integration</source>
        <translation>Integrazione Desktop</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Integrate and run</source>
        <translation>Integra e avvia</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Run once</source>
        <translation>Avvia una volta</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Cancel</source>
        <translation>Annulla</translation>
    </message>
    <message>
        <source>Removes AppImages after desktop integration, for use by Linux distributions</source>
        <translation type="vanished">Rimuove AppImages dopo l&apos;integrazione desktop, per l&apos;utilizzo da parte delle distribuzioni Linux</translation>
    </message>
    <message>
        <source>AppImageLauncher remove</source>
        <comment>remove helper app name</comment>
        <translation type="vanished">Rimuovi AppImageLauncher</translation>
    </message>
    <message>
        <location filename="../src/ui/remove_main.cpp" line="-28"/>
        <source>Helper to delete integrated AppImages easily, e.g., from the application launcher&apos;s context menu</source>
        <translation>Aiuta ad eliminare facilmente le AppImage integrate, ad es. dal menu contestuale del launcher delle applicazioni</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Path to AppImage</source>
        <translation>Percorso dell&apos;AppImage</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>&lt;path&gt;</source>
        <translation>&lt;percorso&gt;</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>AppImage delete helper error</source>
        <translation>Errore assistente di rimozione AppImage</translation>
    </message>
    <message>
        <source>AppImage remove helper error</source>
        <translation type="vanished">AppImage rimuove l&apos;errore dell&apos;helper</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Not an AppImage:

%1</source>
        <translation>Non è un&apos;AppImage:

%1</translation>
    </message>
    <message>
        <source>Refusing to work on non-integrated AppImage:

%1</source>
        <translation type="vanished">Rifiuto di lavorare su un AppImage non integrato:

%1</translation>
    </message>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">Si prega di confermare</translation>
    </message>
    <message>
        <source>Are you sure you want to remove this AppImage?</source>
        <translation type="vanished">Sei sicuro di voler rimuovere questa AppImage?</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Failed to unregister AppImage: %1</source>
        <translation>Impossibile annullare la registrazione dell&apos;AppImage: %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to move AppImage into trash bin directory</source>
        <translation>Impossibile spostare AppImage nel cestino</translation>
    </message>
    <message>
        <source>Failed to remove AppImage: %1</source>
        <translation type="vanished">Impossibile rimuovere AppImage: %1</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-27"/>
        <source>AppImage with same filename has already been integrated.</source>
        <translation>Un&apos;AppImage con lo stesso nome è già stata integrata.</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you wish to overwrite the existing AppImage?</source>
        <translation>Desideri sovrascrivere l&apos;AppImage esistente?</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Choosing No will run the AppImage once, and leave the system in its current state.</source>
        <translation>Scegliendo No, l&apos;AppImage verrà eseguita una volta e il sistema sarà lasciato nel suo stato corrente.</translation>
    </message>
    <message>
        <location line="-652"/>
        <location line="+656"/>
        <source>Warning</source>
        <translation>Avviso</translation>
    </message>
    <message>
        <source>Failed to move AppImage to target location</source>
        <translation type="vanished">Impossibile spostare AppImage nella posizione di destinazione</translation>
    </message>
    <message>
        <location line="-296"/>
        <source>Failed to register AppImage in system via libappimage</source>
        <translation>Impossibile registrare AppImage nel sistema tramite libappimage</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to find integrated desktop file</source>
        <translation>Impossibile trovare il file desktop integrato</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Couldn&apos;t find integrated AppImage&apos;s desktop file</source>
        <translation>Impossibile trovare il file desktop dell&apos;AppImage integrata</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Failed to load desktop file:</source>
        <translation>Caricamento del file desktop non riuscito:</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>AppImage has invalid desktop file</source>
        <translation>L&apos;AppImage ha un file desktop non valido</translation>
    </message>
    <message>
        <location line="+271"/>
        <source>Failed to move AppImage to target location.
Try to copy AppImage instead?</source>
        <translation>Impossibile spostare l&apos;AppImage nella posizione di destinazione.
Provare invece a copiare l&apos;AppImage?</translation>
    </message>
    <message>
        <location line="+181"/>
        <source>Failed to call stat() on path:

%1</source>
        <translation>Impossibile invocare stat() sul percorso:

%1</translation>
    </message>
    <message>
        <source>Failed to copy AppImage to target location</source>
        <translation type="vanished">Impossibile copiare AppImage nella posizione di destinazione</translation>
    </message>
    <message>
        <location filename="../src/ui/update_main.cpp" line="-40"/>
        <source>Updates AppImages after desktop integration, for use by Linux distributions</source>
        <translation>Aggiorna le AppImage dopo l&apos;integrazione desktop, per l&apos;utilizzo da parte delle distribuzioni Linux</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>AppImageLauncher update</source>
        <comment>update helper app name</comment>
        <translation>Aggiornare AppImageLauncher</translation>
    </message>
    <message>
        <source>The AppImage hasn&apos;t been integrated before. This tool will, however, integrate the updated AppImage.</source>
        <translation type="vanished">AppImage non è stato integrato prima. Questo strumento, tuttavia, integrerà l&apos;AppImage aggiornato.</translation>
    </message>
    <message>
        <source>Do you wish to continue?</source>
        <translation type="vanished">Vuoi continuare?</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>No updates found</source>
        <translation>Nessun aggiornamento trovato</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find updates for AppImage %1</source>
        <translation>Impossibile trovare aggiornamenti per l&apos;AppImage %1</translation>
    </message>
    <message>
        <source>Failed to check for updates. Please check the command line output for details.</source>
        <translation type="vanished">Impossibile controllare gli aggiornamenti. Controllare l&apos;output della riga di comando per i dettagli.</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No update information found</source>
        <translation>Nessuna informazione di aggiornamento trovata</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set update information to allow for easy updating.</source>
        <translation type="vanished">Impossibile trovare le informazioni di aggiornamento in AppImage:
%1

AppImage non supporta l&apos;aggiornamento. Si prega di chiedere agli autori di impostare le informazioni di aggiornamento per consentire un facile aggiornamento.</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set upupdate information to allow for easy updating.</source>
        <translation type="vanished">Impossibile trovare le informazioni di aggiornamento in AppImage:
%1

AppImage non supporta l&apos;aggiornamento. Si prega di chiedere agli autori di impostare informazioni aggiornate per consentire un facile aggiornamento.</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to embed update information to allow for easy updating.</source>
        <translation>Impossibile trovare le informazioni di aggiornamento nell&apos;AppImage:
%1

L&apos;AppImage non supporta gli aggiornamenti. Per favore chiedi agli autori di includere le informazioni di aggiornamento per facilitare gli aggiornamenti.</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to check for updates:

%1</source>
        <translation>Impossibile controllare gli aggiornamenti:

%1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>An update has been found for the AppImage %1</source>
        <translation>È stato trovato un aggiornamento per l&apos;AppImage %1</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you want to perform the update?</source>
        <translation>Vuoi eseguire l&apos;aggiornamento?</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Remove old AppImage after successful update</source>
        <translation>Rimuovere la vecchia AppImage dopo un aggiornamento riuscito</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Failed to update AppImage:

%1</source>
        <translation>Impossibile aggiornare l&apos;AppImage:

%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>File reported as updated does not exist: %1</source>
        <translation>Il file segnalato come aggiornato non esiste: %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to register updated AppImage in system</source>
        <translation>Impossibile registrare l&apos;AppImage aggiornata nel sistema</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Failed to unregister old AppImage in system</source>
        <translation>Impossibile annullare la registrazione della vecchia AppImage dal sistema</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Failed to remove old AppImage</source>
        <translation>Impossibile rimuovere la vecchia AppImage</translation>
    </message>
    <message>
        <location filename="../src/daemon/main.cpp" line="+101"/>
        <source>Tracks AppImages in applications directories (user&apos;s, system and other ones). Automatically integrates AppImages moved into those directories and unintegrates ones removed from them.</source>
        <translation>Tiene traccia delle AppImage nelle directory delle applicazioni (dell&apos;utente, di sistema e altre). Integra le AppImage spostate in queste directory e deintegra quelle rimosse automaticamente.</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Lists directories watched by this daemon and exit</source>
        <translation>Elenca le directory monitorate da questo demone ed esce</translation>
    </message>
</context>
<context>
    <name>RemoveDialog</name>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">Si prega di confermare</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to remove this AppImage?&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sei sicuro di voler rimuovere questa AppImage?&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/remove.ui" line="+17"/>
        <source>Delete AppImage</source>
        <translation>Elimina AppImage</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to delete this AppImage?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Sei sicuro di voler eliminare questa AppImage?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Uncheck to only remove the desktop integration, but leave the file on the system.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Deseleziona per rimuovere solo l&apos;integrazione desktop, ma lascia il file sul sistema.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Remove AppImage file from system</source>
        <translation type="vanished">Rimuovere il file AppImage dal sistema</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="../src/ui/settings_dialog.ui" line="+14"/>
        <source>AppImageLauncher Settings</source>
        <translation>Impostazioni AppImageLauncher</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Launcher Dialog</source>
        <translation>Finestra di dialogo del launcher</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ask whether to move AppImage files into the applications directory</source>
        <translation>Chiedere se spostare i file AppImage nella directory delle applicazioni</translation>
    </message>
    <message>
        <location line="+119"/>
        <source>General settings</source>
        <translation>Impostazioni generali</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;When this box is checked, AppImageLauncher automatically starts a daemon called appimagelauncherd.&lt;/p&gt;&lt;p&gt;This daemon automatically integrates AppImages you copy into the &amp;quot;Applications directory&amp;quot; and the additional directories you configured. When the files are deleted, the daemon will clean up the integration data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Quando questa checkbox è spuntata, AppImageLauncher avvia automaticamente un demone chiamato appimagelauncherd.&lt;/p&gt;&lt;p&gt;Questo demone integra automaticamente le AppImage che vengono copiate nella &quot;directory delle applicazioni&quot; e nelle directory aggiuntive configurate. Quando i file vengono eliminati, il demone pulisce automaticamente i dati d&apos;integrazione.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Auto start auto-integration daemon</source>
        <translation>Avviare il demone di auto-integrazione automaticamente</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Additional directories to watch</source>
        <translation>Directory aggiuntive da monitorare</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Add new directory to list</source>
        <translation>Aggiungi nuova directory all&apos;elenco</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove selected directory from list</source>
        <translation>Rimuovi directory selezionata dall&apos;elenco</translation>
    </message>
    <message>
        <source>Applications directory</source>
        <translation type="vanished">Directory delle applicazioni</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;When enabled, the daemon automatically integrates AppImages you copy into the &amp;quot;Applications directory&amp;quot;. When the files are deleted, the daemon will clean up the integration data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Se abilitato, il demone integra automaticamente le AppImages copiate dentro nel &amp;quot;Directory delle applicazioni&amp;quot;. Quando i file vengono eliminati, il demone pulirà i dati di integrazione.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="-144"/>
        <source>Applications directory path</source>
        <translation>Percorso della directory delle applicazioni</translation>
    </message>
    <message>
        <location line="-11"/>
        <source>Location where to store your AppImage files to ease their management</source>
        <translation>Posizione in cui archiviare i file AppImage per facilitarne la gestione</translation>
    </message>
    <message>
        <source>Enable auto-integration daemon</source>
        <translation type="vanished">Abilita il demone di integrazione automatica</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Available Features</source>
        <translation>Funzionalità disponibili</translation>
    </message>
    <message>
        <location filename="../src/ui/settings_dialog.cpp" line="+40"/>
        <source>updater available for AppImages supporting AppImageUpdate</source>
        <translation>aggiornamenti disponibili per le AppImage che supportano AppImageUpdate</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>updater unavailable</source>
        <translation>aggiornamenti non disponibili</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&lt;strong&gt;Note: this is an AppImageLauncher Lite build, only supports a limited set of features&lt;/strong&gt;&lt;br /&gt;Please install the full version via the provided native packages to enjoy the full AppImageLauncher experience</source>
        <translation>&lt;strong&gt;Nota: questa è una build di AppImageLauncher Lite, supporta solo un set limitato di funzionalità&lt;/strong&gt;&lt;br /&gt;Installa la versione completa tramite i pacchetti nativi forniti per usufruire dell&apos;esperienza AppImageLauncher completa</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Select Applications directory</source>
        <translation>Seleziona la directory Applicazioni</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Select additional directory to watch</source>
        <translation>Seleziona directory aggiuntive da monitorare</translation>
    </message>
</context>
</TS>
