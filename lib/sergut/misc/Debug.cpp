#include "sergut/misc/StringRef.h"

/* Note: The "MS" section flags are to remove duplicates.  */
#define DEFINE_GDB_PY_SCRIPT(scriptName) \
  asm("\
.pushsection \".debug_gdb_scripts\", \"MS\",@progbits,1\n\
.byte 1 /* Python */\n\
.asciz \"" scriptName "\"\n\
.popsection \n\
");

DEFINE_GDB_PY_SCRIPT("sergut-1.1.0-gdb-script.py")
