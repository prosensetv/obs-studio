#include <obs-module.h>
#include <graphics/image-file.h>
#include <util/dstr.h>

#define SETTING_HFOV_AMOUNT            "hfov_amount"
#define SETTING_VFOV_AMOUNT            "vfov_amount"


#define TEXT_HFOV                    obs_module_text("Horizontal FOV")
#define TEXT_VFOV                    obs_module_text("Vertical FOV")

struct polar_filter_data {
	obs_source_t                   *context;
	gs_effect_t                    *effect;
	gs_image_file_t                image;

	char                           *file;
	float                          hfov;
	float                          vfov;
};

static const char *polar_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("PolarFilter");
}

static void polar_filter_update(void *data, obs_data_t *settings)
{
	struct polar_filter_data *filter = data;

	double hfov_amount = obs_data_get_double(settings, SETTING_HFOV_AMOUNT);
	double vfov_amount = obs_data_get_double(settings, SETTING_VFOV_AMOUNT);

	obs_enter_graphics();

	filter->hfov = (float)hfov_amount;
	filter->vfov = (float)vfov_amount;

	char *effect_path = obs_module_file("polar_filter.effect");
	gs_effect_destroy(filter->effect);
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	bfree(effect_path);

	obs_leave_graphics();
}

static void polar_filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, SETTING_HFOV_AMOUNT, 220);
	obs_data_set_default_double(settings, SETTING_VFOV_AMOUNT, 160);
}

static obs_properties_t *polar_filter_properties(void *data)
{
	struct polar_filter_data *s = data;

	obs_properties_t *props = obs_properties_create();

	obs_properties_add_float_slider(props, SETTING_VFOV_AMOUNT, TEXT_VFOV, 0, 360, 1);
	obs_properties_add_float_slider(props, SETTING_HFOV_AMOUNT, TEXT_HFOV, 0, 360, 1);

	UNUSED_PARAMETER(data);
	return props;
}

static void *polar_filter_create( obs_data_t *settings, obs_source_t *context)
{
	struct polar_filter_data *filter = bzalloc(sizeof(struct polar_filter_data));
	filter->context = context;

	obs_source_update(context, settings);
	return filter;
}

static void polar_filter_destroy(void *data)
{
	struct polar_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	gs_image_file_free(&filter->image);
	obs_leave_graphics();

	bfree(filter->file);
	bfree(filter);
}

static void polar_filter_render(void *data, gs_effect_t *effect)
{
	struct polar_filter_data *filter = data;
	obs_source_t *target = obs_filter_get_target(filter->context);
	gs_eparam_t *param;

	if (!target || !filter->effect) {
		obs_source_skip_video_filter(filter->context);
		return;
	}

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA, OBS_ALLOW_DIRECT_RENDERING))
		return;

	param = gs_effect_get_param_by_name(filter->effect, "hfov_amount");
	gs_effect_set_float(param, filter->hfov*M_PI/180);

	param = gs_effect_get_param_by_name(filter->effect, "vfov_amount");
	gs_effect_set_float(param, filter->vfov*M_PI/180);


	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

struct obs_source_info polar_filter = {
	.id                            = "polar_filter",
	.type                          = OBS_SOURCE_TYPE_FILTER,
	.output_flags                  = OBS_SOURCE_VIDEO,
	.get_name                      = polar_filter_get_name,
	.create                        = polar_filter_create,
	.destroy                       = polar_filter_destroy,
	.update                        = polar_filter_update,
	.get_defaults                  = polar_filter_defaults,
	.get_properties                = polar_filter_properties,
	.video_render                  = polar_filter_render
};
