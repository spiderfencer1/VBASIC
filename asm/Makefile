all:
	/Applications/nasm/nasm -f macho32 count.s -o count.o && ld -arch i386 -no_pie -lSystem -macosx_version_min 10.10 count.o
