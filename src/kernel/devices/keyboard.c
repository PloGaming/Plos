#include <devices/keyboard.h>
#include <io/io.h>
#include <strings/string.h>

//! scan code corrente
static unsigned char _scancode;

//! lock keys
static bool _numlock, _scrolllock, _capslock;

//! shift, alt, and ctrl keys state
static bool _shift, _alt, _ctrl;

//! Impostato se BAT ha fallito (BAT é il self test della tastiera)
static bool _kkybrd_bat_res = false;

//! Impostato se la diagnosi ha rilevato un errore
static bool _kkybrd_diag_res = false;

//! Impostato se la tastiera dovrebbe reinviare un tasto
static bool _kkybrd_resend_res = false;

//! Impostato se la tastiera é disabilitata
static bool _kkybrd_disable = false;

// XT scand code set
static int _kkybrd_scancode_std[] = {

	//! key			scancode
	KEY_UNKNOWN,	  // 0
	KEY_ESCAPE,		  // 1
	KEY_1,			  // 2
	KEY_2,			  // 3
	KEY_3,			  // 4
	KEY_4,			  // 5
	KEY_5,			  // 6
	KEY_6,			  // 7
	KEY_7,			  // 8
	KEY_8,			  // 9
	KEY_9,			  // 0xa
	KEY_0,			  // 0xb
	KEY_MINUS,		  // 0xc
	KEY_EQUAL,		  // 0xd
	KEY_BACKSPACE,	  // 0xe
	KEY_TAB,		  // 0xf
	KEY_Q,			  // 0x10
	KEY_W,			  // 0x11
	KEY_E,			  // 0x12
	KEY_R,			  // 0x13
	KEY_T,			  // 0x14
	KEY_Y,			  // 0x15
	KEY_U,			  // 0x16
	KEY_I,			  // 0x17
	KEY_O,			  // 0x18
	KEY_P,			  // 0x19
	KEY_LEFTBRACKET,  // 0x1a
	KEY_RIGHTBRACKET, // 0x1b
	KEY_RETURN,		  // 0x1c
	KEY_LCTRL,		  // 0x1d
	KEY_A,			  // 0x1e
	KEY_S,			  // 0x1f
	KEY_D,			  // 0x20
	KEY_F,			  // 0x21
	KEY_G,			  // 0x22
	KEY_H,			  // 0x23
	KEY_J,			  // 0x24
	KEY_K,			  // 0x25
	KEY_L,			  // 0x26
	KEY_SEMICOLON,	  // 0x27
	KEY_QUOTE,		  // 0x28
	KEY_GRAVE,		  // 0x29
	KEY_LSHIFT,		  // 0x2a
	KEY_BACKSLASH,	  // 0x2b
	KEY_Z,			  // 0x2c
	KEY_X,			  // 0x2d
	KEY_C,			  // 0x2e
	KEY_V,			  // 0x2f
	KEY_B,			  // 0x30
	KEY_N,			  // 0x31
	KEY_M,			  // 0x32
	KEY_COMMA,		  // 0x33
	KEY_DOT,		  // 0x34
	KEY_SLASH,		  // 0x35
	KEY_RSHIFT,		  // 0x36
	KEY_KP_ASTERISK,  // 0x37
	KEY_RALT,		  // 0x38
	KEY_SPACE,		  // 0x39
	KEY_CAPSLOCK,	  // 0x3a
	KEY_F1,			  // 0x3b
	KEY_F2,			  // 0x3c
	KEY_F3,			  // 0x3d
	KEY_F4,			  // 0x3e
	KEY_F5,			  // 0x3f
	KEY_F6,			  // 0x40
	KEY_F7,			  // 0x41
	KEY_F8,			  // 0x42
	KEY_F9,			  // 0x43
	KEY_F10,		  // 0x44
	KEY_KP_NUMLOCK,	  // 0x45
	KEY_SCROLLLOCK,	  // 0x46
	KEY_HOME,		  // 0x47
	KEY_KP_8,		  // 0x48	//keypad up arrow
	KEY_PAGEUP,		  // 0x49
	KEY_KP_2,		  // 0x50	//keypad down arrow
	KEY_KP_3,		  // 0x51	//keypad page down
	KEY_KP_0,		  // 0x52	//keypad insert key
	KEY_KP_DECIMAL,	  // 0x53	//keypad delete key
	KEY_UNKNOWN,	  // 0x54
	KEY_UNKNOWN,	  // 0x55
	KEY_UNKNOWN,	  // 0x56
	KEY_F11,		  // 0x57
	KEY_F12			  // 0x58
};

const int INVALID_SCANCODE = 0;

// Funzione che legge lo stato della keyboard controller
uint8_t kybrd_ctrl_read_status()
{
	return insbyte(KEYBOARD_CONTROLLER_STATUS);
}

