
# MakeFile for CMDALPHA
PNAME=cmdalpha
SRC = cmdalpha.c http.c


all: build

build:
	gcc ${SRC} -o ${PNAME}

