#include "hal.h"

#define NR_KEYS 18

enum {KEY_STATE_EMPTY, KEY_STATE_WAIT_RELEASE, KEY_STATE_RELEASE, KEY_STATE_PRESS};

/* Only the following keys are used in NEMU-PAL. */
static const int keycode_array[] = {
	K_UP, K_DOWN, K_LEFT, K_RIGHT, K_ESCAPE,
	K_RETURN, K_SPACE, K_PAGEUP, K_PAGEDOWN, K_r,
	K_a, K_d, K_e, K_w, K_q,
	K_s, K_f, K_p
};

//static int key_state[NR_KEYS] = {0};

#define NR_STACK 100
static int keystack[NR_STACK] = {0};
static int statestack[NR_STACK] = {0};
static int pstack = 0;

void
keyboard_event(void) {
	/* TODO: Fetch the scancode and update the key states. */

	uint32_t scancode = in_byte(0x60);
	uint32_t updown = ((scancode >> 0x7) & 0x1);

	nemu_assert(pstack < NR_STACK - 1);
	keystack[pstack] = (scancode & 0x7f);
	statestack[pstack] = updown;
	pstack ++;
}

/*
static inline int
get_keycode(int index) {
	assert(index >= 0 && index < NR_KEYS);
	return keycode_array[index];
}

static inline int
query_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	return key_state[index];
}

static inline void
release_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	key_state[index] = KEY_STATE_WAIT_RELEASE;
}

static inline void
clear_key(int index) {
	assert(index >= 0 && index < NR_KEYS);
	key_state[index] = KEY_STATE_EMPTY;
}
*/
bool 
process_keys(void (*key_press_callback)(int), void (*key_release_callback)(int)) {
	cli();
	/* TODO: Traverse the key states. Find a key just pressed or released.
	 * If a pressed key is found, call ``key_press_callback'' with the keycode.
	 * If a released key is found, call ``key_release_callback'' with the keycode.
	 * If any such key is found, the function return true.
	 * If no such key is found, the function return false.
	 * Remember to enable interrupts before returning from the function.
	 */
	bool ret = false;
	for(;pstack > 0; pstack --)
	{
		if(statestack[pstack] == 0x0)
		{
			key_press_callback(keystack[pstack]);
			ret = true;
		}
		else
		{
			key_release_callback(keystack[pstack]);
			ret = true;
		}
	}
	sti();
	return ret;
}
