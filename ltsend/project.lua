ProjectName = "ltsend"
project(ProjectName)

  --Settings
  kind "WindowedApp"
  language "C++"
  staticruntime "On"

  filter { "system:windows" }
    buildoptions { '/Gm-' }
    buildoptions { '/MP' }

    ignoredefaultlibraries { "msvcrt" }
    ignoredefaultlibraries { "libcmt" }
  filter { "system:linux" }
    cppdialect "C++11"
  filter { }
  
  defines { "_CRT_SECURE_NO_WARNINGS", "SSE2" }
  
  objdir "intermediate/obj"

  files { "src/**.cpp", "src/**.c", "src/**.cc", "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl", "src/**rc", "src/**.asm" }
  files { "project.lua" }

  includedirs { "3rdParty/monocypher" }
  
  targetname(ProjectName)
  targetdir "../builds/bin/client"
  debugdir "../builds/bin/client"

  if os.getenv("CI_BUILD_REF_NAME") then
    postbuildcommands { "etc/etc_git_build.bat" }
  else
    postbuildcommands { "etc/etc_no_git_build.bat" }
  end
  
filter {}
configuration {}

warnings "Extra"
targetname "%{prj.name}"

-- Strings
if os.getenv("CI_BUILD_REF_NAME") then
  defines { "GIT_BRANCH=\"" .. os.getenv("CI_BUILD_REF_NAME") .. "\"" }
end
if os.getenv("CI_COMMIT_SHA") then
  defines { "GIT_REF=\"" .. os.getenv("CI_COMMIT_SHA") .. "\"" }
end

-- Numbers
if os.getenv("CI_PIPELINE_IID") then
  defines { "GIT_BUILD=" .. os.getenv("CI_PIPELINE_IID") }
else
  defines { "DEV_BUILD" }
end
if os.getenv("UNIXTIME") then
  defines { "BUILD_TIME=" .. os.getenv("UNIXTIME") }
end

filter {}
configuration {}
flags { "NoMinimalRebuild", "NoPCH" }
exceptionhandling "Off"
rtti "Off"
floatingpoint "Fast"

filter { "configurations:Debug*" }
	defines { "_DEBUG" }
	optimize "Off"
	symbols "On"

filter { "configurations:Release" }
	defines { "NDEBUG" }
	optimize "Speed"
	flags { "NoBufferSecurityCheck", "NoIncrementalLink" }
  omitframepointer "On"
	symbols "On"

filter { "system:windows", "configurations:Release", "action:vs2012" }
	buildoptions { "/d2Zi+" }

filter { "system:windows", "configurations:Release", "action:vs2013" }
	buildoptions { "/Zo" }

filter { "system:windows", "configurations:Release" }
	flags { "NoIncrementalLink" }
  editandcontinue "Off"
