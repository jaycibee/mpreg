# mpreg

mpreg - **m**inimal **p**arser for **reg**ular expressions

a c99 header-only library for parsing (limited) regular expressions

## regex format

in order of precedence:
* a non-reserved character is a regex
* if `a` is a regex, `a*` is a regex representing the kleene closure of `a`
* if `a` is a regex, `a+` is a regex representing the kleene star closure of `a`
* if `a` and `b` are regexes, `a.b` represents the concatenation of `a` and `b`
* if `a` and `b` are regexes, `a|b` represents the union of `a` and `b`

## usage

mpreg exports one struct `mpreg_t` and three functions
```c
/* used to compile a pattern into a regular expression. returns true if successful, false if pattern is invalid. */
bool mpreg_compile(mpreg_t*, char*);
/* used to match a string against a compiled regular expression. returns true on match, false if otherwise. */
bool mpreg_match(mpreg_t*, char*);
/* used to deallocate memory used by a compiled regex. regex cannot be used after this. */
void mpreg_free(mpreg_t*);
```