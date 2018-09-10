REM Copyright 2018 WebAssembly Community Group participants
REM
REM Licensed under the Apache License, Version 2.0 (the "License");
REM you may not use this file except in compliance with the License.
REM You may obtain a copy of the License at
REM
REM     http://www.apache.org/licenses/LICENSE-2.0
REM
REM Unless required by applicable law or agreed to in writing, software
REM distributed under the License is distributed on an "AS IS" BASIS,
REM WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
REM See the License for the specific language governing permissions and
REM limitations under the License.

REM Run unittests
%APPVEYOR_BUILD_FOLDER%\bin\wabt-unittests

REM Set up environment so cl.exe is in the path.
REM See docs here: https://www.appveyor.com/docs/lang/cpp/#visual-studio-2015

IF "%TARGET%" == "x86" (
  call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
) ELSE (
  call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /x64
  call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86_amd64
)

REM Run tests
python test\run-tests.py -v --bindir %APPVEYOR_BUILD_FOLDER%\bin %MSVC_FLAG%
