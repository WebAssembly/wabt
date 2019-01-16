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

REM Set up the artifact for this build, but only if this is a tag build.

IF "%DEPLOY%" == "true" (
  IF "%APPVEYOR_REPO_TAG%" == "true" (
    ren "%APPVEYOR_BUILD_FOLDER%\\bin" "wabt-%APPVEYOR_REPO_TAG_NAME%"
    7z a %DEPLOY_NAME% "%APPVEYOR_BUILD_FOLDER%\\wabt-%APPVEYOR_REPO_TAG_NAME%\\*.exe"
    python "%APPVEYOR_BUILD_FOLDER%\\scripts\\sha256sum.py" "%DEPLOY_NAME%" > "%DEPLOY_NAME%.sha256"
  )
)
