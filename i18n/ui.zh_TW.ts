<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_TW">
<context>
    <name>FirstRunDialog</name>
    <message>
        <location filename="../src/ui/first-run.ui" line="+20"/>
        <source>First run</source>
        <translation>首次執行</translation>
    </message>
    <message>
        <location line="+55"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;Welcome to AppImageLauncher!&lt;/span&gt;&lt;/p&gt;&lt;p&gt;This little helper is designed to improve your AppImage experience on your computer.&lt;/p&gt;&lt;p&gt;It appears you have never run AppImageLauncher before. Please take a minute and configure your preferences. You can always change these later on, using the control panel.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;&lt;span style=&quot; font-size:11pt; font-weight:600;&quot;&gt;歡迎使用 AppImageLauncher！&lt;/span&gt;&lt;/p&gt;&lt;p&gt;這個工具旨在能更方便的使用 AppImage。&lt;/p&gt;&lt;p&gt;看來您以前從未執行過 AppImageLauncher 。請豁一點時間設定 AppImageLauncher。您隨後可以使用設定調整這些内容。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>Ask me whether to move new AppImages into a central location</source>
        <translation>詢問我是否要將新的 AppImage 移動到一個中間位置</translation>
    </message>
    <message>
        <location line="+21"/>
        <source>Integration target destination directory:</source>
        <translation>在目標資料夾中已匯成：</translation>
    </message>
    <message>
        <location line="+36"/>
        <source>Customize</source>
        <translation>自訂</translation>
    </message>
    <message>
        <location filename="../src/ui/first-run.cpp" line="+86"/>
        <source>Choose integration destination dir</source>
        <translation>選擇匯成後appimage的存放資料夾</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>(default)</source>
        <translation>（預設）</translation>
    </message>
</context>
<context>
    <name>IntegrationDialog</name>
    <message>
        <location filename="../src/ui/integration_dialog.ui" line="+23"/>
        <source>Desktop Integration</source>
        <translation type="unfinished">桌面匯成</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Icon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+21"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Noto Sans&apos;; font-size:10pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;%1 has not been integrated into your system.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt; &lt;br /&gt;Integrating it will move the AppImage into a predefined location, and include it in your application launcher.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;br /&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;To remove or update the AppImage, please use the context menu of the application icon in your task bar or launcher. &lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;br /&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The directory where the integrated AppImages are stored in is currently set to: %2&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="+41"/>
        <source>Integrate and run</source>
        <translation type="unfinished">匯成並執行</translation>
    </message>
    <message>
        <location line="+10"/>
        <source>Run once</source>
        <translation type="unfinished">執行一次</translation>
    </message>
</context>
<context>
    <name>QMessageBox</name>
    <message>
        <location filename="../src/ui/main.cpp" line="+385"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <source>AppImage has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?
Choosing No will run the AppImage once, and leave the AppImage in its current directory.</source>
        <translation type="vanished">AppImage 已匯成，但它不在當前集成目標資料夾。

您想要將它移至新位置嗎？
選擇否將執行一次 AppImage ，並將 AppImage 留在當前資料夾。</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>AppImage %1 has already been integrated, but it is not in the current integration destination directory.

Do you want to move it into the new destination?

Choosing No will run the AppImage once, and leave the AppImage in its current directory.

</source>
        <translation>AppImage %1 已匯成，但它不在當前已匯成的資料夾内。

您想要將它移至新位置嗎？

選擇否將執行一次 AppImage ，並將 AppImage 留在當前資料夾。

</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="+1232"/>
        <location line="+8"/>
        <source>Error</source>
        <translation>錯誤</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="+23"/>
        <source>Failed to unregister AppImage before re-integrating it</source>
        <translation>未能在重新匯成 AppImage 前取消匯成</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-485"/>
        <source>Could not parse desktop file translations:
Could not open file for reading:

%1</source>
        <translation>無法清除桌面翻譯檔案：
