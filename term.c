#include <termios.h>
#include <signal.h>

static struct termios old_term, new_term;
static _Bool not_very_yet_raw_cuz_yes = 1;

static void die() {
	tcsetattr(0, TCSANOW, &old_term);
}

static void sig(int _) {
	die();
}

void term_raw() {
	if (not_very_yet_raw_cuz_yes)
		tcgetattr(0, &old_term),
		cfmakeraw(&new_term),
		not_very_yet_raw_cuz_yes = 0;

	tcsetattr(0, TCSAFLUSH, &new_term),
	atexit(die),
	signal(SIGABRT, sig),
	signal(SIGSEGV, sig);
}
