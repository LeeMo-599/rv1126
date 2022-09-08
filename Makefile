hide := @
ECHO := echo

G++ := /opt/rv1126_gcc/prebuilts/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++
CFLAGS := -I./include/rkmedia \
			-I./include/rkaiq/common \
			-I./include/rkaiq/xcore \
			-I./include/rkaiq/uAPI \
			-I./include/rkaiq/algos \
			-I./include/rkaiq/iq_parser \
			-I./rknn_rockx_include \
			-I./im2d_api          \
			-I./arm_libx264/include \
			-I./arm32_ffmpeg_srt/include \
			-I/ 

LIB_FILES := -L./rv1126_lib  -L./arm_libx264/lib -L./arm32_ffmpeg_srt/lib #-L/opt/arm32_ffmpeg/lib

LD_FLAGS := -lpthread -leasymedia -ldrm -lrockchip_mpp \
	        -lavformat -lavcodec -lswresample -lavutil \
			-lasound -lv4l2 -lv4lconvert -lrga \
			-lRKAP_ANR -lRKAP_Common -lRKAP_3A \
			-lmd_share -lrkaiq -lod_share -lrknn_api \
			-lrockx -lx264 	
			

CFLAGS += -DRKAIQ

SAMPLE_COMMON := common/sample_common_isp.c
SAMPLE_COMMON_02 := sample_common_isp.c

all:
	$(G++) rv1126_ffmpeg_main.cpp rv1126_isp_function.cpp rv1126_task_function.cpp rv1126_vi_ai_function.cpp rv1126_vi_ai_map.cpp rv1126_task_manage.cpp ffmpeg_module.cpp ffmpeg_group.cpp ffmpeg_video_queue.cpp ffmpeg_audio_queue.cpp rv1126_data_process.cpp $(SAMPLE_COMMON_02) $(CFLAGS) $(LIB_FILES) $(LD_FLAGS) -o rv1126_ffmpeg_main
	$(G++) rv1126_venc_demo.cpp $(SAMPLE_COMMON_02) $(CFLAGS) $(LIB_FILES) $(LD_FLAGS) -o rv1126_venc_demo
	$(G++) rv1126_aenc_demo.cpp $(SAMPLE_COMMON_02) $(CFLAGS) $(LIB_FILES) $(LD_FLAGS) -o rv1126_aenc_demo
	$(hide)$(ECHO) "Build Done ..."

