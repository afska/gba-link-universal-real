#---------------------------------------------------------------------------------------------------------------------
# TARGET is the name of the output.
# BUILD is the directory where object files & intermediate files will be placed.
# LIBBUTANO is the main directory of butano library (https://github.com/GValiente/butano).
# PYTHON is the path to the python interpreter.
# SOURCES is a list of directories containing source code.
# INCLUDES is a list of directories containing extra header files.
# DATA is a list of directories containing binary data.
# GRAPHICS is a list of files and directories containing files to be processed by grit.
# AUDIO is a list of files and directories containing files to be processed by mmutil.
# DMGAUDIO is a list of files and directories containing files to be processed by mod2gbt and s3m2gbt.
# ROMTITLE is a uppercase ASCII, max 12 characters text string containing the output ROM title.
# ROMCODE is a uppercase ASCII, max 4 characters text string containing the output ROM code.
# USERFLAGS is a list of additional compiler flags:
#     Pass -flto to enable link-time optimization.
#     Pass -O0 or -Og to try to make debugging work.
# USERCXXFLAGS is a list of additional compiler flags for C++ code only.
# USERASFLAGS is a list of additional assembler flags.
# USERLDFLAGS is a list of additional linker flags:
#     Pass -flto=<number_of_cpu_cores> to enable parallel link-time optimization.
# USERLIBDIRS is a list of additional directories containing libraries.
#     Each libraries directory must contains include and lib subdirectories.
# USERLIBS is a list of additional libraries to link with the project.
# DEFAULTLIBS links standard system libraries when it is not empty.
# STACKTRACE enables stack trace logging when it is not empty.
# USERBUILD is a list of additional directories to remove when cleaning the project.
# EXTTOOL is an optional command executed before processing audio, graphics and code files.
#
# All directories are specified relative to the project directory where the makefile is found.
#---------------------------------------------------------------------------------------------------------------------
TARGET      	:=  $(notdir $(CURDIR))
BUILD       	:=  build
LIBBUTANO   	:=  butano
PYTHON      	:=  python
SOURCES     	:=  src \
					src/assets \
					src/assets/fonts \
					src/player \
					src/player/core \
					src/scenes \
					src/objects \
					src/utils \
					src/utils/gba-link-connection \
					src/utils/gbfs
INCLUDES    	:=  include
DATA        	:=
GRAPHICS    	:=  graphics
AUDIO       	:=  audio
DMGAUDIO    	:=  dmg_audio
ROMTITLE    	:=  LinkUni
ROMCODE     	:=  LINK
USERFLAGS   	:=  -Ofast -Wno-unused-parameter -DLINK_WIRELESS_QUEUE_SIZE=10 -DLINK_WIRELESS_MAX_SERVER_TRANSFER_LENGTH=6 -DLINK_WIRELESS_PUT_ISR_IN_IWRAM=1 -DLINK_WIRELESS_ENABLE_NESTED_IRQ=1
USERCXXFLAGS	:=  
USERASFLAGS 	:=  
USERLDFLAGS 	:= -Wl,--print-memory-usage
USERLIBDIRS 	:= $(DEVKITPRO)/libgba
USERLIBS    	:= -lgba
DEFAULTLIBS 	:= true
STACKTRACE		:= 
USERBUILD   	:=  
EXTTOOL     	:=  

#---------------------------------------------------------------------------------------------------------------------
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak

XCF ?= XCF_FILE_NEEDED

.PHONY: rebuild sprites

sprites:
	rm -rf graphics/sprites/output graphics/sprites/export_layers.scm
	cd graphics/sprites && node export.js $(XCF).xcf
	rm -rf graphics/sprites/output/_*
	cp -rf graphics/sprites/output/* graphics
	rm -rf graphics/sprites/output graphics/sprites/export_layers.scm

rebuild:
	make -j$(nproc)