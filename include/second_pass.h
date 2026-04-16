#ifndef SECOND_PASS_H
#define SECOND_PASS_H

#include "globals.h"
#include "structures.h"

/*
 * הפעלת המעבר השני
 * קלט: שם קובץ .am, נתונים מהמעבר הראשון
 * פלט: TRUE אם הצליח, FALSE אם יש שגיאות
 */
bool execute_second_pass(const char *filename, SymbolTable *symbols, 
                         int *code_img, int *data_img, int ic, int dc);

#endif