無法讀取此檔案：

%1</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Could not parse desktop file translations:
Invalid syntax:

%1</source>
        <translation>無法清除桌面翻譯檔案：
無此指令：

%1</translation>
    </message>
    <message>
        <location line="+409"/>
        <source>File %1 is owned by another user: %2</source>
        <translation>檔案 %1 的擁有者是：%2</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Permissions problem</source>
        <translation>權限問題</translation>
    </message>
    <message>
        <location line="+52"/>
        <source>Failed to run permissions helper, exited with return code %1</source>
        <translation>執行權限工具失敗，退出 %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Could not find suitable permissions helper, aborting</source>
        <translation>未找到合适的权限助手，正在退出</translation>
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
        <translation>错误</translation>
    </message>
    <message>
        <location filename="../src/ui/main.cpp" line="-362"/>
        <source>AppImageLauncher does not support type %1 AppImages at the moment.</source>
        <translation>AppImageLauncher 目前不支援 %1 類型的 AppImage 。</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>Could not make AppImage executable: %1</source>
        <translation>无法设置 AppImage 为可执行： %1</translation>
    </message>
    <message>
        <source>Failed to open AppImage for reading: %1</source>
        <translation type="vanished">無法開啟 AppImage 以讀取： %1</translation>
    </message>
    <message>
        <source>Failed to create temporary directory</source>
        <translation type="vanished">無法建立暫時性的資料夾</translation>
    </message>
    <message>
        <source>Failed to create temporary copy of type 1 AppImage</source>
        <translation type="vanished">無法建立 %1 類型 AppImage 的暫時性副本</translation>
    </message>
    <message>
        <source>Failed to open temporary AppImage copy for writing</source>
        <translation type="vanished">無法開啟暫時性 AppImage 副本以寫入</translation>
    </message>
    <message>
        <source>Failed to remove magic bytes from temporary AppImage copy</source>
        <translation type="vanished">無法從暫時性 AppImage 副本中移除 magic bytes</translation>
    </message>
    <message>
        <source>execv() failed: %1</source>
        <comment>error message</comment>
        <translation type="vanished">execv() 失敗： %1</translation>
    </message>
    <message>
        <source>runtime not found: no such file or directory: %1</source>
        <translation type="vanished">位址到執行期：無此檔案匯資料夾： %1</translation>
    </message>
    <message>
        <location line="+40"/>
        <source>execv() failed: %1</source>
        <translation>execv() 失敗： %1</translation>
    </message>
    <message>
        <location line="+70"/>
        <source>Usage: %1 [options] &lt;path&gt;</source>
        <translation>用法： %1 [選項] &lt;路徑&gt;</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Desktop integration helper for AppImages, for use by Linux distributions.</source>
        <translation>AppImage 桌面匯成工具，供 Linux 發行版使用。</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Options:</source>
        <translation>設定：</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display this help and exit</source>
        <translation>顯示此說明並退出</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Display version and exit</source>
        <translation>顯示版本並退出</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Arguments:</source>
        <translation>參數：</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Path to AppImage (mandatory)</source>
        <translation>AppImage 路徑（強制性）</translation>
    </message>
    <message>
        <location line="+265"/>
        <source>Unexpected result from the integration dialog.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location line="-287"/>
        <source>Failed to clean up old desktop files</source>
        <translation>無法清除舊的桌面檔案</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: error while trying to start appimagelauncherfs.service</source>
        <translation type="vanished">在 AppImageLauncherFS 中註冊 AppImage 失敗：啟動 appimagelauncherfs.service 時發生錯誤</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: failed to register AppImage path %1</source>
        <translation type="vanished">在 AppImageLauncherFSFailed 中註冊 AppImage 失敗：註冊 AppImage 路徑 %1 失敗</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not open map file</source>
        <translation type="vanished">在 AppImageLauncherFSFailed 中註冊 AppImage 失敗：無法開啟映像檔案</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: could not find virtual file for AppImage</source>
        <translation type="vanished">在 AppImageLauncherFSFailed 中註冊 AppImage 失敗：沒有 AppImage 的虛擬檔案</translation>
    </message>
    <message>
        <source>Failed to register AppImage in AppImageLauncherFS: unknown failure</source>
        <translation type="vanished">在 AppImageLauncherFSFailed 中註冊 AppImage 失敗：未知</translation>
    </message>
    <message>
        <location line="+8"/>
        <location filename="../src/ui/remove_main.cpp" line="+1"/>
        <source>Failed to clean up AppImage trash bin: %1</source>
        <translation>清除 AppImage 垃圾桶失敗： %1</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Unknown AppImageLauncher option: %1</source>
        <translation>未知 AppImageLauncher 選項： %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <location filename="../src/ui/remove_main.cpp" line="-74"/>
        <location filename="../src/ui/update_main.cpp" line="-99"/>
        <source>Error: no such file or directory: %1</source>
        <translation>錯誤：無此檔案或資料夾： %1</translation>
    </message>
    <message>
        <location line="+13"/>
        <location filename="../src/ui/update_main.cpp" line="+7"/>
        <source>Not an AppImage: %1</source>
        <translation>不是一個 AppImage： %1</translation>
    </message>
    <message>
        <location line="+162"/>
        <source>The directory the integrated AppImages are stored in is currently set to:
