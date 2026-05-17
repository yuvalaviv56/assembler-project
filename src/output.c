/*
 * output.c
 * יוצרת את קבצי הפלט הסופיים של האסמבלר
 * 
 * :נוצרים שלושה סוגים של קבצי פלט
 * 1. מכיל את הקוד והנתונים המקודדים- ob קובץ אובייקט
 * 2. entries ent - מכיל רשימת סמלים שהוגדרו כ-entryקובץ 
 * 3. מכיל רשימת כתובות שבהן נעשה שימוש בסמלים חיצוניים-externals ext קובץ 
 * 
 * כל הערכים בקבצים מיוצגים בבסיס הקסדצימלי בן 3 ספרות
 * הכתובות מתחילות מ-100 כפי שמוגדר בהנחיות הפרויקט
 */

#include <stdio.h>
#include <string.h>
#include "output.h"
#include "structures.h"
#include "globals.h"

/* ממירה מספר לייצוג הקסדצימלי */
static void to_hex(int numeric_value, char *output_string) {
    sprintf(output_string, "%03X", numeric_value & 0xFFF);
}

/* ob פונקציה אשר יוצרת את קובץ האובייקט  */
bool create_object_file(const char *base_name, MemoryImage *memory) {
    FILE *output_file_pointer;
    char output_filename[256];
    char hexadecimal_representation[4];
    int loop_counter, current_memory_address;
    
    
    sprintf(output_filename, "%s.ob", base_name);
    output_file_pointer = fopen(output_filename, "w");
    if (output_file_pointer == NULL) 
    {
        fprintf(stderr, "Cannot create file: %s\n", output_filename);
        return FALSE;
    }
    
    /*  השורה הראשונה נכתבת לקובץ האובייקט-אשר מכיל מספר פקודות ונתונים */
    fprintf(output_file_pointer, "%d %d\n", memory->ICF - 100, memory->DCF);
    
    /* עובר על כלל הפקודות ומילות הקוד נכתבות לקובץ */
    for (loop_counter=0; loop_counter < memory->ICF - 100; loop_counter = loop_counter + 1) 
    {
        current_memory_address = 100 + loop_counter;
        to_hex(memory->code[100 + loop_counter].word, hexadecimal_representation);
        fprintf(output_file_pointer, "%04d %s\n", current_memory_address, hexadecimal_representation);
    }
    
    /* הנתונים נכתבים לקובץ */
    for (loop_counter=0; loop_counter < memory->DCF; loop_counter++)
    {
        current_memory_address = memory->ICF + loop_counter;
        to_hex(memory->data[loop_counter].word, hexadecimal_representation);
        fprintf(output_file_pointer, "%04d %s\n", current_memory_address, hexadecimal_representation);
    }
    
    fclose(output_file_pointer);
    printf("Created: %s\n", output_filename);
    return TRUE;
}

/*  entry פונקציה אשר יוצרת קובץ  של סמלי *
 * ורשימת סמלים שקבצים אחרים יכולים לגשת אליהם */

bool create_entries_file(const char *base_name, SymbolTable *symbols) {
    FILE *entries_file_pointer;
    char entries_filename[256];
    Symbol *current_symbol_pointer;
    int found_any_entry_symbols = FALSE;
    
    /* entryעובר על כלל הסמלים ומחפש את אלו אשר מסומנים כ*
     *לסמלים אלו יש גישה מקבצים אחרים */
    current_symbol_pointer = symbols->head;
    while (current_symbol_pointer != NULL) 
    {
        if (current_symbol_pointer->attributes & ATTR_ENTRY) {
            found_any_entry_symbols = TRUE;
            break;
        }
        current_symbol_pointer = current_symbol_pointer->next;
    }
    
    /*  ent אין צורך ליצור קבצי entry אם אין סמלי*/
    if (found_any_entry_symbols == FALSE) {
        return TRUE;
    }
    /*ent-בונה את קובץ ה*/
    sprintf(entries_filename, "%s.ent", base_name);
    entries_file_pointer = fopen(entries_filename, "w");
    if (entries_file_pointer == NULL) {
        fprintf(stderr, "Cannot create file: %s\n", entries_filename);
        return FALSE;
    }
    
    /* entry-עוברת על כלל הסמלים וכותבת רק את אלו אשר מסומנים כ *
    *כל שורה בקובץ מכילהאת שם הסמל וכתובתו,כך שקבצים אחרים יוכלו לדעת היכן הסמל נמצא ולאן לגשת*/
    current_symbol_pointer = symbols->head;
    while (current_symbol_pointer != NULL) {
        if (current_symbol_pointer->attributes & ATTR_ENTRY) {
            fprintf(entries_file_pointer, "%s %04d\n", current_symbol_pointer->name, current_symbol_pointer->value);
        }
        current_symbol_pointer = current_symbol_pointer->next;
    }
    
    fclose(entries_file_pointer);
    printf("Created: %s\n", entries_filename);
    return TRUE;
}

/* externals יוצרת קובץ של *
 *מכיל רשימה של כלל המקומות שבהם השתמשנו בסמלים חיצוניים*
 *כאשר סמל חיצוני מוגדר בקובץ אחר והתכנית הנוכחית משתמשת בו*/
bool create_externals_file(const char *base_name, ExternalList *externals) {
    FILE *externals_file_pointer;
    char externals_filename[256];
    ExternalRef *current_external_reference;
    
    current_external_reference = (ExternalRef*)externals->head;
    
    if (current_external_reference == NULL) {
        return TRUE;
    }
    
    sprintf(externals_filename, "%s.ext", base_name);
    externals_file_pointer = fopen(externals_filename, "w");
    if (externals_file_pointer == NULL) 
    {
        fprintf(stderr, "Cannot create file: %s\n", externals_filename);
        return FALSE;
    }
    
    /* כותב את כלל ההפניות לסמלים חיצוניים*
    *כאשר כל שורה מכילה את שם הסמל והכתובת שממנה נלקחה */
    while (current_external_reference != NULL) 
    {
        fprintf(externals_file_pointer, "%s %04d\n", current_external_reference->symbol, current_external_reference->address);
        current_external_reference = current_external_reference->next;
    }
    
    fclose(externals_file_pointer);
    printf("Created: %s\n", externals_filename);
    return TRUE;
}
