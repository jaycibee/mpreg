/**
Copyright (C) 2023 Helena Jäger

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>. 
*/

#ifndef _MPREG_H
#define _MPREG_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define MAX_STACK_SIZE 256

struct state {
    struct state *m_next;
    struct state *c_next;
	struct state *n_next;
    char trans1_c;
    struct state* trans1;
    char trans2_c;
    struct state* trans2;
	bool accept;
};

typedef struct {
	struct state* states;
	struct state* start;
} mpreg_t;

bool mpreg_compile(mpreg_t*, char*);
bool mpreg_match(mpreg_t*, char*);
void mpreg_free(mpreg_t*);

struct parse_state {
	char *in;
	char *out;
};

static inline bool parse_infix(struct parse_state*);

static inline bool parse_infix_3(struct parse_state* state)
{
	if (*(state->in) == '(') {
		(state->in)++;
		if (!parse_infix(state)) return false;
		if (*(state->in)++ != ')') return false;
	} else {
		switch (*(state->in)) {
			case '.':
			case '*':
			case '+':
			case '|':
			case '(':
			case ')':
			case '\0':
				return false;
			default:
				*(state->out)++ = *(state->in)++;
				break;
		}
	}
	return true;
}

static inline bool parse_infix_2(struct parse_state* state)
{
	if (!parse_infix_3(state)) return false;
	if (*(state->in) == '*' || *(state->in) == '+') {
		*(state->out)++ = *(state->in)++;
	}
	return true;
}

static inline bool parse_infix_1(struct parse_state* state)
{
	if (!parse_infix_2(state)) return false;
	while (true) {
		if (*(state->in) != '.') {
			break;
		} else {
			(state->in)++;
		}
		if (!parse_infix_2(state)) return false;
		*(state->out)++ = '.';
	}
	return true;
}

static inline bool parse_infix(struct parse_state* state)
{
	if (!parse_infix_1(state)) return false;
	while (true) {
		if (*(state->in) != '|') {
			break;
		} else {
			(state->in)++;
		}
		if (!parse_infix_1(state)) return false;
		*(state->out)++ = '|';
	}
	return true;
}

char* infix_to_postfix(char* in)
{
	struct parse_state st;
	st.in = in;
	char* out = malloc(strlen(st.in) + 1);
	memset(out, 0, strlen(st.in) + 1);
	st.out = out;
	if (parse_infix(&st) && *(st.in) == '\0') {
		return out;
	} else {
		free(out);
		return NULL;
	}
}

static inline struct state* regex_add_state(mpreg_t* regex, char t1c, struct state* t1, char t2c, struct state* t2, bool accept)
{
	struct state* new_state = malloc(sizeof(struct state));
	new_state->trans1_c = t1c;
	new_state->trans1 = t1;
	new_state->trans2_c = t2c;
	new_state->trans2 = t2;
	new_state->accept = accept;
	new_state->m_next = regex->states;
	regex->states = new_state;
	return new_state;
}

static inline void regex_compile_plus(mpreg_t* regex, struct state** stack, size_t* stack_top)
{
	struct state* end = stack[--(*stack_top)];
	struct state* start = stack[--(*stack_top)];
	struct state* new2 = regex_add_state(regex, '\0', NULL, '\0', NULL, true);
	struct state* new1 = regex_add_state(regex, '\0', start, '\0', NULL, false);
	regex->start = new1;
	end->trans1_c = '\0';
	end->trans1 = new2;
	end->trans2_c = '\0';
	end->trans2 = start;
	end->accept = false;
	stack[(*stack_top)++] = new1;
	stack[(*stack_top)++] = new2;
}

static inline void regex_compile_star(mpreg_t* regex, struct state** stack, size_t* stack_top)
{
	struct state* end = stack[--(*stack_top)];
	struct state* start = stack[--(*stack_top)];
	struct state* new2 = regex_add_state(regex, '\0', NULL, '\0', NULL, true);
	struct state* new1 = regex_add_state(regex, '\0', start, '\0', new2, false);
	regex->start = new1;
	end->trans1_c = '\0';
	end->trans1 = new2;
	end->trans2_c = '\0';
	end->trans2 = start;
	end->accept = false;
	stack[(*stack_top)++] = new1;
	stack[(*stack_top)++] = new2;
}