%1</source>
        <translation>儲存匯成的 AppImage 的資料夾現在被設為：
%1</translation>
    </message>
    <message>
        <source>The directory the integrated AppImages are stored in is currently set to: %1</source>
        <translation type="vanished">儲存匯成的 AppImage 的資料夾現在被設為： %1</translation>
    </message>
    <message>
        <source>Integrating it will move the AppImage into a predefined location, and include it in your application launcher.</source>
        <translation type="vanished">匯成它會將 AppImage 移動到預設位置，並將它包含進你的應用程式啟動器。</translation>
    </message>
    <message>
        <source>To remove or update the AppImage, please use the context menu of the application icon in your task bar or launcher.</source>
        <translation type="vanished">請使用側欄或啟動器中應用程式清單移除或更新 AppImage 。</translation>
    </message>
    <message>
        <source>The directory the integrated AppImages are stored in is currently set to:</source>
        <translation type="vanished">儲存匯成的 AppImage 的資料夾現在被設為：</translation>
    </message>
    <message>
        <source>%1 has not been integrated into your system.</source>
        <translation type="vanished">%1 尚未匯成到你的系统。</translation>
    </message>
    <message>
        <source>Desktop Integration</source>
        <translation type="vanished">桌面匯成</translation>
    </message>
    <message>
        <source>Integrate and run</source>
        <translation type="vanished">匯成並執行</translation>
    </message>
    <message>
        <source>Run once</source>
        <translation type="vanished">執行一次</translation>
    </message>
    <message>
        <source>Cancel</source>
        <translation type="vanished">取消</translation>
    </message>
    <message>
        <source>Removes AppImages after desktop integration, for use by Linux distributions</source>
        <translation type="vanished">桌面匯成後移除 AppImage ，供 Linux 發行版使用</translation>
    </message>
    <message>
        <source>AppImageLauncher remove</source>
        <comment>remove helper app name</comment>
        <translation type="vanished">AppImageLauncher 移除</translation>
    </message>
    <message>
        <location filename="../src/ui/remove_main.cpp" line="-28"/>
        <source>Helper to delete integrated AppImages easily, e.g., from the application launcher&apos;s context menu</source>
        <translation>移除匯成的 AppImage 工具，例如，從應用程式啟動器上移除</translation>
    </message>
    <message>
        <location line="+19"/>
        <source>Path to AppImage</source>
        <translation>AppImage 路徑</translation>
    </message>
    <message>
        <location line="+0"/>
        <source>&lt;path&gt;</source>
        <translation>&lt;路徑&gt;</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>AppImage delete helper error</source>
        <translation>移除 AppImage 錯誤</translation>
    </message>
    <message>
        <source>AppImage remove helper error</source>
        <translation type="vanished">移除 AppImage 錯誤</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Not an AppImage:

