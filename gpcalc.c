//SPDX-License-Identifier: GPL-2.1-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "expr.h"

static void *uids;

gp_widget *edit;

static const struct expr_var vars[] = {
	{. name = "pi", .val = M_PI},
	{. name = "e", .val = M_E},
	{}
};

int clear(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_clear(edit);

	return 0;
}

int do_backspace(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_del(edit, -1, GP_SEEK_CUR, 1);

	return 0;
}

int do_eq(gp_widget_event *ev)
{
	struct expr *expr;
	struct expr_err err;

	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	expr = expr_create(gp_widget_tbox_str(edit), vars, &err);
	if (!expr) {
		gp_widget_tbox_printf(edit, "%i:%s", err.pos, err.err);
		return 0;
	}

	gp_widget_tbox_printf(edit, "%.16g", expr_eval(expr));

	expr_destroy(expr);

	return 1;
}

int edit_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	if (ev->sub_type != GP_WIDGET_TBOX_TRIGGER)
		return 0;

	return do_eq(ev);
}


int do_append(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_ins(edit, 0, GP_SEEK_END, ev->self->button->label);

	return 1;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("gpcalc", &uids);
	edit = gp_widget_by_uid(uids, "edit", GP_WIDGET_TBOX);

	gp_widgets_main_loop(layout, "gpcalc", NULL, argc, argv);

	return 0;
}
