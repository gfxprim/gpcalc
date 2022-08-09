//SPDX-License-Identifier: GPL-2.0-or-later
/*

   Copyright (C) 2007-2022 Cyril Hrubis <metan@ucw.cz>

 */

 /*

   Simple and fast expression evaluator. The expression is compiled into binary
   format first, then may be evaluated.

   Supported operators:

   + (both unary and binary)
   - (both unary and binary)
   * (binary)
   / (binary)
   ^ (binary)

   Expression could contain:

   * Floating point numbers.
   * Variables (as passed by array of struct expr_var)
   * Any correct sequence of brackets.

   Math functions:

   * Math functions abs, mod, rem, max, min
   * Exponential functions exp, exp2, log, log10
   * Power functions sqrt, cbrt, hypot, pow
   * Trigonometric functions sin, cos, tan, asin, acos, atan, atan2
   * Hyperbolic functions sinh, cosh, tanh, asinh, acosh, atanh
   * Error and gamma functions erf, erfc, lgamma, tgamma
   * Nearest integer floating point operations ceil, floor, trunc, round

   Variable named same as any supported fuction is possible but utterly
   confusing.

  */

#ifndef EXPR_H__
#define EXPR_H__

#include <stdint.h>

/*
 * NULL-terminated array of these is passed to expression compiler to define
 * variables.
 *
 * The variables are, when evaluating, referenced by a pointer.
 */
struct expr_var {
	const char *name;
	double val;
};

/*
 * Error returns an error + position
 */
struct expr_err {
	const char  *err;
	unsigned int pos;
};

struct expr_fn {
	union {
		void *ptr;
		double (*fn2)(double f1, double f2);
		double (*fn1)(double f);
	};
	/* set if angle is input/output */
	uint32_t a1_in:1;
	uint32_t a2_in:1;
	uint32_t a_out:1;
};

struct expr_elem {
	uint8_t type;
	union {
		double f;
		const struct expr_fn *fn;
		const double *var;
	};
};

enum expr_angle_unit {
	EXPR_DEGREES,
	EXPR_RADIANS,
	EXPR_GRADIANS,
};

struct expr_ctx {
	enum expr_angle_unit angle_unit;
};

struct expr {
	const struct expr_var *vars;
	unsigned int stack;
	struct expr_elem elems[];
};

/*
 * Compile expression into binary representation.
 *
 * The second parameter is an NULL-terminated array of variables to be used
 * for expression is evaluation.
 *
 * When an ill formed expression is encountered err is filled and NULL is returned.
 */
struct expr *expr_create(const char *expr,
                         const struct expr_var vars[],
                         struct expr_err *err);

/*
 * Free allocated memory.
 */
void expr_destroy(struct expr *self);

/*
 * Dumps list of variables and compiled expression into stdout.
 */
void expr_dump(struct expr *self);

/*
 * Evaluates compiled expression. Returns floating point number.
 */
double expr_eval(struct expr *self, struct expr_ctx *ctx);

#endif /* EXPR_H__ */
