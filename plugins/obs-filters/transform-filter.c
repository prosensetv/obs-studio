#include <obs-module.h>
#include <graphics/image-file.h>
#include <util/dstr.h>

struct lut_filter_data {
	obs_source_t                   *context;
	gs_effect_t                    *effect;

	struct vec3					   image_pos;
	struct vec3					   image_normal;
	float						   fov_h;
	float						   fov_v;
};

static const char *transform_filter_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("Transform Filter");
}

static void transform_filter_update(void *data, obs_data_t *settings)
{
	struct lut_filter_data *filter = data;

	double pos_x = obs_data_get_double(settings, "pos_x");
	double pos_y = obs_data_get_double(settings, "pos_y");
	double pos_z = obs_data_get_double(settings, "pos_z");

	double normal_x = obs_data_get_double(settings, "normal_x");
	double normal_y = obs_data_get_double(settings, "normal_y");
	double normal_z = obs_data_get_double(settings, "normal_z");

	double fov_h = obs_data_get_double(settings, "fov_h");
	double fov_v = obs_data_get_double(settings, "fov_v");

	obs_enter_graphics();

	
	filter->fov_h = (float)fov_h;
	filter->fov_v = (float)fov_v;

	vec3_set(&filter->image_pos, (float)pos_x, (float)pos_y, (float)pos_z);
	vec3_set(&filter->image_normal, (float)normal_x, (float)normal_y, (float)normal_z);

	char *effect_path = obs_module_file("transform_filter.effect");
	gs_effect_destroy(filter->effect);
	filter->effect = gs_effect_create_from_file(effect_path, NULL);
	bfree(effect_path);

	obs_leave_graphics();
}

static void transform_filter_defaults(obs_data_t *settings)
{
	obs_data_set_default_double(settings, "pos_x", 1);
	obs_data_set_default_double(settings, "pos_y", 0);
	obs_data_set_default_double(settings, "pos_z", 0);

	obs_data_set_default_double(settings, "normal_x", 0);
	obs_data_set_default_double(settings, "normal_y", 1);
	obs_data_set_default_double(settings, "normal_z", 0);

	obs_data_set_default_double(settings, "fov_h", 3.1415);
	obs_data_set_default_double(settings, "fov_v", 3.1415);

}

static obs_properties_t *transform_filter_properties(void *data)
{
	struct lut_filter_data *s = data;
	struct dstr path = {0};
	const char *slash;

	obs_properties_t *props = obs_properties_create();

	obs_properties_add_float_slider(props, "pos_x", "Pos X", -100, 100, 0.01);
	obs_properties_add_float_slider(props, "pos_y", "Pos Y", -100, 100, 0.01);
	obs_properties_add_float_slider(props, "pos_z", "Pos Z", -100, 100, 0.01);

	obs_properties_add_float_slider(props, "normal_x", "Normal X", -100, 100, 0.01);
	obs_properties_add_float_slider(props, "normal_y", "Normal Y", -100, 100, 0.01);
	obs_properties_add_float_slider(props, "normal_z", "Normal Z", -100, 100, 0.01);

	obs_properties_add_float_slider(props, "fov_h", "Fov H", 0, M_PI*2.0, 0.01);
	obs_properties_add_float_slider(props, "fov_v", "Fov V", 0, M_PI*2.0, 0.01);



	UNUSED_PARAMETER(data);
	return props;
}

static void *transform_filter_create(
		obs_data_t *settings, obs_source_t *context)
{
	struct lut_filter_data *filter =
		bzalloc(sizeof(struct lut_filter_data));
	filter->context = context;

	obs_source_update(context, settings);
	return filter;
}

static void transform_filter_destroy(void *data)
{
	struct lut_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);

	obs_leave_graphics();

	bfree(filter);
}

static void transform_filter_render(void *data, gs_effect_t *effect)
{
	struct lut_filter_data *filter = data;
	obs_source_t *target = obs_filter_get_target(filter->context);
	gs_eparam_t *param;

	if (!target || !filter->effect) {
		obs_source_skip_video_filter(filter->context);
		return;
	}

	if (!obs_source_process_filter_begin(filter->context, GS_RGBA,
				OBS_ALLOW_DIRECT_RENDERING))
		return;

	param = gs_effect_get_param_by_name(filter->effect, "image_pos");
	gs_effect_set_vec3(param, &filter->image_pos);
	param = gs_effect_get_param_by_name(filter->effect, "image_normal");
	gs_effect_set_vec3(param, &filter->image_normal);

	param = gs_effect_get_param_by_name(filter->effect, "fov_h");
	gs_effect_set_float(param, filter->fov_h);
	param = gs_effect_get_param_by_name(filter->effect, "fov_v");
	gs_effect_set_float(param, filter->fov_v);



	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

	UNUSED_PARAMETER(effect);
}

struct obs_source_info transform_filter = {
	.id                            = "transform_filter",
	.type                          = OBS_SOURCE_TYPE_FILTER,
	.output_flags                  = OBS_SOURCE_VIDEO,
	.get_name                      = transform_filter_get_name,
	.create                        = transform_filter_create,
	.destroy                       = transform_filter_destroy,
	.update                        = transform_filter_update,
	.get_defaults                  = transform_filter_defaults,
	.get_properties                = transform_filter_properties,
	.video_render                  = transform_filter_render
};
