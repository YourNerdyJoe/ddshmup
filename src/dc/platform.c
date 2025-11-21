#include "../platform.h"

#include <dc/pvr.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>

static pvr_poly_hdr_t poly;

void platform_init(void) {
	pvr_init(&(pvr_init_params_t){
		.opb_sizes = { PVR_BINSIZE_16, },
		.vertex_buf_size = 64 * 1024,
	});

	pvr_set_bg_color(0.0f, 0.5f, 0.0f);

	pvr_poly_cxt_t cxt;
	pvr_poly_cxt_col(&cxt, PVR_LIST_OP_POLY);
	pvr_poly_compile(&poly, &cxt);
}

void platform_scene_begin(void) {
	pvr_wait_ready();
	pvr_scene_begin();

	//testing
	pvr_list_begin(PVR_LIST_OP_POLY);

	pvr_prim(&poly, sizeof(poly));

	pvr_vertex_t vert = {
		.flags = PVR_CMD_VERTEX,
		.x = 0.f,
		.y = 64.f,
		.z = 0.1f,
		.argb = 0xFFFFFFFF,
	};
	pvr_prim(&vert, sizeof(vert));

	vert.y = 0.f;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.x = 64.f;
	vert.y = 64.f;
	pvr_prim(&vert, sizeof(vert));
}

void platform_scene_end(void) {
	pvr_list_finish();
	pvr_scene_finish();
}

void platform_shutdown(void) {
	pvr_shutdown();
}

bool platform_input_poll(void) {
	return true;
}

int main(int argc, char* argv[]) {
	platform_init();

	while(platform_input_poll()) {
		platform_scene_begin();
		platform_scene_end();
	}

	platform_shutdown();
	return 0;
}
