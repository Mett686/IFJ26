// Implementace překladače imperativního jazyka IFJ26
// stdFunc.h
// Implementace standardních funkcí v IFJcode26

// Autor: Martin Knotek (xknotem00)
// Autor: Filip Maxa (xmaxafi00)

#ifndef STDFUNC_H
#define STDFUNC_H

#define STD_GLOBALS "DEFVAR GF@CONCAT$$$0\nDEFVAR GF@CONCAT$$$1\n"
#define STD_FUNC_STR0                                                                                                  \
    "# PRINT\nLABEL print\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nPOPS "          \
    "LF@value\nTYPE LF@type LF@value\nEQ LF@isnil LF@type string@nil\nJUMPIFEQ print$unit LF@isnil bool@true\nWRITE "  \
    "LF@value\nJUMP print$end\n\nLABEL print$unit\nWRITE string@()\n\nLABEL print$end\nPOPFRAME\nRETURN\n\n\n# "       \
    "READLINE\nLABEL readLine\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nREAD "      \
    "LF@value string\nTYPE LF@type LF@value\nEQ LF@isnil LF@type string@nil\nJUMPIFEQ readLine$eof LF@isnil "          \
    "bool@true\nPUSHS LF@value\nJUMP readLine$end\n\nLABEL readLine$eof\nPUSHS string@\n\nLABEL "                      \
    "readLine$end\nPOPFRAME\nRETURN\n\n\n# READINT\nLABEL readInt\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR " \
    "LF@type\nDEFVAR LF@isnil\nREAD LF@value int\nTYPE LF@type LF@value\nEQ LF@isnil LF@type string@nil\nJUMPIFEQ "    \
    "readInt$bad LF@isnil bool@true\nPUSHS LF@value\nJUMP readInt$end\n\nLABEL readInt$bad\nPUSHS int@0\n\nLABEL "     \
    "readInt$end\nPOPFRAME\nRETURN\n\n\n# READDOUBLE\nLABEL readDouble\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR "             \
    "LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nREAD LF@value float\nTYPE LF@type LF@value\nEQ LF@isnil LF@type "      \
    "string@nil\nJUMPIFEQ readDouble$bad LF@isnil bool@true\nPUSHS LF@value\nJUMP readDouble$end\n\nLABEL "            \
    "readDouble$bad\nPUSHS float@0x0p+0\n\nLABEL readDouble$end\nPOPFRAME\nRETURN\n\n\n# READBOOLEAN\nLABEL "          \
    "readBoolean\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nREAD LF@value "          \
    "bool\nTYPE LF@type LF@value\nEQ LF@isnil LF@type string@nil\nJUMPIFEQ readBoolean$bad LF@isnil bool@true\nPUSHS " \
    "LF@value\nJUMP readBoolean$end\n\nLABEL readBoolean$bad\nPUSHS bool@false\n\nLABEL "                              \
    "readBoolean$end\nPOPFRAME\nRETURN\n\n\n# READINTMAYBE\nLABEL readIntMaybe\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR "     \
    "LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nREAD LF@value int\nTYPE LF@type LF@value\nEQ LF@isnil LF@type "        \
    "string@nil\nJUMPIFEQ readIntMaybe$bad LF@isnil bool@true\nPUSHS LF@value\nJUMP readIntMaybe$end\n\nLABEL "        \
    "readIntMaybe$bad\nPUSHS nil@nil\n\nLABEL readIntMaybe$end\nPOPFRAME\nRETURN\n\n\n# READDOUBLEMAYBE\nLABEL "       \
    "readDoubleMaybe\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nREAD LF@value "      \
    "float\nTYPE LF@type LF@value\nEQ LF@isnil LF@type string@nil\nJUMPIFEQ readDoubleMaybe$bad LF@isnil "             \
    "bool@true\nPUSHS LF@value\nJUMP readDoubleMaybe$end\n\nLABEL readDoubleMaybe$bad\nPUSHS nil@nil\n\nLABEL "        \
    "readDoubleMaybe$end\nPOPFRAME\nRETURN\n"
