#TODO add md5 zip check
#TODO tighten vars scope
#TODO add UAC Publisher (verisign)
#TODO add comments
#TODO try to do not hardcode addSectionContent macro inside addFiletypeAssociationsComponentContent (but add only to
#those really checked
#TODO cleanup all garbage files when quit running bzr2 or when quit from uninstaller (also in silent mode)

#TODO add "isPortable" checkbox/flag that when checked don't touch the registry but add ".portable" file to ${DIR_SETTINGS_USER} with version string
#if the invoked installer.exe dir have ".portable" & bzrplayer.exe files then pre-set its checkbox by default (and put "wasPortable" flag to true)
#in "isPortable" mode $INSTDIR will pre-set by default to the same of the installer.exe if "wasPortable" is true
#add a command line param for forcing "isPortable" true

!system "./filetype_associations_component_util_generator.sh" = 0

Section
!tempfile stdoutfile
!system 'printf "%x" $(date +%s) > "${stdoutfile}"' = 0
!define /file SETUP_LATEST_VERSION "${stdoutfile}"
!delfile "${stdoutfile}"
SectionEnd

SetCompressor /SOLID /FINAL lzma
CRCCheck force

!define SETUP_FILENAME "bzr2_setup.exe"
!define SETUP_FILENAME_LATEST_VERSION "${SETUP_FILENAME}_latest"
!define URL_REPO "https://raw.githubusercontent.com/aargirakis/BZRPlayer"
!define URL_SETUP_MAIN "${URL_REPO}/main/src/inst/nsis"
!define URL_SETUP_LATEST_VERSION "${URL_SETUP_MAIN}/${SETUP_FILENAME_LATEST_VERSION}"
!define URL_SETUP_EXE "${URL_SETUP_MAIN}/${SETUP_FILENAME}"
!define LICENSE_FILE "../../../LICENSE"
!define ICON_FILE "../../app/resources/icon.ico"
!define PUBLISHER "Blazer Studios"
!define NAME_SHORT "BZR2"
!define NAME_UNVERSIONED "BZR Player"
!define NAME "${NAME_UNVERSIONED} 2"
!define EXE_FILENAME "BZRPlayer.exe"
!define VERSION_FILENAME "bzr2_version_latest"
!define RELEASE_ARCHIVE_FILENAME_UNVERSIONED "BZR-Player-"
!define RELEASE_ARCHIVE_FILE_PATH "$releaseArchiveFileDir\$releaseArchiveFilename"
!define URL_MAIN "http://bzrplayer.blazer.nu"
!define URL_LATEST_VERSION "${URL_MAIN}/latest-version.php"
!define URL_CHANGELOG "${URL_MAIN}/versions_json.php"
!define URL_RELEASE_ARCHIVE "${URL_MAIN}/getFile.php?id=$version"
!define URL_RELEASE_ARCHIVE_FALLBACK "https://github.com/aargirakis/BZRPlayer/releases/download/$version"
!define CHANGELOG_FILE_JSON "$TEMP\changelog.json"
!define CHANGELOG_FILE_RTF "$TEMP\changelog.rtf"
!define DESCRIPTION "Audio player supporting a wide array of multi-platform exotic file formats"
!define CAPTION "$title Setup"
!define CAPTION_VERSIONED "${CAPTION} (v${SETUP_LATEST_VERSION})"
!define DIR_SETTINGS_USER "$INSTDIR\user"

var title
var version
var i
var letter
var mruListContent
var localReleaseArchivePath
var releaseArchiveFileDir

!include FileFunc.nsh
!include MUI2.nsh
!include nsArray.nsh
!include nsProcess.nsh
!include Registry.nsh
!include WinVer.nsh
!include WordFunc.nsh
!include include\FiletypeAssociationsComponentUtil.nsh
!include include\FiletypeAssociationsUtil.nsh

