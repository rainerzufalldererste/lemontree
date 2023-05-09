git clean -ffdx
git submodule foreach --recursive "git clean -ffdx"
git fetch origin

git merge origin/master
IF %ERRORLEVEL% NEQ 0 (
  echo "merged with master failed"
  git merge --abort
  exit 1;
)
echo "merged successfully with master"

git submodule sync --recursive
git submodule update --init

"3rdParty\\premake\\premake5.exe" vs2022
IF %ERRORLEVEL% NEQ 0 (exit 1)

SET VCTargetsPath="C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Microsoft\VC\v170\"
SET VCTargetsPath=%VCTargetsPath:"=%

"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" lemontree.sln /p:Configuration="%~1" /p:Platform="x64" /m:4 /v:m
IF %ERRORLEVEL% NEQ 0 (exit 1)