#define STD_FUNC_STR1                                                                                                  \
    "# TOSTR\nLABEL toStr\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@type\nDEFVAR LF@isnil\nDEFVAR "        \
    "LF@isint\nDEFVAR LF@isfloat\nDEFVAR LF@isbool\nDEFVAR LF@result\nPOPS LF@value\nTYPE LF@type LF@value\n\nEQ "     \
    "LF@isnil LF@type string@nil\nJUMPIFEQ toStr$nil LF@isnil bool@true\n\nEQ LF@isint LF@type string@int\nJUMPIFEQ "  \
    "toStr$int LF@isint bool@true\n\nEQ LF@isfloat LF@type string@float\nJUMPIFEQ toStr$float LF@isfloat "             \
    "bool@true\n\nEQ LF@isbool LF@type string@bool\nJUMPIFEQ toStr$bool LF@isbool bool@true\n\nMOVE LF@result "        \
    "LF@value\nJUMP toStr$end\n\nLABEL toStr$nil\nMOVE LF@result string@()\nJUMP toStr$end\n\nLABEL "                  \
    "toStr$int\nINT2STR LF@result LF@value\nJUMP toStr$end\n\nLABEL toStr$float\nFLOAT2STR LF@result LF@value\nJUMP "  \
    "toStr$end\n\nLABEL toStr$bool\nJUMPIFEQ toStr$booltrue LF@value bool@true\nMOVE LF@result string@false\nJUMP "    \
    "toStr$end\n\nLABEL toStr$booltrue\nMOVE LF@result string@true\n\nLABEL toStr$end\nPUSHS "                         \
    "LF@result\nPOPFRAME\nRETURN\n\n\n# TOINT\nLABEL toInt\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR "        \
    "LF@result\nPOPS LF@value\nFLOAT2INT LF@result LF@value\nPUSHS LF@result\nPOPFRAME\nRETURN\n\n\n# FLOOR\nLABEL "   \
    "floor\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@truncated\nDEFVAR LF@truncatedfloat\nDEFVAR "         \
    "LF@negative\nDEFVAR LF@result\nPOPS LF@value\nFLOAT2INT LF@truncated LF@value\nINT2FLOAT LF@truncatedfloat "      \
    "LF@truncated\nLT LF@negative LF@value LF@truncatedfloat\nJUMPIFEQ floor$done LF@negative bool@false\nSUB "        \
    "LF@truncated LF@truncated int@1\nINT2FLOAT LF@result LF@truncated\nJUMP floor$end\n\nLABEL floor$done\nMOVE "     \
    "LF@result LF@truncatedfloat\n\nLABEL floor$end\nPUSHS LF@result\nPOPFRAME\nRETURN\n\n# strLength\nLABEL "         \
    "strLength\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR LF@result\nPOPS LF@value\nSTRLEN LF@result "         \
    "LF@value\nPUSHS LF@result\nPOPFRAME\nRETURN\n\n\n# SUBSTR\nLABEL subStr\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR "       \
    "LF@value\nDEFVAR LF@start\nDEFVAR LF@end\nDEFVAR LF@length\nDEFVAR LF@index\nDEFVAR LF@char\nDEFVAR "             \
    "LF@result\nDEFVAR LF@tmp\nPOPS LF@value\nPOPS LF@start\nPOPS LF@end\n\nSTRLEN LF@length LF@value\nLT LF@tmp "     \
    "LF@start int@0\nJUMPIFEQ substr$invalid LF@tmp bool@true\nLT LF@tmp LF@end int@0\nJUMPIFEQ substr$invalid "       \
    "LF@tmp bool@true\nGT LF@tmp LF@start LF@end\nJUMPIFEQ substr$invalid LF@tmp bool@true\nGT LF@tmp LF@start "       \
    "LF@length\nJUMPIFEQ substr$invalid LF@tmp bool@true\nEQ LF@tmp LF@start LF@length\nJUMPIFEQ substr$invalid "      \
    "LF@tmp bool@true\nGT LF@tmp LF@end LF@length\nJUMPIFEQ substr$invalid LF@tmp bool@true\n\nMOVE LF@index "         \
    "LF@start\nMOVE LF@result string@\n\nLABEL substr$loop\nEQ LF@tmp LF@index LF@end\nJUMPIFEQ substr$done LF@tmp "   \
    "bool@true\nGETCHAR LF@char LF@value LF@index\nCONCAT LF@result LF@result LF@char\nADD LF@index LF@index "         \
    "int@1\nJUMP substr$loop\n\nLABEL substr$invalid\nEXIT int@40\n\nLABEL substr$done\nPUSHS "                        \
    "LF@result\nPOPFRAME\nRETURN\n\n\n# ORD\nLABEL ord\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@value\nDEFVAR "            \
    "LF@index\nDEFVAR LF@length\nDEFVAR LF@tmp\nDEFVAR LF@result\nPOPS LF@value\nPOPS LF@index\n\nSTRLEN LF@length "   \
    "LF@value\nEQ LF@tmp LF@length int@0\nJUMPIFEQ ord$minusone LF@tmp bool@true\nLT LF@tmp LF@index int@0\nJUMPIFEQ " \
    "ord$minusone LF@tmp bool@true\nGT LF@tmp LF@index LF@length\nJUMPIFEQ ord$minusone LF@tmp bool@true\nEQ LF@tmp "  \
    "LF@index LF@length\nJUMPIFEQ ord$minusone LF@tmp bool@true\n\nSTRI2INT LF@result LF@value LF@index\nPUSHS "       \
    "LF@result\nJUMP ord$end\n\nLABEL ord$minusone\nPUSHS int@-1\n\nLABEL ord$end\nPOPFRAME\nRETURN\n\n\n# "           \
    "CHR\nLABEL chr\nCREATEFRAME\nPUSHFRAME\n\nDEFVAR LF@code\nDEFVAR LF@tmp\nDEFVAR LF@result\nPOPS LF@code\n\nLT "   \
    "LF@tmp LF@code int@0\nJUMPIFEQ chr$invalid LF@tmp bool@true\nGT LF@tmp LF@code int@127\nJUMPIFEQ chr$invalid "    \
    "LF@tmp bool@true\n\nINT2CHAR LF@result LF@code\nPUSHS LF@result\nJUMP chr$end\n\nLABEL chr$invalid\nEXIT "        \
    "int@58\n\nLABEL chr$end\nPOPFRAME\nRETURN\n"

#endif