!macro DetectRunningBzr2
  loop:
  ${nsProcess::FindProcess} "${EXE_FILENAME}" $0
  Pop $0

  ${If} $0 == 603
    goto notRunning
  ${EndIf}

  ${If} $0 == 0
    MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "${NAME_UNVERSIONED} is running. Please close it first" /SD IDCANCEL IDRETRY loop
    abort
  ${EndIf}

  MessageBox MB_OK|MB_ICONEXCLAMATION "Unable to detect if ${NAME_UNVERSIONED} is running. Make sure it is closed before proceeding" /SD IDOK

  notRunning:
!macroend

#TODO avoid associating multiple times the same extension but from different filetypes
!macro addSectionContent ext
  !insertmacro AssociateExtensionMacro "${ext}"
!macroend

!macro createDirSettingsUserWithWritePermissions
  CreateDirectory "${DIR_SETTINGS_USER}"

  setDirPermissions:
  AccessControl::GrantOnFile "${DIR_SETTINGS_USER}" "(BU)" "GenericRead + GenericWrite"
  Pop $0
  ${If} $0 == error
    Pop $0
    DetailPrint "AccessControl error: $0"
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Unable to set permissions for directory$\r$\n${DIR_SETTINGS_USER}:\
    $\r$\n'$0'$\r$\n" /SD IDABORT IDRETRY setDirPermissions IDIGNORE afterSetDirPermissions
    quit
  ${EndIf}

  afterSetDirPermissions:
!macroend

RequestExecutionLevel admin
OutFile "${SETUP_FILENAME}"
Name "$title"
Caption "${CAPTION_VERSIONED}"
InstallDir "$PROGRAMFILES32\${NAME}"
InstallDirRegKey HKLM "Software\${PUBLISHER}\${NAME}" "InstallDir"
ShowInstDetails show
ShowUninstDetails show
SpaceTexts none

!define MUI_BGCOLOR "SYSCLR:Window"
!define MUI_TEXTCOLOR "SYSCLR:WindowText"
!define MUI_ICON "${ICON_FILE}"
!define MUI_UNICON "${ICON_FILE}"
!define MUI_ABORTWARNING
!define MUI_ABORTWARNING_CANCEL_DEFAULT
!define MUI_UNABORTWARNING
!define MUI_UNABORTWARNING_CANCEL_DEFAULT

#TODO
#!define MUI_WELCOMEFINISHPAGE_BITMAP "banner.bmp"
!define MUI_WELCOMEPAGE_TEXT "Setup will first update itself (if needed) then check for latest ${NAME} version \
and guide you through its installation (an internet connection is required).$\r$\n$\r$\n\
It is recommended that you close all other applications before starting. \
This will make it possible to update relevant system files without having to reboot your computer.\
$\r$\n$\r$\n$_CLICK"
#TODO
#!define MUI_UNWELCOMEFINISHPAGE_BITMAP "banner.bmp"

