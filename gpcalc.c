//SPDX-License-Identifier: GPL-2.1-or-later

/*

    Copyright (C) 2007-2021 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "expr.h"

static gp_htable *uids;

static gp_widget *edit;
static gp_widget *layout_switch;

static const struct expr_var vars[] = {
	{. name = "pi", .val = M_PI},
	{. name = "e", .val = M_E},
	{}
};

static struct expr_ctx ctx;

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

	expr = expr_create(gp_widget_tbox_text(edit), vars, &err);
	if (!expr) {
		gp_widget_tbox_printf(edit, "%i:%s", err.pos, err.err);
		return 0;
	}

	gp_widget_tbox_printf(edit, "%.16g", expr_eval(expr, &ctx));

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

	const char *label = ev->self->button->label;

	if (!strcmp(label, "\u00d7"))
		label = "*";

	if (!strcmp(label, "\u00f7"))
		label = "/";

	gp_widget_tbox_ins(edit, 0, GP_SEEK_END, label);

	return 1;
}

int prev_layout(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_switch_move(layout_switch, -1);
	return 0;
}

int next_layout(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_switch_move(layout_switch, 1);
	return 0;
}

int set_angle_unit(gp_widget_event *ev)
{
	const char *angle_unit = gp_widget_choice_sel_name_get(ev->self);

	if (!strcmp(angle_unit, "degrees"))
		ctx.angle_unit = EXPR_DEGREES;
	else if (!strcmp(angle_unit, "radians"))
		ctx.angle_unit = EXPR_RADIANS;
	else if (!strcmp(angle_unit, "gradians"))
		ctx.angle_unit = EXPR_GRADIANS;
	else
		GP_WARN("Invalid angle unit '%s'", angle_unit);

	return 0;
}

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("gpcalc", &uids);

	edit = gp_widget_by_uid(uids, "edit", GP_WIDGET_TBOX);
	layout_switch = gp_widget_by_uid(uids, "layout_switch", GP_WIDGET_SWITCH);

	gp_widgets_main_loop(layout, "gpcalc", NULL, argc, argv);

	return 0;
}