%1</source>
        <translation>不是 AppImage:

%1</translation>
    </message>
    <message>
        <source>Refusing to work on non-integrated AppImage:

%1</source>
        <translation type="vanished">拒絕在未匯成的 AppImage 上工作:

%1</translation>
    </message>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">請確定</translation>
    </message>
    <message>
        <source>Are you sure you want to remove this AppImage?</source>
        <translation type="vanished">您確定要移除此 AppImage 嗎？</translation>
    </message>
    <message>
        <location line="+38"/>
        <source>Failed to unregister AppImage: %1</source>
        <translation>未能註銷 AppImage： %1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to move AppImage into trash bin directory</source>
        <translation>將 AppImage 移入垃圾桶時發生錯誤</translation>
    </message>
    <message>
        <source>Failed to remove AppImage: %1</source>
        <translation type="vanished">移除 AppImage 時發生錯誤： %1</translation>
    </message>
    <message>
        <location filename="../src/shared/shared.cpp" line="-27"/>
        <source>AppImage with same filename has already been integrated.</source>
        <translation>具有相同檔案名稱的 AppImage 已匯成。</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you wish to overwrite the existing AppImage?</source>
        <translation>你想要覆寫已存在的 AppImage 吗？</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Choosing No will run the AppImage once, and leave the system in its current state.</source>
        <translation>是否再執行一次 AppImage ，並儲存當前系統狀態。</translation>
    </message>
    <message>
        <location line="-652"/>
        <location line="+656"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <source>Failed to move AppImage to target location</source>
        <translation type="vanished">無法將 AppImage 移動到目標位置</translation>
    </message>
    <message>
        <location line="-296"/>
        <source>Failed to register AppImage in system via libappimage</source>
        <translation>libappimage 無法在系统中註冊 AppImage</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to find integrated desktop file</source>
        <translation>未找到已匯成桌面檔案</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Couldn&apos;t find integrated AppImage&apos;s desktop file</source>
        <translation>無法找到已匯成 AppImage 的桌面檔案</translation>
    </message>
    <message>
        <location line="+20"/>
        <source>Failed to load desktop file:</source>
        <translation>重新載入桌面檔案失敗：</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>AppImage has invalid desktop file</source>
        <translation>AppImage 有無效桌面檔案</translation>
    </message>
    <message>
        <location line="+271"/>
        <source>Failed to move AppImage to target location.
Try to copy AppImage instead?</source>
        <translation>無法移動 AppImage 到目標位置。
要嘗試複製 AppImage 嗎？</translation>
    </message>
    <message>
        <location line="+181"/>
        <source>Failed to call stat() on path:

%1</source>
        <translation>在此路徑上叫出 stat() 失敗:

%1</translation>
    </message>
    <message>
        <source>Failed to copy AppImage to target location</source>
        <translation type="vanished">無法複製 AppImage 到目標位置</translation>
    </message>
    <message>
        <location filename="../src/ui/update_main.cpp" line="-40"/>
        <source>Updates AppImages after desktop integration, for use by Linux distributions</source>
        <translation>桌面匯成後更新 AppImage ，供 Linux 使用</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>AppImageLauncher update</source>
        <comment>update helper app name</comment>
        <translation>AppImageLauncher 更新</translation>
    </message>
    <message>
        <source>The AppImage hasn&apos;t been integrated before. This tool will, however, integrate the updated AppImage.</source>
        <translation type="vanished">AppImage 未匯成。然而此工具將匯成更新後的 AppImage 。</translation>
    </message>
    <message>
        <source>Do you wish to continue?</source>
        <translation type="vanished">你想繼續嗎？</translation>
    </message>
    <message>
        <location line="+67"/>
        <source>No updates found</source>
        <translation>未找到更新</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find updates for AppImage %1</source>
        <translation>無法找到 AppImage %1 的更新</translation>
    </message>
    <message>
        <source>Failed to check for updates. Please check the command line output for details.</source>
        <translation type="vanished">檢查更新失敗。請檢查命令行輸出獲取詳細資訊。</translation>
    </message>
    <message>
        <location line="+7"/>
        <source>No update information found</source>
        <translation>未找到更新資訊</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set update information to allow for easy updating.</source>
        <translation type="vanished">AppImage 中未找到更新資訊：