static inline void regex_compile_union(mpreg_t* regex, struct state** stack, size_t* stack_top)
{
	struct state* end2 = stack[--(*stack_top)];
	struct state* start2 = stack[--(*stack_top)];
	struct state* end1 = stack[--(*stack_top)];
	struct state* start1 = stack[--(*stack_top)];
	struct state* new2 = regex_add_state(regex, '\0', NULL, '\0', NULL, true);
	struct state* new1 = regex_add_state(regex, '\0', start1, '\0', start2, false);
	regex->start = new1;
	end1->trans1_c = '\0';
	end1->trans1 = new2;
	end1->accept = false;
	end2->trans1_c = '\0';
	end2->trans1 = new2;
	end2->accept = false;
	stack[(*stack_top)++] = new1;
	stack[(*stack_top)++] = new2;
}

static inline void regex_compile_concatenation(mpreg_t* regex, struct state** stack, size_t* stack_top)
{
	struct state* end2 = stack[--(*stack_top)];
	struct state* start2 = stack[--(*stack_top)];
	struct state* end1 = stack[--(*stack_top)];
	struct state* start1 = stack[--(*stack_top)];
	regex->start = start1;
	end1->accept = false;
	end1->trans1_c = '\0';
	end1->trans1 = start2;
	stack[(*stack_top)++] = start1;
	stack[(*stack_top)++] = end2;
}

static inline void regex_compile_literal(mpreg_t* regex, struct state** stack, size_t* stack_top, char c)
{
	struct state* new2 = regex_add_state(regex, '\0', NULL, '\0', NULL, true);
	struct state* new1 = regex_add_state(regex, c, new2, '\0', NULL, false);
	regex->start = new1;
	stack[(*stack_top)++] = new1;
	stack[(*stack_top)++] = new2;
}

bool mpreg_compile(mpreg_t* regex, char* pattern)
{
	char* postfix = infix_to_postfix(pattern);
	if (postfix == NULL) return false;
	struct state* stack[MAX_STACK_SIZE];
	size_t stack_top = 0;
	for (char* c = postfix; *c != '\0'; c++) {
		switch (*c) {
			case '+':
				regex_compile_plus(regex, stack, &stack_top);
				break;
			case '*':
				regex_compile_star(regex, stack, &stack_top);
				break;
			case '|':
				regex_compile_union(regex, stack, &stack_top);
				break;
			case '.':
				regex_compile_concatenation(regex, stack, &stack_top);
				break;
			default:
				regex_compile_literal(regex, stack, &stack_top, *c);
				break;
		}
	}
	free(postfix);
	return true;
}

static inline bool regex_add_next_state(struct state** list, struct state* s)
{
	for (struct state* c = *list; c != NULL; c = c->n_next) {
		if (c == s) return false;
	}
	s->n_next = *list;
	*list = s;
	return true;
}

static inline bool regex_add_current_state(struct state** list, struct state* s)
{
	for (struct state* c = *list; c != NULL; c = c->c_next) {
		if (c == s) return false;
	}
	s->c_next = *list;
	*list = s;
	return true;
}


static inline void regex_copy_next_to_current(struct state** next, struct state** current)
{
	*current = *next;
	for (struct state* n = *next; n != NULL; n = n->n_next) {
		n->c_next = n->n_next;
	}
}

bool mpreg_match(mpreg_t* regex, char* string)
{
	struct state* current = regex->start;
	current->c_next = NULL;
	bool change;
	do {
		change = false;
		for (struct state* s = current; s != NULL; s = s->c_next) {
			if (s->trans1 != NULL && s->trans1_c == '\0') {
				change |= regex_add_current_state(&current, s->trans1);
			}
			if (s->trans2 != NULL && s->trans2_c == '\0') {
				change |= regex_add_current_state(&current, s->trans2);
			}
		}
	} while (change);
	for (char* c = string; *c != '\0'; c++) {
		struct state* next = NULL;
		for (struct state* s = current; s != NULL; s = s->c_next) {
			if (s->trans1 != NULL && s->trans1_c == *c) {
				regex_add_next_state(&next, s->trans1);
			}
			if (s->trans2 != NULL && s->trans2_c == *c) {
				regex_add_next_state(&next, s->trans2);
			}
		}
		bool change;
		do {
			change = false;
			for (struct state* s = next; s != NULL; s = s->n_next) {
				if (s->trans1 != NULL && s->trans1_c == '\0') {
					change |= regex_add_next_state(&next, s->trans1);
				}
				if (s->trans2 != NULL && s->trans2_c == '\0') {
					change |= regex_add_next_state(&next, s->trans2);
				}
			}
		} while (change);
		regex_copy_next_to_current(&next, &current);
	}
	bool accept = false;
	for (struct state* s = current; s != NULL; s = s->c_next) {
		accept |= s->accept;
	}
	return accept;
}

void mpreg_free(mpreg_t* regex)
{
	for (struct state* s = regex->states; s != NULL;) {
		struct state* next = s->m_next;
		free(s);
		s = next;
	}
}

#endif
