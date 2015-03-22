#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("obs-filters", "en-US")

extern struct obs_source_info async_delay_filter;

bool obs_module_load(void)
{
	obs_register_source(&async_delay_filter);
	return true;
}
