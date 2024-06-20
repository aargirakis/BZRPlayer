	IFND	EXEC_FUNCDEF_I
EXEC_FUNCDEF_I	SET	1
**
**	$VER: funcdef.i 1.0 (24.7.96)
**	Includes Release 40.13+1
**
**	FUNCDEF macro definition for library bases
**
**	Created by Harry Sintonen. Public Domain.
**

FUNCDEF     MACRO    *function
_LVO\1      EQU      FUNC_CNT
FUNC_CNT    SET      FUNC_CNT-6
            ENDM
FUNC_CNT    SET      5*-6
FDEFINIT    MACRO    *[1st func offset]
            IFGT     NARG
FUNC_CNT    SET      (\1)
            ELSE
FUNC_CNT    SET      5*-6
            ENDC
            ENDM
	ENDC	; EXEC_FUNCDEF_I