!define MUI_PAGE_CUSTOMFUNCTION_PRE skipInstallerUpdateCheck
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE updateInstallerItself
!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE checkLatestVersion
!insertmacro MUI_PAGE_LICENSE "${LICENSE_FILE}"
page custom changelog
!define MUI_PAGE_CUSTOMFUNCTION_PRE componentsPre
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Run ${NAME}"
!define MUI_FINISHPAGE_RUN_CHECKED
!define MUI_FINISHPAGE_LINK "${URL_MAIN}"
!define MUI_FINISHPAGE_LINK_LOCATION "${URL_MAIN}"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE saveRunButtonCheckState
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "!${NAME} (required)"
  SectionIn RO

  var /global releaseArchiveFilename

  ${If} $localReleaseArchivePath != ""
    goto beforeUnpacking
  ${EndIf}

  StrCpy $releaseArchiveFilename "${RELEASE_ARCHIVE_FILENAME_UNVERSIONED}$version.zip"

  inetc::get /RESUME "" /QUESTION "" "${URL_RELEASE_ARCHIVE}" "${RELEASE_ARCHIVE_FILE_PATH}"
  Pop $0
  StrCmp $0 "OK" downloadReleaseArchiveOk
  DetailPrint "$0"
  inetc::get /RESUME "" /QUESTION "" "${URL_RELEASE_ARCHIVE_FALLBACK}/$releaseArchiveFilename" "${RELEASE_ARCHIVE_FILE_PATH}"
  Pop $0
  StrCmp $0 "OK" downloadReleaseArchiveOk
  DetailPrint "$0"
  MessageBox MB_OK|MB_ICONEXCLAMATION "Error downloading $releaseArchiveFilename:$\r$\n'$0'$\r$\nInstallation aborted" /SD IDOK
  quit

  downloadReleaseArchiveOk:
  DetailPrint "$releaseArchiveFilename successufully downloaded"

  beforeUnpacking:
  !insertmacro DetectRunningBzr2
  SetOutPath "$INSTDIR"

  nsisunz::UnzipToLog "${RELEASE_ARCHIVE_FILE_PATH}" "$INSTDIR"
  Pop $0
  StrCmp $0 "success" unpackOk
  DetailPrint "$0"
  MessageBox MB_OK|MB_ICONEXCLAMATION "Error unpacking $releaseArchiveFilename:$\r$\n'$0'$\r$\nInstallation aborted" /SD IDOK
  abort

  unpackOk:
  CopyFiles "$ExePath" "$INSTDIR\install.exe"

  !insertmacro createDirSettingsUserWithWritePermissions

  WriteUninstaller "$INSTDIR\uninstall.exe"

  WriteRegStr HKLM "Software\${PUBLISHER}\${NAME}" "ApplicationName" "${NAME}"
  WriteRegStr HKLM "Software\${PUBLISHER}\${NAME}" "ApplicationDescription" "${DESCRIPTION}"
  WriteRegStr HKLM "Software\${PUBLISHER}\${NAME}" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayIcon" "$INSTDIR\${EXE_FILENAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayName" "${NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayVersion" "$version"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "Publisher" "${PUBLISHER}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "UninstallString" "$INSTDIR\uninstall.exe"

  # Add to "recommended programs" for the following extensions
  WriteRegStr HKCR "Applications\${EXE_FILENAME}" "" ""
  WriteRegStr HKCR "Applications\${EXE_FILENAME}" "FriendlyAppName" "${NAME}"
  WriteRegStr HKCR "Applications\${EXE_FILENAME}\shell\Open" "" "Play with ${NAME}"
  WriteRegStr HKCR "Applications\${EXE_FILENAME}\shell\Open\command" "" '"$INSTDIR\${EXE_FILENAME}" "%1"'

  #TODO avoid registering multiple times the same extension but from different filetypes
  ${ForEachIn} extensions $R0 $R1
    !insertmacro RegisterAudioExtensionMacro "$R1"
  ${Next}

  # Add to "App Paths" to run bzr2 from ShellExecute/ShellExecuteEx/run dialog without giving a full path
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}" "" "$INSTDIR\${EXE_FILENAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}" "Path" "$INSTDIR"

  # Windows default programs Registration
  ${If} ${AtLeastWinVista}
    WriteRegStr HKLM "Software\RegisteredApplications" "${NAME_SHORT}" "Software\Clients\Media\${NAME_SHORT}\Capabilities"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities" "ApplicationName" "${NAME}"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}\Capabilities" "ApplicationDescription" "${NAME_SHORT} - ${DESCRIPTION}"
    WriteRegStr HKLM "Software\Clients\Media\${NAME_SHORT}" "" "${NAME}"
  ${EndIf}
SectionEnd

Section "Start Menu Shortcut"
  CreateDirectory "$SMPROGRAMS\${NAME}"
  CreateShortCut "$SMPROGRAMS\${NAME}\${NAME}.lnk" "$INSTDIR\${EXE_FILENAME}" "" "$INSTDIR\${EXE_FILENAME}" "" "" "" "${DESCRIPTION}"
  CreateShortCut "$SMPROGRAMS\${NAME}\Update.lnk" "$INSTDIR\install.exe"
  CreateShortCut "$SMPROGRAMS\${NAME}\Uninstall.lnk" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Desktop Shortcut"
  CreateShortCut "$DESKTOP\${NAME}.lnk" "$INSTDIR\${EXE_FILENAME}"
SectionEnd

SectionGroup "File type associations"
  !insertmacro addFiletypeAssociationsComponentContent
SectionGroupEnd

Section ""
  # unAssociate unchecked extensions
  ${ForEachIn} extensions $R0 $R1
    !insertmacro SectionFlagIsSet $R0 ${SF_SELECTED} +1 extensionUnchecked
  ${Continue}

  extensionUnchecked:
  !insertmacro UnAssociateExtensionMacro "$R1"
  ${Next}

#TODO association removal of not still supported extensions:
#so if a next bzr2 version drop support for an extension, and this was previosuly associated to bzr2, the installer will remove the association:
#remove any BZR.ext from registry (loop on them) that is not included in "extensions" array (they come from old versions)
#TODO (maybe?) unRegister stuff from registry (for all extensions) if its checkbox has not been checked

SectionEnd

Section "Context Menus" secCtxMenus
  ${ForEachIn} extensions $R0 $R1
    ${If} ${SectionIsSelected} $R0
      !insertmacro AddContextMenuExtMacro "$R1"
    ${EndIf}
  ${Next}
  !insertmacro AddContextMenuDirMacro
SectionEnd

Section ""
  ${If} ${SectionIsSelected} ${secCtxMenus}
  ${Else}
    ${ForEachIn} extensions $R0 $R1
      !insertmacro DeleteContextMenuExtMacro "$R1"
    ${Next}
  ${EndIf}
SectionEnd

Section /o "Delete Preferences"
  RMDir /r "${DIR_SETTINGS_USER}"
  !insertmacro createDirSettingsUserWithWritePermissions
SectionEnd

Function .onInit
  ClearErrors
  ${GetOptions} $CMDLINE "/localReleaseArchivePath=" $localReleaseArchivePath

  ${If} $localReleaseArchivePath != ""
    IfFileExists $localReleaseArchivePath 0 localReleaseArchiveFileNotFound
    ${GetParent} "$localReleaseArchivePath" $releaseArchiveFileDir
    ${GetFileName} "$localReleaseArchivePath" $releaseArchiveFilename

    #TODO a regex should be used here
    ${WordFind} "$releaseArchiveFilename" "${RELEASE_ARCHIVE_FILENAME_UNVERSIONED}" "E+1}" $version
    IfErrors localReleaseArchiveFilenameNotValid
    ${WordFind} "$version" ".zip" "E+1{" $version
    IfErrors localReleaseArchiveFilenameNotValid
    ${WordFind} "$version" "2." "E+1}" $R0
    IfErrors localReleaseArchiveFilenameNotValid
    StrCmp $R0 "" localReleaseArchiveFilenameNotValid
    StrCpy $title "${NAME_UNVERSIONED} $version"
    goto localReleaseArchivePathCheckEnd

    localReleaseArchiveFileNotFound:
    MessageBox MB_ICONSTOP "Invalid local release archive path provided:$\r$\nfile not found" IDOK quit

    localReleaseArchiveFilenameNotValid:
    MessageBox MB_ICONSTOP "Invalid local release archive filename provided" IDOK quit

    quit:
    quit

    localReleaseArchivePathCheckEnd:
  ${Else}
    StrCpy $releaseArchiveFileDir "$TEMP"
    StrCpy $title "${NAME}"
  ${EndIf}

  var /global skipInstallerUpdate
  ClearErrors
  ${GetOptions} $CMDLINE "/skipInstallerUpdate" $skipInstallerUpdate
FunctionEnd

Function skipInstallerUpdateCheck
  ${If} $skipInstallerUpdate == 1
    BringToFront
    StrCpy $skipInstallerUpdate 2
    abort
  ${EndIf}
FunctionEnd

Function updateInstallerItself
  ${If} $skipInstallerUpdate == 2
    goto end
  ${EndIf}

  delete "$TEMP\${SETUP_FILENAME}"

  checkLatestInstallerVersion:
  inetc::get /RESUME "" /QUESTION "" /TOSTACK "${URL_SETUP_LATEST_VERSION}" ""
  Pop $0
  ${If} $0 == "OK"
    Pop $0
    DetailPrint "latest installer version found: $0"
    ${If} "${SETUP_LATEST_VERSION}" != "$0"
      downloadLatestInstallerVersion:
      inetc::get /RESUME "" /QUESTION "" "${URL_SETUP_EXE}" "$TEMP\${SETUP_FILENAME}"
      Pop $0
      ${If} $0 == "OK"
        DetailPrint "latest installer version successfully downloaded"
        Exec '"$TEMP\${SETUP_FILENAME}" /skipInstallerUpdate 1'
        quit
      ${Else}
        DetailPrint "$0"
        MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Error downloading latest installer version:\
        $\r$\n'$0'$\r$\n" /SD IDABORT IDRETRY downloadLatestInstallerVersion IDIGNORE end
        quit
      ${EndIf}
    ${EndIf}
  ${Else}
    DetailPrint "$0"
    MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Unable to check latest installer version online: \
    $\r$\n'$0'$\r$\n" /SD IDABORT IDRETRY checkLatestInstallerVersion IDIGNORE end
    quit
  ${EndIf}



  end:
FunctionEnd

Function checkLatestVersion
  ${If} $localReleaseArchivePath != ""
    goto end
  ${EndIf}

  var /global latestVersionAlreadyChecked

  ${If} $latestVersionAlreadyChecked != 1
    var /global alreadyInstalledVersion

    check:
    ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" "DisplayVersion"
    StrCpy $alreadyInstalledVersion $0

    inetc::get /RESUME "" /QUESTION "" /TOSTACK "${URL_LATEST_VERSION}" ""
    Pop $0
    ${If} $0 == "OK"
      Pop $0
      StrCpy $version $0
      DetailPrint "latest version found: $1"
    ${Else}
      DetailPrint "$0"
      StrCmp "$alreadyInstalledVersion" "" 0 canIgnoreLatestVersionCheck
      MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "Unable to check latest ${NAME} version online:$\r$\n'$0'" \
      /SD IDCANCEL IDRETRY check IDCANCEL 0
      quit

      canIgnoreLatestVersionCheck:
      MessageBox MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION "Unable to check latest ${NAME} version online:$\r$\n'$0'" \
      /SD IDABORT IDRETRY check IDIGNORE next
      quit
    ${EndIf}

    next:
    ${If} $alreadyInstalledVersion != ""
      ${If} $version == ""
        StrCpy $version "$alreadyInstalledVersion"
        MessageBox MB_YESNO|MB_ICONQUESTION "Do you want to reinstall ${NAME_UNVERSIONED} $version?" \
        /SD IDNO IDYES end
        quit
      ${EndIf}
      ${If} $alreadyInstalledVersion == "$version"
        MessageBox MB_YESNO|MB_ICONQUESTION "Latest version of ${NAME_UNVERSIONED} $version it is already installed. \
        Do you want to reinstall it?" /SD IDNO IDYES end
        quit
      ${EndIf}
    ${EndIf}

    end:
    var /global isNewVersionFound
    ${If} "$version" != ""
    ${AndIf} "$version" != "$alreadyInstalledVersion"
      StrCpy $isNewVersionFound 1
    ${EndIf}

    StrCpy $title "${NAME_UNVERSIONED} $version"
    StrCpy $latestVersionAlreadyChecked 1
  ${EndIf}
FunctionEnd

Function changelog
  ${If} $localReleaseArchivePath != ""
    goto end
  ${EndIf}

  ${If} "$isNewVersionFound" != 1
    abort
  ${EndIf}

  nsDialogs::Create /NOUNLOAD 1044
  Pop $0

  var /global richEditChangelog

  nsDialogs::CreateControl /NOUNLOAD "RICHEDIT20W" \
  ${WS_VISIBLE}|${WS_CHILD}|${WS_BORDER}|${WS_TABSTOP}|${ES_READONLY}|${WS_VSCROLL}|${ES_MULTILINE} \
  ${WS_EX_STATICEDGE} 0 0 100% 100% ""
  Pop $richEditChangelog

  var /global changelogAlreadyProcessed
  ${If} $changelogAlreadyProcessed == 1
    goto changelogProcessed
  ${EndIf}

  inetc::get /RESUME "" /QUESTION "" /BANNER "Downloading changelog..." /CAPTION "${CAPTION}" \
  "${URL_CHANGELOG}" "${CHANGELOG_FILE_JSON}"
  Pop $0
  ${If} $0 != "OK"
    DetailPrint "$0"
    SendMessage $richEditChangelog ${WM_SETTEXT} 0 "STR:Error retrieving online changelog: '$0'"
    goto end
  ${EndIf}

  var /global changelogRtfFile
  ClearErrors
  FileOpen $changelogRtfFile "${CHANGELOG_FILE_RTF}" w

  ${If} ${Errors}
    SendMessage $richEditChangelog ${WM_SETTEXT} 0 "STR:Error processing changelog"
    goto end
  ${EndIf}

  FileWrite $changelogRtfFile "{\rtf1 "

  var /global indexReleases
  var /global indexFeatures
  var /global indexBugFixes
  var /global indexOther
  StrCpy $indexReleases 0

  nsJSON::Set /file "${CHANGELOG_FILE_JSON}"

  StrCpy $0 0
  ClearErrors
  nsJSON::Get /count "releases"
  Pop $0

  ${If} ${Errors}
  ${OrIf} $0 == 0
    SendMessage $richEditChangelog ${WM_SETTEXT} 0 "STR:Error processing changelog"
    goto end
  ${EndIf}

  loopReleases:
  ClearErrors
  nsJSON::Get "releases" /index "$indexReleases" /end
  IfErrors changelogJsonParsed 0
  Pop $R0

  ClearErrors
  nsJSON::Get "releases" /index "$indexReleases" "version" /end
  Pop $R0
  nsJSON::Get "releases" /index "$indexReleases" "date" /end
  Pop $R1
  FileWrite $changelogRtfFile "\fs20\b $R0 \b0\fs16 ($R1)"

  ClearErrors
  nsJSON::Get /exists "releases" /index "$indexReleases" "features" /end
  Pop $R0
  ${If} "$R0" == "yes"
    ClearErrors
    nsJSON::Get /count "releases" /index "$indexReleases" "features" /end
    Pop $R0
    IntOp $R0 $R0 - 1
    FileWrite $changelogRtfFile "\line\line\fs18\i New features:\i0\fs16"
    StrCpy $indexFeatures 0
    ${ForEach} $indexFeatures 0 $R0 + 1
      nsJSON::Get "releases" /index "$indexReleases" "features" /index "$indexFeatures" /end
      Pop $R1
      FileWrite $changelogRtfFile "\line  \bullet  $R1"
    ${Next}
  ${EndIf}

  ClearErrors
  nsJSON::Get /exists "releases" /index "$indexReleases" "bugFixes" /end
  Pop $R0
  ${If} "$R0" == "yes"
    ClearErrors
    nsJSON::Get /count "releases" /index "$indexReleases" "bugFixes" /end
    Pop $R0
    IntOp $R0 $R0 - 1
    FileWrite $changelogRtfFile "\line\line\fs18\i Bug fixes:\i0\fs16"
    StrCpy $indexBugFixes 0
    ${ForEach} $indexBugFixes 0 $R0 + 1
      nsJSON::Get "releases" /index "$indexReleases" "bugFixes" /index "$indexBugFixes" /end
      Pop $R1
      FileWrite $changelogRtfFile "\line  \bullet  $R1"
    ${Next}
  ${EndIf}

  ClearErrors
  nsJSON::Get /exists "releases" /index "$indexReleases" "other" /end
  Pop $R0
  ${If} "$R0" == "yes"
    ClearErrors
    nsJSON::Get /count "releases" /index "$indexReleases" "other" /end
    Pop $R0
    IntOp $R0 $R0 - 1
    FileWrite $changelogRtfFile "\line\line\fs18\i Other:\i0\fs16"
    StrCpy $indexOther 0
    ${ForEach} $indexOther 0 $R0 + 1
      nsJSON::Get "releases" /index "$indexReleases" "other" /index "$indexOther" /end
      Pop $R1
      FileWrite $changelogRtfFile "\line  \bullet  $R1"
    ${Next}
  ${EndIf}

  FileWrite $changelogRtfFile "\line\line"
  IntOp $indexReleases $indexReleases + 1
  goto loopReleases

  changelogJsonParsed:
  FileSeek $changelogRtfFile -5 CUR
  FileWrite $changelogRtfFile "}"
  FileClose $changelogRtfFile

  StrCpy $changelogAlreadyProcessed 1

  changelogProcessed:
  nsRichEdit::Load $richEditChangelog "${CHANGELOG_FILE_RTF}"
  SendMessage $richEditChangelog ${EM_SETSEL} 0 0

  end:
  nsDialogs::Show
FunctionEnd


Function componentsPre
  nsArray::Clear extensions
  StrCpy $i 0
  ${ForEach} $i ${firstFiletypeAssociationsSection} ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRP} +1 isNotSectionGroup
    ${Continue}

    isNotSectionGroup:
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd
    ${Continue}

    isNotSectionGroupEnd:
    SectionGetText "$i" $R0
    nsArray::Set extensions /key=$i $R0
  ${Next}

  # pre-uncheck extensions registered but not associated (if bzr2 it is already installed)
  ClearErrors
  EnumRegKey $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}" 0
  IfErrors done 0

  ${ForEachIn} extensions $R0 $R1
    ${If} ${AtLeastWinVista}
      StrCpy $letter ""
      StrCpy $mruListContent ""
      ${registry::Open} "HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\$R1\OpenWithList" "K=0" $R2
      StrCmp $R4 0 close loop

      loop:
      ${registry::Find} $R2 $R3 $R4 $R5 $R6
      StrCmp $R3 "" close 0
      StrCmp $R4 "MRUList" 0 noMRUList
      StrCpy $mruListContent $R5
      goto loop

      noMRUList:
      StrCmp $R5 "${EXE_FILENAME}" 0 loop
      StrCpy $letter $R4
      goto loop

      close:
      ${registry::Close} $R2

      StrCpy $R2 "$mruListContent" 1

      ${If} "$R2" != ""
      ${AndIf} $R2 != "$letter"
        goto deselect
      ${Else}
        goto next
      ${EndIf}
    ${Else}
      ReadRegStr $0 HKCR "${NAME_SHORT}$R1" ""
      StrCmp $0 "" next
      ReadRegStr $0 HKCR "$R1" ""
      StrCmp $0 "${NAME_SHORT}$R1" next
    ${EndIf}

   deselect:
   !insertmacro ClearSectionFlag $R0 ${SF_SELECTED}

   next:
  ${Next}

  done:
