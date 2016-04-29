@echo off
rem
rem  Copyright 2016 WebAssembly Community Group participants
rem
rem  Licensed under the Apache License, Version 2.0 (the "License");
rem  you may not use this file except in compliance with the License.
rem  You may obtain a copy of the License at
rem
rem      http://www.apache.org/licenses/LICENSE-2.0
rem
rem  Unless required by applicable law or agreed to in writing, software
rem  distributed under the License is distributed on an "AS IS" BASIS,
rem  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
rem  See the License for the specific language governing permissions and
rem  limitations under the License.
rem

setlocal

set CONFIG=Release

if "%1" == "/Debug" (
  set CONFIG=Debug
)

set SCRIPT_DIR=%~dp0
set ROOT_DIR=%SCRIPT_DIR%\..
set V8_DIR=%ROOT_DIR%\third_party\v8
set DEPOT_TOOLS_DIR=%V8_DIR%\depot_tools

cd %V8_DIR%

if not exist %DEPOT_TOOLS_DIR% (
  call git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
)

set PATH=%DEPOT_TOOLS_DIR%;%PATH%
call gclient sync

cd v8
set GYP_GENERATORS=ninja
call python gypfiles\gyp_v8
call ninja -C out\%CONFIG% d8.exe
