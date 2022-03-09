solution "lemontree"
  
  editorintegration "On"
  platforms { "x64" }

  configurations { "Debug", "Release" }

  dofile "liblt/project.lua"
  dofile "ltsend/project.lua"
  dofile "ltshow/project.lua"
  dofile "ltsrv/project.lua"
  dofile "lttrace/project.lua"
  dofile "example/project.lua"