FunctionEnd

Function .onSelChange
  var /global triggeredSectionNum
  StrCpy $triggeredSectionNum $0

  ${If} $triggeredSectionNum < "${firstFiletypeAssociationsSection}"
  ${OrIf} $triggeredSectionNum > "${lastFiletypeAssociationsSection}"
    goto onSelChangeEnd
  ${EndIf}

  var /global triggeredSectionText
  SectionGetText $triggeredSectionNum $triggeredSectionText
  nsArray::Clear extensionsToToggle

  !insertmacro SectionFlagIsSet $triggeredSectionNum ${SF_SECGRP} isTriggeredSectionGroup isNotTriggeredSectionGroup
  isNotTriggeredSectionGroup:
  nsArray::Set extensionsToToggle /key=$triggeredSectionNum $triggeredSectionText
  goto propagateToggles

  isTriggeredSectionGroup:
  StrCpy $i $triggeredSectionNum
  IntOp $i $i + 1

  ${ForEach} $i $i ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd1
    ${Break}

    isNotSectionGroupEnd1:
      SectionGetText "$i" $R0
      nsArray::Set extensionsToToggle /key=$i $R0
  ${Next}

  propagateToggles:
  StrCpy $i 0

  ${ForEach} $i ${firstFiletypeAssociationsSection} ${lastFiletypeAssociationsSection} + 1
    !insertmacro SectionFlagIsSet $i ${SF_SECGRP} +1 isNotSectionGroup
    ${Continue}

    isNotSectionGroup:
    !insertmacro SectionFlagIsSet $i ${SF_SECGRPEND} +1 isNotSectionGroupEnd2
    ${Continue}

    isNotSectionGroupEnd2:
    ${ForEachIn} extensionsToToggle $R0 $R1
      SectionGetText "$i" $R2

      ${If} "$R2" == "$R1"
      ${AndIf} "$i" != "$R0"
        !insertmacro SectionFlagIsSet $R0 ${SF_SELECTED} +1 extensionsToToggleIsNotSet
        !insertmacro SelectSection $i
        goto extensionTogglingEnd

        extensionsToToggleIsNotSet:
        !insertmacro UnSelectSection $i

        extensionTogglingEnd:
      ${EndIf}
    ${Next}
  ${Next}

  onSelChangeEnd:
