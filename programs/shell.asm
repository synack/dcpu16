		IFE [0x1001], 0
		SET PC, blank
		SET PC, prompt

:blank	SET [0x8000+I], 0x20
		IFE I, 384
		SET [0x1001], 1
		ADD I, 1
		SET PC, 0

:prompt	SET I, [0x1000]
		SET [0x8000+I], 0x3E
		ADD I, 1
		SET [0x8000+I], 0x20
		ADD I, 1
		SET [0x1000], I
		SET PUSH, prompt
		SET PUSH, exec
		SET PC, getch

:getch	SET B, [0x9010]
		SET A, [0x9000+B]
		SET [0x9000+B], 0
		ADD B, 1
		MOD B, 16
		SET [0x9010], B
		IFE A, 0
		SET PC, getch
		SET PC, echo

:echo	IFE A, 0x0A
		SET PC, newline
		IFE A, 0x7F
		SET PC, backspace
:putch	SET B, [0x1000]
		SET [0x8000+B], A
		ADD B, 1
		MOD B, 384
		SET [0x1000], B

		SET B, [0x1100]
		ADD B, 1
		IFG B, 0x100
		SET B, 0
		SET [0x1101+B], A
		SET [0x1100], B
		SET PC, getch

:backspace
		SET B, [0x1000]
		SUB B, 1
		SET [0x8000+B], 0x20
		MOD B, 384
		SET [0x1000], B

		SET B, [0x1100]
		SUB B, 1
		IFG B, 0x100
		SET B, 0
		SET [0x1101+B], 0x20
		SET [0x1100], B
		SET PC, getch

:newline
		SET B, [0x1000]
		SET C, B
		MOD B, 32
		SUB C, B
		ADD C, 32
		SET [0x1000], C
		SET PC, POP

:clearbuf
		SET [0x1100+A], 0
		ADD A, 1
		IFE A, 0x100
		SET PC, POP
		SET PC, clearbuf

; At this point, 0x1100 contains the length of the line, followed by that many
; characters of input up to 0x1201
:exec	SET A, 0
		SET J, [0x1100]
		SET PC, clearbuf
