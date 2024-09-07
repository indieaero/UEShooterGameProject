:: VisualStudio
RMDIR /s /q .vs
RMDIR /s /q .idea
DEL /s /q *.sln
DEL /s /q *.vsconfig

:: Unreal Project
RMDIR /s /q Binaries
RMDIR /s /q DerivedDataCache
RMDIR /s /q Intermediate
RMDIR /s /q Script


:: Сохранение папки Saved\Config\WindowsEditor для сохранения настроек редактора.
IF EXIST Saved\Config\WindowsEditor\ (
    XCOPY /E /I /Y Saved\Config\WindowsEditor Temp\WindowsEditor
)

RMDIR /s /q Saved

IF EXIST Temp\WindowsEditor\ (
    XCOPY /E /I /Y Temp\WindowsEditor Saved\Config\WindowsEditor
)

RMDIR /s /q Temp



:: Удаление времянок из плагинов.
for /d %%d in (Plugins\*) do (
    if exist "%%d\Binaries" (
        rmdir /s /q "%%d\Binaries"
    )
    
    if exist "%%d\Intermediate" (
        rmdir /s /q "%%d\Intermediate"
    )
)