//SPDX-License-Identifier: GPL-2.1-or-later

/*

    Copyright (C) 2007-2020 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "expr.h"

static void *uids;

gp_widget *edit;

int clear(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_ACTION)
		return 0;

	gp_widget_tbox_clear(edit);

	return 0;
}

int do_eq(gp_widget_event *ev)
{
	struct expr *expr;
	struct expr_err err;

	if (ev->type != GP_WIDGET_EVENT_ACTION)
		return 0;

	expr = expr_create(gp_widget_tbox_str(edit), NULL, &err);
	if (!expr) {
		gp_widget_tbox_printf(edit, "%i:%s", err.pos, err.err);
		return 0;
	}

	gp_widget_tbox_printf(edit, "%.16g", expr_eval(expr));

	expr_destroy(expr);

	return 1;
}

int do_append(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_ACTION)
		return 0;

	gp_widget_tbox_ins(edit, 0, GP_SEEK_END, ev->self->btn->label);

	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("gpcalc", &uids);
	edit = gp_widget_by_uid(uids, "edit", GP_WIDGET_TBOX);

	gp_widgets_main_loop(layout, "gpcalc", NULL, argc, argv);

	return 0;
}
