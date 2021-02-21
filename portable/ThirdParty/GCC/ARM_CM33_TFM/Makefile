CC = arm-none-eabi-gcc
BIN := RTOSDemo.axf

BUILD_DIR := build

FREERTOS_DIR_REL := ../../../../../freertos_kernel
FREERTOS_DIR := $(abspath $(FREERTOS_DIR_REL))
KERNEL_DIR := $(FREERTOS_DIR)/

FREERTOS_PLUS_DIR_REL := ../../../../../../FreeRTOS-Plus
FREERTOS_PLUS_DIR := $(abspath $(FREERTOS_PLUS_DIR_REL))

TFM_DIR_REL := ../../../../../../../projects/tfm-fwu
INCLUDE_DIRS += -I${FREERTOS_DIR_REL}/portable/ThirdParty/GCC/ARM_CM33_TFM
INCLUDE_DIRS += -I${TFM_DIR_REL}/interface/include
INCLUDE_DIRS += -I${TFM_DIR_REL}/cmake_build/generated/interface/include
INCLUDE_DIRS += -I${FREERTOS_DIR_REL}/portable/GCC/ARM_CM33/non_secure
INCLUDE_DIRS += -I$(KERNEL_DIR)/include

SOURCE_FILES += init/startup.c syscall.c main.c os_wrapper_freertos.c
SOURCE_FILES += ${FREERTOS_DIR_REL}/portable/GCC/ARM_CM33_NTZ/non_secure/port.c
SOURCE_FILES += ${FREERTOS_DIR_REL}/portable/GCC/ARM_CM33_NTZ/non_secure/portasm.c
SOURCE_FILES += ${TFM_DIR_REL}/interface/src/tfm_ns_interface.c
SOURCE_FILES += ${TFM_DIR_REL}/interface/src/tfm_psa_ns_api.c
SOURCE_FILES += ${TFM_DIR_REL}/interface/src/tfm_firmware_update_ipc_api.c

SOURCE_FILES += $(KERNEL_DIR)/tasks.c
SOURCE_FILES += $(KERNEL_DIR)/list.c
SOURCE_FILES += $(KERNEL_DIR)/queue.c
SOURCE_FILES += $(KERNEL_DIR)/timers.c
SOURCE_FILES += $(KERNEL_DIR)/event_groups.c
SOURCE_FILES += ${KERNEL_DIR}/portable/MemMang/heap_3.c

ifeq ($(FULL_DEMO), 1)
    SOURCE_FILES += main_full.c
    SOURCE_FILES += $(KERNEL_DIR)/stream_buffer.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/AbortDelay.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/BlockQ.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/blocktim.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/countsem.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/death.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/dynamic.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/EventGroupsDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/flop.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/GenQTest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/integer.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/IntSemTest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferAMP.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/MessageBufferDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/PollQ.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/QPeek.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueOverwrite.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSet.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/QueueSetPolling.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/recmutex.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/semtest.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/StaticAllocation.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/StreamBufferInterrupt.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/TaskNotify.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Minimal/TimerDemo.c
    SOURCE_FILES += ${FREERTOS_DIR}/Demo/Common/Full/print.c

    INCLUDE_DIRS += -I$(FREERTOS_DIR)/Demo/Common/include
    INCLUDE_DIRS += -I${FREERTOS_PLUS_DIR}/FreeRTOS-Plus-Trace/Include/

    CFLAGS := -DmainCREATE_FULL_DEMO_ONLY=1
else
    SOURCE_FILES += main_blinky.c

    CFLAGS := -DmainCREATE_SIMPLE_BLINKY_DEMO_ONLY=1
endif

#DEFINES := -DprojCOVERAGE_TEST -DQEMU_SOC_MPS2 -DHEAP3
DEFINES :=  -DQEMU_SOC_MPS2 -DHEAP3

LDFLAGS = -T ./scripts/mps2_m3.ld -specs=nano.specs --specs=rdimon.specs -lc -lrdimon
LDFLAGS += -Xlinker -Map=${BUILD_DIR}/output.map

CFLAGS += -nostartfiles -mcpu=cortex-m33+nodsp -Wno-error=implicit-function-declaration
CFLAGS += -Wno-builtin-declaration-mismatch -Werror

ifeq ($(DEBUG), 1)
    CFLAGS += -ggdb3 -Og
else
    CFLAGS += -O3
endif
    CFLAGS += -fstrict-aliasing -Wstrict-aliasing -Wno-error=address-of-packed-member

OBJ_FILES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)
OBJ_FILES += ${TFM_DIR_REL}/cmake_build/install/interface/lib/s_veneers.o ${TFM_DIR_REL}/cmake_build/platform/libplatform_ns.a

CPPFLAGS += $(DEFINES)
CFLAGS += $(INCLUDE_DIRS)

.PHONY: clean

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES)
	$(CC) -ffunction-sections -fdata-sections $(CFLAGS) $(LDFLAGS) $+ -o $(@)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

INCLUDES := $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.d)
-include $(INCLUDES)

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@

clean:
	-rm -rf build

