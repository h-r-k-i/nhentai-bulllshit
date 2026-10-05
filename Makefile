all: build run clean

ifeq ($(OS),Windows_NT)
EXE := porn.exe
BUILD_CMD := gcc -Wall -Wextra cJSON.c -IC:/msys64/ucrt64/include -LC:/msys64/ucrt64/lib pornography.c -o porn.exe -lcurl
RUN_CMD := porn.exe $(CODE)
RM_CMD := del
else
EXE := porn
BUILD_CMD := gcc -Wall -Wextra cJSON.c pornography.c -o porn -lcurl
RUN_CMD := ./porn $(CODE)
RM_CMD := rm -f
endif

build:
	$(BUILD_CMD)

run:
	$(RUN_CMD)

clean:
	$(RM_CMD) $(EXE)

clean1:
	$(RM_CMD) out.txt
	$(RM_CMD) $(EXE)

PHONY: clean clean1
