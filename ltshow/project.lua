ProjectName = "ltshow"
project(ProjectName)

  --Settings
  kind "ConsoleApp"
  language "C++"
  staticruntime "On"

  filter { "system:windows" }
    buildoptions { '/Gm-' }
    buildoptions { '/MP' }

    ignoredefaultlibraries { "msvcrt" }
  filter { }
  
  cppdialect "C++17"
  
  defines { "_CRT_SECURE_NO_WARNINGS", "SSE2" }
  
  objdir "intermediate/obj"

  files { "src/**.cpp", "src/**.c", "src/**.cc", "src/**.h", "src/**.hh", "src/**.hpp", "src/**.inl", "src/**rc" }
  files { "project.lua" }
  
  includedirs { "../liblt/common" }
  
  filter { "configurations:Debug", "system:Windows" }
    ignoredefaultlibraries { "libcmt" }
  filter { }
  
  targetname(ProjectName)
  targetdir "../builds/bin/server"
  debugdir "../builds/bin/server"
  
filter {}
configuration {}

warnings "Extra"

targetname "%{prj.name}"

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
