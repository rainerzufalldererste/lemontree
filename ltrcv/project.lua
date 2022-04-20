ProjectName = "ltrcv"
project(ProjectName)

  --Settings
  kind "ConsoleApp"
  language "C#"
  framework "4.7.2"

  objdir "intermediate/obj"

  files { "src/**.cs" }
  files { "project.lua" }

  configuration {}
  
  configuration { }
  
  targetname(ProjectName)
  targetdir "../builds/bin/server"
  debugdir "../builds/bin/server"
  
filter {}
configuration {}

links { "System" }
links { "System.Data" }

filter { "configurations:Debug*" }
  defines { "_DEBUG", "DEBUG" }

filter { "configurations:Release" }
  defines { "NDEBUG", "RELEASE" }

filter {}

warnings "Extra"
