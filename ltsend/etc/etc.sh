mkdir include

xxd -i ../builds/bin/client/ltsend.exe > ../builds/bin/client/ltsend_exe.h
if [ $? -ne 0 ]; then exit 1; fi
