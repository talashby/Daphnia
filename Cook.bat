@echo off

set WORKSPACE=%~dp0.
set PROJECT_NAME=Daphnia
if not defined SOLUTION_CONFIGURATION (
  echo "SOLUTION_CONFIGURATION is not defined"
  exit 1
  )
if exist CookEnginePathSet.bat call CookEnginePathSet.bat else exit 1
if not defined UE4_ROOT echo "UE4_ROOT is not defined"
call "%UE4_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%WORKSPACE%/%PROJECT_NAME%.uproject" -noP4 -platform=Win64 -platform=Win64 -clientconfig=%SOLUTION_CONFIGURATION% -serverconfig=%SOLUTION_CONFIGURATION% -noclean -build -cook -allmaps -utf8output -stage -pak -compressed -archive -archivedirectory="%WORKSPACE%/Archive"