// Funzione che invia un byte al keyboard controller
void kybrd_ctrl_send_cmd(uint8_t cmd)
{
	// Prima di inviare il byte dobbiamo aspettare
	// che l'input buffer sia impostato a 0
	while (1)
	{
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;
	}

	// Inviamo il byte
	outbyte(KEYBOARD_CONTROLLER_OUTPUT, cmd);
}

// Legge un byte dal Keyboard encoder
uint8_t kybrd_encoder_read_buf()
{
	return insbyte(KEYBOARD_ENC_INPUT);
}

// Invia un byte al Keyboard encoder
void kybrd_encoder_send_cmd(uint8_t cmd)
{
	// Dato che i comandi inviati al keyboard encoder
	// vengono prima inviati al keyboard controller
	// dobbiamo lo stesso aspettare che l'input buffer sia 0
	while (1)
	{
		if ((kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
			break;
	}

	// Inviamo il byte
	outbyte(KEYBOARD_ENC_OUTPUT, cmd);
}

// Funzione che verra chiamata ogni qualvolta verra premuto
// un tasto sulla tastiera
void keyboard_handler()
{
	// Variabile che conterra lo scan code
	int code = 0;

	// Leggiamo lo scan code solo quando il keybord controller é pieno (ovvero contiene lo scan code)
	if (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF)
	{
		// Leggiamo lo scan code
		code = kybrd_encoder_read_buf();

		if (code == 0xE0 || code == 0xE1)
			goto out;

		// Test per vedere se é un break code (quindi una key é stata rilasciata)
		if (code & 0x80)
		{
			// Viene convertito nella sua forma normale
			code -= 0x80;

			// Ottieni la key
			int key = _kkybrd_scancode_std[code];

			// Test se é stata rilasciata una key speciale
			switch (key)
			{
			case KEY_LCTRL:
			case KEY_RCTRL:
				_ctrl = false;
				break;

			case KEY_LSHIFT:
			case KEY_RSHIFT:
				_shift = false;
				break;

			case KEY_LALT:
			case KEY_RALT:
				_alt = false;
				break;

			default:
				break;
			}
		}
		else
		{
			// Questo é un break code (quindi una key é stata premuta)
			_scancode = code;

			// Otteniamo la key
			int key = _kkybrd_scancode_std[code];

			// Test per vedere se l'utente sta tenendo premuto una key speciale
			switch (key)
			{
			case KEY_LCTRL:
			case KEY_RCTRL:
				_ctrl = true;
				break;

			case KEY_LSHIFT:
			case KEY_RSHIFT:
				_shift = true;
				break;

			case KEY_LALT:
			case KEY_RALT:
				_alt = true;
				break;

			case KEY_CAPSLOCK:
				_capslock = (_capslock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;

			case KEY_KP_NUMLOCK:
				_numlock = (_numlock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;

			case KEY_SCROLLLOCK:
				_scrolllock = (_scrolllock) ? false : true;
				kkybrd_set_leds(_numlock, _capslock, _scrolllock);
				break;

			default:
				break;
			}
		}

	out:
		// Controlliamo gli errori
		switch (code)
		{
		case KYBRD_ERR_BAT_FAILED:
			_kkybrd_bat_res = false;
			break;

		case KYBRD_ERR_DIAG_FAILED:
			_kkybrd_diag_res = false;
			break;

		case KYBRD_ERR_RESEND_CMD:
			_kkybrd_resend_res = true;
		}
	}
}

bool kkybrd_get_scroll_lock()
{
	return _scrolllock;
}

bool kkybrd_get_numlock()
{
	return _numlock;
}

bool kkybrd_get_capslock()
{
	return _capslock;
}

bool kkybrd_get_ctrl()
{
	return _ctrl;
}

bool kkybrd_get_alt()
{
	return _alt;
}

bool kkybrd_get_shift()
{
	return _shift;
}

void kkybrd_ignore_resend()
{
	_kkybrd_resend_res = false;
}

bool kkybrd_check_resend()
{
	return _kkybrd_resend_res;
}

bool kkybrd_get_diagnostic_res()
{
	return _kkybrd_diag_res;
}

bool kkybrd_get_bat_res()
{
	return _kkybrd_bat_res;
}

uint8_t kkybrd_get_last_scan()
{
	return _scancode;
}

void kkybrd_set_leds(bool num, bool caps, bool scroll)
{
	uint8_t data = 0;

	// Imposta i bit
	data = (scroll) ? (data | 1) : (data & 1);
	data = (num) ? (num | 2) : (num & 2);
	data = (caps) ? (num | 4) : (num & 4);

	// Invia il comando di updatare i LED della tastiera
	kybrd_encoder_send_cmd(KYBRD_ENC_CMD_SET_LED);
	kybrd_encoder_send_cmd(data);
}

// Ottieni l'ultima key premuta
enum KEYCODE kkybrd_get_last_key()
{
	return (_scancode != INVALID_SCANCODE) ? ((enum KEYCODE)_kkybrd_scancode_std[_scancode]) : (KEY_UNKNOWN);
}

// Scarta l'ultima key premuta
void kkybrd_discard_last_key()
{
	_scancode = INVALID_SCANCODE;
}

// Converte una key ad ascii character
enum KEYCODE kkybrd_key_to_ascii(enum KEYCODE code)
{
	uint8_t key = code;

	//! Controlliamo che la key sia un carattere ascii valido
	if (isascii(key))
	{
		//! Controlliamo se lo shift o il capslock sono attivi
		//! se si trasforma il carattere in maiuscolo
		if (_shift || _capslock)
		{
			if (key >= 'a' && key <= 'z')
				key -= 32;
		}

		// Controlliamo se lo shift é attivo, se si alcuni tasti
		// assumeranno significati diversi
		if (_shift && !_capslock)
		{
			if (key >= '0' && key <= '9')
			{
				switch (key)
				{
				case '0':
					key = KEY_RIGHTPARENTHESIS;
					break;
				case '1':
					key = KEY_EXCLAMATION;
					break;
				case '2':
					key = KEY_AT;
					break;
				case '3':
					key = KEY_EXCLAMATION;
					break;
				case '4':
					key = KEY_HASH;
					break;
				case '5':
					key = KEY_PERCENT;
					break;
				case '6':
					key = KEY_CARRET;
					break;
				case '7':
					key = KEY_AMPERSAND;
					break;
				case '8':
					key = KEY_ASTERISK;
					break;
				case '9':
					key = KEY_LEFTPARENTHESIS;
					break;
				}
			}
			else
			{
				switch (key)
				{
				case KEY_COMMA:
					key = KEY_LESS;
					break;

				case KEY_DOT:
					key = KEY_GREATER;
					break;

				case KEY_SLASH:
					key = KEY_QUESTION;
					break;

				case KEY_SEMICOLON:
					key = KEY_COLON;
					break;

				case KEY_QUOTE:
					key = KEY_QUOTEDOUBLE;
					break;

				case KEY_LEFTBRACKET:
					key = KEY_LEFTCURL;
					break;

				case KEY_RIGHTBRACKET:
					key = KEY_RIGHTCURL;
					break;

				case KEY_GRAVE:
					key = KEY_TILDE;
					break;

				case KEY_MINUS:
					key = KEY_UNDERSCORE;
					break;

				case KEY_PLUS:
					key = KEY_EQUAL;
					break;

				case KEY_BACKSLASH:
					key = KEY_BAR;
					break;
				}
			}
		}

		//! Ritorna la key
		return key;
	}

	//! Ritorna 0 se il carattere ascii non é valido
	return 0;
}

// Disabilita la tastiera
void kkybrd_disable()
{
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_DISABLE);
	_kkybrd_disable = true;
}

// Abilita la tastiera
void kkybrd_enable()
{
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_ENABLE);
	_kkybrd_disable = false;
}

// Ritorna true se la tastiera é disabilitata
bool kkybrd_is_disabled()
{
	return _kkybrd_disable;
}

// Resetta la tastiera
void kkybrd_reset_system()
{
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_WRITE_OUT_PORT);
	kybrd_encoder_send_cmd(0xfe);
}

// Esegue un test di controllo sulla tastiera
bool kkybrd_self_test()
{
	// Esegui il comando
	kybrd_ctrl_send_cmd(KYBRD_CTRL_CMD_SELF_TEST);

	//! Attendiamo che abbia finito
	while (1)
		if (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF)
			break;

	//! Se l'ouput é uguale a 0x55 il test é positivo
	return (kybrd_encoder_read_buf() == 0x55) ? true : false;
}

// Prepara la tastiera
void kkybrd_install()
{
	// Imposta variabili
	_kkybrd_bat_res = true;
	_scancode = 0;

	// Imposta i led
	_numlock = _scrolllock = _capslock = false;
	kkybrd_set_leds(false, false, false);

	// Imposta le keys speciali
	_shift = _alt = _ctrl = false;

	// Abilitiamo la tastiera
	kkybrd_enable();
}

// Aspetta finchè una key non è premuta
enum KEYCODE getch()
{
	enum KEYCODE key = KEY_UNKNOWN;

	//! Aspetta per la pressione di un tasto
	while (key == KEY_UNKNOWN)
		key = kkybrd_get_last_key();

	//! Scarta il tasto rilasciato
	kkybrd_discard_last_key();
	return key;
}