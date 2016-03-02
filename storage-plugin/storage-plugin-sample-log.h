#ifndef __STORAGE_PLUGIN_SAMPLE_LOG_H__
#define __STORAGE_PLUGIN_SAMPLE_LOG_H__

/**
 *  HOW TO USE IT:
 *  First you need to set platform logging on the device:
 *
 *    # dlogctrl set platformlog 1
 *
 *  After reboot you are able to see logs from this application, when you launch dlogutil with a proper filter e.g.:
 *
 *    # dlogutil PUSH_PLUGIN_SPP:D
 *
 *  You may use different logging levels as: D (debug), I (info), W (warning), E (error) or F (fatal).
 *  Higher level messages are included by default e.g. dlogutil PUSH_PLUGIN_SPP:W prints warnings but also errors and fatal messages.
 */

#include <unistd.h>
#include <linux/unistd.h>

/* These defines must be located before #include <dlog.h> */
#define TIZEN_ENGINEER_MODE
// TODO: Investigate why this macro is defined somewhere else
#ifndef TIZEN_DEBUG_ENABLE
#define TIZEN_DEBUG_ENABLE
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif
/* Literal to filter logs from dlogutil */
#define LOG_TAG "STORAGE_PLUGIN_SAMPLE"

#include <dlog.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	/**
	 *  Colors of font
	 */
#define FONT_COLOR_RESET      "\033[0m"
#define FONT_COLOR_BLACK      "\033[30m"             /* Black */
#define FONT_COLOR_RED        "\033[31m"             /* Red */
#define FONT_COLOR_GREEN      "\033[32m"             /* Green */
#define FONT_COLOR_YELLOW     "\033[33m"             /* Yellow */
#define FONT_COLOR_BLUE       "\033[34m"             /* Blue */
#define FONT_COLOR_PURPLE     "\033[35m"             /* Purple */
#define FONT_COLOR_CYAN       "\033[36m"             /* Cyan */
#define FONT_COLOR_WHITE      "\033[37m"             /* White */
#define FONT_COLOR_BOLDBLACK  "\033[1m\033[30m"      /* Bold Black */
#define FONT_COLOR_BOLDRED    "\033[1m\033[31m"      /* Bold Red */
#define FONT_COLOR_BOLDGREEN  "\033[1m\033[32m"      /* Bold Green */
#define FONT_COLOR_BOLDYELLOW "\033[1m\033[33m"      /* Bold Yellow */
#define FONT_COLOR_BOLDBLUE   "\033[1m\033[34m"      /* Bold Blue */
#define FONT_COLOR_BOLDPURPLE "\033[1m\033[35m"      /* Bold Purple */
#define FONT_COLOR_BOLDCYAN   "\033[1m\033[36m"      /* Bold Cyan */
#define FONT_COLOR_BOLDWHITE  "\033[1m\033[37m"      /* Bold White */

	/**
	 *  Gets thread ID
	 */
#define storage_plugin_sample_gettid() syscall(__NR_gettid)

	/**
	 *  @brief Macro for returning value if expression is satisfied
	 *  @param[in]  expr Expression to be checked
	 *  @param[out] val  Value to be returned when expression is true
	 */
#define storage_plugin_sample_retv_if(expr, val) do { \
	if(expr) { \
		LOGE(FONT_COLOR_PURPLE"[%d]"FONT_COLOR_RESET, storage_plugin_sample_gettid());    \
		return (val); \
	} \
} while (0)

/**
 * @brief Prints debug messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_debug(fmt, arg...) do { \
	LOGD(FONT_COLOR_GREEN"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid(), ##arg);     \
} while (0)

/**
 * @brief Prints info messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_info(fmt, arg...) do { \
	LOGI(FONT_COLOR_BLUE"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid() ,##arg);     \
} while (0)

/**
 * @brief Prints warning messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_warning(fmt, arg...) do { \
	LOGW(FONT_COLOR_YELLOW"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid(), ##arg);     \
} while (0)

/**
 * @brief Prints error messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_error(fmt, arg...) do { \
	LOGE(FONT_COLOR_RED"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid(), ##arg);     \
} while (0)

/**
 * @brief Prints fatal messages
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_fatal(fmt, arg...) do { \
	LOGF(FONT_COLOR_BOLDRED"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid(), ##arg);     \
} while (0)

/**
 * @brief Prints debug message on entry to particular function
 * @param[in]  fmt  Format of data to be displayed
 * @param[in]  args Arguments to be displayed
 */
#define storage_plugin_sample_debug_func(fmt, arg...) do { \
	LOGD(FONT_COLOR_CYAN"[%d]"fmt""FONT_COLOR_RESET, storage_plugin_sample_gettid(), ##arg);     \
} while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __STORAGE_PLUGIN_SAMPLE_LOG_H__ */
