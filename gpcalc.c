//SPDX-License-Identifier: GPL-2.0-or-later

/*

    Copyright (C) 2007-2022 Cyril Hrubis <metan@ucw.cz>

 */

#include <string.h>
#include <widgets/gp_widgets.h>
#include "expr.h"

static gp_htable *uids;

static gp_widget *edit;
static gp_widget *layout_switch;

static double last_val;

static struct expr_var vars[] = {
	{.name = "A"},
	{.name = "B"},
	{.name = "C"},
	{.name = "D"},
	{.name = "E"},
	{.name = "F"},
	{.name = "G"},
	{.name = "H"},
	{.name = "pi", .val = M_PI},
	{.name = "e", .val = M_E},
	{}
};

static struct expr_ctx ctx;

int var_store(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	const char *label = gp_widget_button_label_get(ev->self);

	if (label[0] < 'A' || label[0] > 'H')
		return 0;

	vars[label[0]-'A'].val = last_val;

	return 0;
}

int clear(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_clear(edit);

	last_val = 0;

	return 0;
}

int do_backspace(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_del(edit, -1, GP_SEEK_CUR, 1);

	return 0;
}

static void close_parens(void)
{
	const char *buf = gp_widget_tbox_text(edit);
	int pars = 0;
	uint32_t ch;

	while ((ch = gp_utf8_next(&buf))) {
		switch (ch) {
		case '(':
			pars++;
		break;
		case ')':
			pars--;
		break;
		}
	}

	while (pars-- > 0)
		gp_widget_tbox_append(edit, ")");
}

static int eval(void)
{
	struct expr *expr;
	struct expr_err err;

	close_parens();

	expr = expr_create(gp_widget_tbox_text(edit), vars, &err);
	if (!expr) {
		gp_widget_tbox_printf(edit, "%i:%s", err.pos, err.err);
		gp_widget_tbox_clear_on_input(edit);
		return 0;
	}

	last_val = expr_eval(expr, &ctx);

	gp_widget_tbox_printf(edit, "%.16g", last_val);

	expr_destroy(expr);

	return 1;
}

int do_eq(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	eval();

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

static char last_chr(const char *str)
{
	char prev = 0;

	while (*str)
		prev = *(str++);

	return prev;
}

static int is_func(const char *str)
{
	return last_chr(str) == '(';
}

static int is_op_fn(int ch)
{
	switch (ch) {
	case '*':
	case '+':
	case '-':
	case '/':
	case '^':
	case '(':
		return 1;
	default:
		return 0;
	}
}

static int op_fn_before_cur_pos(gp_widget *tbox)
{
	gp_utf8_pos cur_pos = gp_widget_tbox_cursor_get(tbox);

	int ch = gp_utf8_pos_prev(gp_widget_tbox_text(tbox), &cur_pos);

	return is_op_fn(ch);
}

int do_append(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	const char *label = gp_widget_button_label_get(ev->self);

	if (!strcmp(label, "\u00d7"))
		label = "*";

	if (!strcmp(label, "\u00f7"))
		label = "/";

	int ins_whence = GP_SEEK_CUR;
	if (is_func(label) && !op_fn_before_cur_pos(edit))
		ins_whence = GP_SEEK_SET;

	gp_widget_tbox_ins(edit, 0, ins_whence, label);

	return 1;
}

int cursor_home(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_cursor_set(edit, 0, GP_SEEK_SET);
	return 0;
}

int cursor_end(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_tbox_cursor_set(edit, 0, GP_SEEK_END);
	return 0;
}

int prev_layout(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_layout_switch_move(layout_switch, -1);
	return 0;
}

int next_layout(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_WIDGET)
		return 0;

	gp_widget_layout_switch_move(layout_switch, 1);
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

static int app_on_event(gp_widget_event *ev)
{
	if (ev->type != GP_WIDGET_EVENT_INPUT)
		return 0;

	if (ev->input_ev->type == GP_EV_UTF &&
	    ev->input_ev->utf.ch == '=') {
		eval();
		return 1;
	}

	return gp_widget_input_inject(edit, ev);
}

gp_app_info app_info = {
	.name = "gpcalc",
	.desc = "A scientific calculator",
	.version = "1.0",
	.license = "GPL-2.0-or-later",
	.url = "http://github.com/gfxprim/gpcalc",
	.authors = (gp_app_info_author []) {
		{.name = "Cyril Hrubis", .email = "metan@ucw.cz", .years = "2007-2022"},
		{}
	}
};

int main(int argc, char *argv[])
{
	gp_widget *layout = gp_app_layout_load("gpcalc", &uids);

	edit = gp_widget_by_uid(uids, "edit", GP_WIDGET_TBOX);
	layout_switch = gp_widget_by_uid(uids, "layout_switch", GP_WIDGET_SWITCH);

	gp_app_event_unmask(GP_WIDGET_EVENT_INPUT);
	gp_app_on_event_set(app_on_event);

	gp_widgets_main_loop(layout, NULL, argc, argv);

	return 0;
}
