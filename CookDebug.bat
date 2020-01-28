@echo off

set WORKSPACE=%~dp0.
set PROJECT_NAME=Daphnia
set UE4_ROOT=D:\UnrealSources

call "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%WORKSPACE%/%PROJECT_NAME%.uproject" -noP4 -platform=Win64 -platform=Win64 -clientconfig=Debug -serverconfig=Debug -noclean -build -cook -allmaps -utf8output -stage -pak -compressed -archive -archivedirectory="%WORKSPACE%/Archive"