%1

此 AppImage 不支援更新。請要求作者更新資訊以便更新。</translation>
    </message>
    <message>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to set upupdate information to allow for easy updating.</source>
        <translation type="vanished">無法在 AppImage 中找到更新資訊：
%1

此 AppImage 不支持更新。请向作者请求设置更新信息以允许轻松更新。</translation>
    </message>
    <message>
        <location line="+1"/>
        <source>Could not find update information in AppImage:
%1

The AppImage doesn&apos;t support updating. Please ask the authors to embed update information to allow for easy updating.</source>
        <translation>無法取得Appimage内的升级資訊。
%1

此Appimage不支援升级。請聯絡作者在Appimage内添加升级資訊以便升級。</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>Failed to check for updates:

%1</source>
        <translation>檢查更新失敗：

%1</translation>
    </message>
    <message>
        <location line="+12"/>
        <source>An update has been found for the AppImage %1</source>
        <translation>已找到 AppImage %1 的一個更新</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>Do you want to perform the update?</source>
        <translation>你想要下載並安裝更新嗎？</translation>
    </message>
    <message>
        <location line="+4"/>
        <source>Remove old AppImage after successful update</source>
        <translation>更新成功後移除舊 AppImage</translation>
    </message>
    <message>
        <location line="+24"/>
        <source>Failed to update AppImage:

%1</source>
        <translation>更新 AppImage 失敗：

%1</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>File reported as updated does not exist: %1</source>
        <translation>報告說已更新的檔案不存在： %1</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Failed to register updated AppImage in system</source>
        <translation>無法在系統中註冊已更新 AppImage</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>Failed to unregister old AppImage in system</source>
        <translation>無法在系统中註銷舊 AppImage</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>Failed to remove old AppImage</source>
        <translation>移除舊 AppImage 失敗</translation>
    </message>
    <message>
        <location filename="../src/daemon/main.cpp" line="+101"/>
        <source>Tracks AppImages in applications directories (user&apos;s, system and other ones). Automatically integrates AppImages moved into those directories and unintegrates ones removed from them.</source>
        <translation>在應用程式資料夾（使用者，系統和其他資料夾）中追蹤 AppImage。 自動匯成移動到這些資料夾中的 AppImage，並取消匯成從其中刪除的 AppImage。</translation>
    </message>
    <message>
        <location line="+8"/>
        <source>Lists directories watched by this daemon and exit</source>
        <translation>列出此守護行程監視的資料夾並退出</translation>
    </message>
