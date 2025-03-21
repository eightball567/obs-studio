project(obs-filters)

option(DISABLE_SPEEXDSP "Disable building of the SpeexDSP-based Noise Suppression filter" OFF)

if(DISABLE_SPEEXDSP)
	message(STATUS "SpeexDSP support disabled")
	set(LIBSPEEXDSP_FOUND FALSE)
else()
	find_package(Libspeexdsp)
	if(LIBSPEEXDSP_FOUND)
		add_definitions(-DLIBSPEEXDSP_ENABLED)
	endif()
endif()

find_package(Librnnoise QUIET)
add_definitions(-DLIBRNNOISE_ENABLED)
if(NOT LIBRNNOISE_FOUND)
	file(GLOB rnnoise_SOURCES
		"rnnoise/src/*.c"
		"rnnoise/src/*.h"
		"rnnoise/include/*.h")
	add_definitions(-DCOMPILE_OPUS)
	if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
		set_property(SOURCE ${rnnoise_SOURCES} PROPERTY COMPILE_FLAGS "-fvisibility=protected")
	endif()
	include_directories("rnnoise/include")
	source_group("rnnoise" FILES ${rnnoise_SOURCES})
	set(LIBRNNOISE_FOUND TRUE)
endif()

if (WIN32)
	if(DISABLE_NVAFX)
		message(STATUS "NVIDIA Audio Effects SDK support disabled")
		set(LIBNVAFX_FOUND FALSE)
	else()
		message(STATUS "NVIDIA Audio Effects SDK support enabled; requires redist to be installed by end-user")
		add_definitions(-DLIBNVAFX_ENABLED)
		set(LIBNVAFX_FOUND TRUE)
	endif()
endif()

if(LIBSPEEXDSP_FOUND OR LIBRNNOISE_FOUND OR LIBNVAFX_FOUND)
	set(obs-filters_NOISEREDUCTION_SOURCES
		noise-suppress-filter.c)
	if(LIBNVAFX_FOUND)
		set(obs-filters_NOISEREDUCTION_HEADERS
			nvafx-load.h)
	else()
		set(obs-filters_NOISEREDUCTION_HEADERS)
	endif()
	set(obs-filters_NOISEREDUCTION_LIBRARIES
		${LIBSPEEXDSP_LIBRARIES} ${LIBRNNOISE_LIBRARIES})
	set(NOISEREDUCTION_ENABLED TRUE)
else()
	set(NOISEREDUCTION_ENABLED FALSE)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/obs-filters-config.h.in"
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config/obs-filters-config.h")

set(obs-filters_config_HEADERS
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config/obs-filters-config.h")
include_directories(${LIBSPEEXDSP_INCLUDE_DIRS} ${LIBRNNOISE_INCLUDE_DIRS}
	"${CMAKE_BINARY_DIR}/plugins/obs-filters/config")

if(MSVC)
	set(obs-filters_PLATFORM_DEPS
		w32-pthreads)
endif()

set(obs-filters_SOURCES
	obs-filters.c
	color-correction-filter.c
	async-delay-filter.c
	gpu-delay.c
	crop-filter.c
	scale-filter.c
	scroll-filter.c
	chroma-key-filter.c
	color-key-filter.c
	color-grade-filter.c
	sharpness-filter.c
	gain-filter.c
	noise-gate-filter.c
	mask-filter.c
	invert-audio-polarity.c
	compressor-filter.c
	limiter-filter.c
	expander-filter.c
	luma-key-filter.c)

set(obs-filters_data_EFFECTS
	data/blend_add_filter.effect
	data/blend_mul_filter.effect
	data/blend_sub_filter.effect
	data/chroma_key_filter.effect
	data/chroma_key_filter_v2.effect
	data/color_correction_filter.effect
	data/color_grade_filter.effect
	data/color_key_filter.effect
	data/color_key_filter_v2.effect
	data/crop_filter.effect
	data/luma_key_filter.effect
	data/luma_key_filter_v2.effect
	data/mask_alpha_filter.effect
	data/mask_color_filter.effect
	data/sharpness.effect)

if(WIN32)
	set(MODULE_DESCRIPTION "OBS A/V Filters")
	configure_file(${CMAKE_SOURCE_DIR}/cmake/winrc/obs-module.rc.in obs-filters.rc)
	list(APPEND obs-filters_SOURCES
		obs-filters.rc)
endif()

add_library(obs-filters MODULE
	${rnnoise_SOURCES}
	${obs-filters_SOURCES}
	${obs-filters_config_HEADERS}
	${obs-filters_data_EFFECTS}
	${obs-filters_NOISEREDUCTION_SOURCES}
	${obs-filters_NOISEREDUCTION_HEADERS})
target_link_libraries(obs-filters
	libobs
	${obs-filters_PLATFORM_DEPS}
	${obs-filters_NOISEREDUCTION_LIBRARIES})
set_target_properties(obs-filters PROPERTIES FOLDER "plugins")

source_group("data\\Effect Files" FILES ${obs-filters_data_EFFECTS})

install_obs_plugin_with_data(obs-filters data)