FunctionEnd

Function saveRunButtonCheckState
  SendMessage $mui.FinishPage.Run ${BM_GETCHECK} 0 0 $mui.FinishPage.ReturnValue

  ${if} $mui.FinishPage.ReturnValue = ${BST_CHECKED}
    var /global isRunButtonChecked
    StrCpy $isRunButtonChecked 1
  ${EndIf}
FunctionEnd

Function .onGUIEnd
  delete "${CHANGELOG_FILE_JSON}"
  delete "${CHANGELOG_FILE_RTF}"
  delete "$PLUGINSDIR"

  ${If} $localReleaseArchivePath == ""
    delete "${RELEASE_ARCHIVE_FILE_PATH}"
  ${EndIf}

  ${if} $isRunButtonChecked == 1
    Exec '"$WINDIR\explorer.exe" "$INSTDIR\${EXE_FILENAME}"'

    # ensures bzr2 opening on foreground
    loop:
    ${nsProcess::FindProcess} "${EXE_FILENAME}" $0
    Pop $0
    IntCmp $0 603 loop
  ${EndIf}
FunctionEnd

Section "un.${NAME}"
  SectionIn RO
  !insertmacro DetectRunningBzr2

  Delete "$INSTDIR\*.*"
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\platforms"
  RMDir "$INSTDIR"
  RMDir /r "$SMPROGRAMS\${NAME}"
  Delete "$DESKTOP\${NAME}.lnk"

  !insertmacro UnRegisterExtensionsAndCleanupGarbageMacro
  !insertmacro DeleteContextMenuDirMacro

  DeleteRegKey HKCR "Applications\${EXE_FILENAME}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\App Paths\${EXE_FILENAME}"
  DeleteRegKey HKLM "Software\Clients\Media\${NAME_SHORT}"
  DeleteRegValue HKLM "Software\RegisteredApplications" "${NAME_SHORT}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${NAME}"
  DeleteRegKey HKLM "Software\${PUBLISHER}\${NAME}"
SectionEnd

Section /o "un.Preferences"
  RMDir /r "${DIR_SETTINGS_USER}"
  RMDir "$INSTDIR"
SectionEnd

Function un.onInit
  StrCpy $title "${NAME}"
FunctionEnd

!system 'echo -n "${SETUP_LATEST_VERSION}" > ${SETUP_FILENAME_LATEST_VERSION}' = 0