</context>
<context>
    <name>RemoveDialog</name>
    <message>
        <source>Please confirm</source>
        <translation type="vanished">請確定</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to remove this AppImage?&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;您確定要移除此 AppImage 嗎？&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location filename="../src/ui/remove.ui" line="+17"/>
        <source>Delete AppImage</source>
        <translation>刪除 AppImage</translation>
    </message>
    <message>
        <location line="+14"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Are you sure you want to delete this AppImage?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;您確定要刪除此 AppImage 嗎?&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+22"/>
        <source>%1</source>
        <translation>%1</translation>
    </message>
    <message>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;Uncheck to only remove the desktop integration, but leave the file on the system.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="vanished">&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;取消句選則只刪除桌面匯成，但將檔案保留在系統上。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <source>Remove AppImage file from system</source>
        <translation type="vanished">從電腦中移除 AppImage 檔案</translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="../src/ui/settings_dialog.ui" line="+14"/>
        <source>AppImageLauncher Settings</source>
        <translation>AppImageLauncher 設定</translation>
    </message>
    <message>
        <location line="+16"/>
        <source>Launcher Dialog</source>
        <translation>啟動器</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>Ask whether to move AppImage files into the applications directory</source>
        <translation>是否將 AppImage 檔案移動到應用程式的資料夾中</translation>
    </message>
    <message>
        <location line="+119"/>
        <source>General settings</source>
        <translation>一般</translation>
    </message>
    <message>
        <location line="+6"/>
        <source>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;When this box is checked, AppImageLauncher automatically starts a daemon called appimagelauncherd.&lt;/p&gt;&lt;p&gt;This daemon automatically integrates AppImages you copy into the &amp;quot;Applications directory&amp;quot; and the additional directories you configured. When the files are deleted, the daemon will clean up the integration data.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;html&gt;&lt;head/&gt;&lt;body&gt;&lt;p&gt;選中此框後，AppImageLauncher 會自動啟動 appimagelauncherd 的守護行程。&lt;/p&gt;&lt;p&gt;此守護行程自動匯成複製到 &amp;quot;Applications 資料夾&amp;quot;及其它您設定的資料夾中的 AppImage。刪除檔案後，守護行程將清除匯成的檔案。&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
    <message>
        <location line="+3"/>
        <source>Auto start auto-integration daemon</source>
        <translation>自動啟動自動匯成守護行程</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Additional directories to watch</source>
        <translation>要追蹤的其他資料夾</translation>
    </message>
    <message>
        <location line="+11"/>
        <source>Add new directory to list</source>
        <translation>向清單中添加新的資料夾</translation>
    </message>
    <message>
        <location line="+13"/>
        <source>Remove selected directory from list</source>
        <translation>從清單刪除指定的資料夾</translation>
    </message>
    <message>
        <location line="-144"/>
        <source>Applications directory path</source>
        <translation>應用程式資料夾路徑</translation>
    </message>
    <message>
        <location line="-11"/>
        <source>Location where to store your AppImage files to ease their management</source>
        <translation>儲存 AppImage 資料以簡化其管理的位置</translation>
    </message>
    <message>
        <location line="+49"/>
        <source>Available Features</source>
        <translation>可用功能</translation>
    </message>
    <message>
        <location filename="../src/ui/settings_dialog.cpp" line="+40"/>
        <source>updater available for AppImages supporting AppImageUpdate</source>
        <translation>可用於支援 AppImageUpdate 的 AppImage 的更新程式</translation>
    </message>
    <message>
        <location line="+2"/>
        <source>updater unavailable</source>
        <translation>無法安裝更新</translation>
    </message>
    <message>
        <location line="+5"/>
        <source>&lt;strong&gt;Note: this is an AppImageLauncher Lite build, only supports a limited set of features&lt;/strong&gt;&lt;br /&gt;Please install the full version via the provided native packages to enjoy the full AppImageLauncher experience</source>
        <translation>&lt;strong&gt;注意：這是一個 AppImageLauncher Lite 的版本，僅支援有限的功能&lt;/strong&gt;&lt;br /&gt;請從官網安裝完整版，以享受完整的 AppImageLauncher 功能</translation>
    </message>
    <message>
        <location line="+137"/>
        <source>Select Applications directory</source>
        <translation>請選擇應用程式資料夾</translation>
    </message>
    <message>
        <location line="+17"/>
        <source>Select additional directory to watch</source>
        <translation>選擇追蹤的其他資料夾</translation>
    </message>
</context>
</TS>
