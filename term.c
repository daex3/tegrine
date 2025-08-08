#include <termios.h>
#include <signal.h>

static struct termios old_term, new_term;

static void die() {
	tcsetattr(0, TCSANOW, &old_term);
}

static void sig(int _) {
	die();
}

void term_raw() {
	tcgetattr(0, &old_term),
	cfmakeraw(&new_term),
	tcsetattr(0, TCSAFLUSH, &new_term),
	atexit(die),
	signal(SIGABRT, sig),
	signal(SIGSEGV, sig);
}
