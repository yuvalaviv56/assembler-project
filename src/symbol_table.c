/*
 * symbol_table.c
 * ניהול טבלת הסמלים
 * 
 * טבלת הסמלים הינו מבנה הנתונים המרכזי באסמבלר
 * הטבלה שומרת את כלל התוויות שמופיעות בקוד עם הכתובות שלהן
 * 
 * :כל סמל בטבלה מכיל
 * - שם הסמל
 * - ערך הכתובת שלו
 * - entry או extern תכונות כמו האם קוד הנתונים הינו 
 * 
 * הטבלה בנויה כרשימה מקושרת ע"מ שהוספת הסמלים תתבצע בבקלות
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "string_utils.h"
#include "constants.h"
#include "globals.h"

/* אתחול טבלת סמלים ריקה */
void symbol_table_init(SymbolTable *table) {
    if (table == NULL) {
        return;
    }
    
    /* אתחול רשימה מקושרת ריקה */
    table->head = NULL;
    table->count = 0;
}

/*
 * מוסיפה סמל חדש לטבלה
 * הפונקציה בודקת האם הסמל החדש כבר קיים
 * במידה ולא היא יוצרת רשומה חדשה
 * הסמל החדש מתווסף בראש הרשימה משיקולי יעילות
 */
int symbol_table_add(SymbolTable *symbol_table_pointer, const char *symbol_name, int symbol_value, unsigned int symbol_attributes) {
    Symbol *new_symbol_entry;
    
    if (symbol_table_pointer == NULL || symbol_name == NULL) {
        return FALSE;
    }
    
    /* בודק אם הסמל כבר קיים בטבלה */
    /* אם אכן המצב, מדובר בשגיאה של הגדרה כפולה */
    if (symbol_table_find(symbol_table_pointer, symbol_name) != NULL) {
        return FALSE;
    }
    
    /* מקצה זיכרון לסמל החדש */
    new_symbol_entry = (Symbol*)malloc(sizeof(Symbol));
    if (new_symbol_entry == NULL) 
    {
        return FALSE;
    }
    
    /* ממלא את כלל הפרטים של הסמל החדש */
    safe_strcpy(new_symbol_entry->name, symbol_name, MAX_LABEL_LENGTH + 1);
    new_symbol_entry->value = symbol_value;
    new_symbol_entry->attributes = symbol_attributes;
    
    /* הוספת ההסמל בראש הרשימה */
    /*  פעולה יעילה מכיוון שאין צורך לעבור על כלל הרשימה בכל הוספת של סמל חדש */
    new_symbol_entry->next = symbol_table_pointer->head;
    symbol_table_pointer->head = new_symbol_entry;
    symbol_table_pointer->count = symbol_table_pointer->count + 1;
    
    return TRUE;
}

/*
 * פונ' אשר מחפשת סמל בטבלה לפי שם
 * עוברת על כלל הסמלים אשר כבר נמצאים בטבלה ומשווה שמות
 * NULL אם מוצאת מחזירה מצביע לסמל אחרת מחזירה 
 */
Symbol* symbol_table_find(SymbolTable *symbol_table_pointer, const char *symbol_name_to_find) {
    Symbol *current_symbol_in_list;
    
    if (symbol_table_pointer == NULL || symbol_name_to_find == NULL) {
        return NULL;
    }
    
    /* עוברת על כלל הסמלים ברשימה */
    current_symbol_in_list = symbol_table_pointer->head;
    while (current_symbol_in_list != NULL) 
    {
        /* משווה את השם של הסמל הנוכחי עם השם הנוכחי שמחפשים */
        if (strcmp(current_symbol_in_list->name, symbol_name_to_find) == 0) 
        {
            return current_symbol_in_list;
        }
        current_symbol_in_list = current_symbol_in_list->next;
    }
    
    /* במידה ולא מצאנו את הסמל */
    return NULL;
}

/*
 * פונ' אשר מעדכנת תכונות של סמל קיים
 *   לסמל שכבר קייםentry משמש בעיקר להוסיף תכונת  
 *  or התכונות מתווספות בעזרת האופרטור הלוגי  
 * בעזרת האופרטור תכונות קיימות נשארות ומווספות תכונות חדשות מבלי לפגוע בקיימות 
 */
int symbol_table_update_attributes(SymbolTable *symbol_table_pointer, const char *symbol_name, unsigned int new_attributes) {
    Symbol *found_symbol_entry;
    
    /* מחפש את הסמל בטבלה */
    found_symbol_entry = symbol_table_find(symbol_table_pointer, symbol_name);
    
    if (found_symbol_entry == NULL) 
    {
        return FALSE;
    }
    
    /* מוסיף את התכונות החדשות לתכונות הקיימות */
    /* השימוש ב-OR ביטים שומר את התכונות הישנות */
    found_symbol_entry->attributes = found_symbol_entry->attributes | new_attributes;
    
    return TRUE;
}

/* פונ' אשר מעדכנת את הערך של סמל*/
int symbol_table_update_value(SymbolTable *symbol_table_pointer, const char *symbol_name, int new_value) {
    Symbol *found_symbol_entry;
    
    /* מחפש את הסמל */
    found_symbol_entry = symbol_table_find(symbol_table_pointer, symbol_name);
    
    if (found_symbol_entry == NULL) {
        return FALSE;
    }
    
    /* מעדכן את הערך לערך החדש */
    found_symbol_entry->value = new_value;
    
    return TRUE;
}

/*
 * פונ' אשר מעדכנת את כלל סמלי הנתונים בעזרת הזזה
 * 
 * במעבר הראשון הקוד והנתונים מקבלים כתובות בנפרד
 * הקוד מתחיל מכתובת ההתחלה והנתונים מאותחלים לכתובת אפס
 * אך בזיכרון הסופי הכל צריך להיות רצוף ברצף אחד
 * 
 * הנתונים צריכים להופיע בזיכרון מיד אחרי סיום הקוד 
 * לכן צריך להוסיף לכל כתובת של סמל נתונים את גודל הקוד
 * ההזזה שמתווספת היא המרחק בין תחילת הזיכרון לסוף הקוד
 * 
 * הפונקציה עוברת על כלל הסמלים בטבלה
 * לכל סמל שמסומן כנתונים היא מוסיפה את ההזזה לערך שלו
 * כך כל הנתונים מוזזים להתחיל מהמיקום הנכון אחרי הקוד
 * 
 * הפונקציה נקראת בסוף המעבר הראשון אחרי שסיימנו לעבור על כל הקובץ
 */
void symbol_table_update_data_symbols(SymbolTable *symbol_table_pointer, int offset_to_add) {
    Symbol *current_symbol_in_iteration;
    
    if (symbol_table_pointer == NULL) {
        return;
    }
    
    /* עובר על כלל הסמלים ברשימה */
    current_symbol_in_iteration = symbol_table_pointer->head;
    while (current_symbol_in_iteration != NULL) 
    {
        /* אם הסמל הוא נתונים מוסיף לו את ההזזה */
        /* ההזזה היא בעצם גודל כל הקוד */
        if (current_symbol_in_iteration->attributes & ATTR_DATA) 
        {
            current_symbol_in_iteration->value = current_symbol_in_iteration->value + offset_to_add;
        }
        current_symbol_in_iteration = current_symbol_in_iteration->next;
    }
}

/*
 * פונ' אשר בודקת אם לסמל יש תכונה מסוימת
 *  AND-בעזרת אופרטור לוגי 
 */
int symbol_has_attribute(const Symbol *symbol_to_check, SymbolAttribute attribute_to_check) {
    if (symbol_to_check == NULL) {
        return FALSE;
    }
    
    /* בודק אם הביט של התכונה דלוק */
    return (symbol_to_check->attributes & attribute_to_check) != 0;
}

/*
 * מדפיסה את כלל טבלת הסמלים למסך
 * מדפיס כל סמל עם הכתובת והתכונות שלו
 */
void symbol_table_print(const SymbolTable *symbol_table_pointer) {
    Symbol *current_symbol_to_print;
    
    if (symbol_table_pointer == NULL) 
    {
        printf("Symbol table is NULL\n");
        return;
    }
    
   /* מדפיס את הכותרת של הטבלה */
    printf("\nSYMBOL TABLE\n");
    printf("Total symbols: %d\n\n", symbol_table_pointer->count);
    
    /* מדפיס שורת כותרות לעמודות */
    printf("%-15s %-10s %-20s\n", "Name", "Value", "Attributes");
    printf("%-15s %-10s %-20s\n", "----", "-----", "----------");
    
    /* עובר על כל הסמלים ברשימה ומדפיס כל אחד */
    current_symbol_to_print = symbol_table_pointer->head;
    while (current_symbol_to_print != NULL) 
    {
        char attributes_string_for_display[50] = "";
        
        /* בונה מחרוזת טקסט שמכילה את כל התכונות של הסמל */
        /* התכונות מופרדות ברווח אחת מהשנייה */
        if (current_symbol_to_print->attributes & ATTR_CODE) 
        {
            strcat(attributes_string_for_display, "CODE ");
        }
        if (current_symbol_to_print->attributes & ATTR_DATA) {
            strcat(attributes_string_for_display, "DATA ");
        }
        if (current_symbol_to_print->attributes & ATTR_ENTRY) 
        {
            strcat(attributes_string_for_display, "ENTRY ");
        }
        if (current_symbol_to_print->attributes & ATTR_EXTERNAL) {
            strcat(attributes_string_for_display, "EXTERNAL ");
        }
        
        /* NONE אם אין תכונות בכלל כותבים  */
        if (current_symbol_to_print->attributes == ATTR_NONE) {
            strcat(attributes_string_for_display, "NONE");
        }
        
        /* מדפיס שורה אחת עם שם הסמל הכתובת והתכונות */
        printf("%-15s %-10d %-20s\n", current_symbol_to_print->name, current_symbol_to_print->value, attributes_string_for_display);
        
        /* עובר לסמל הבא ברשימה */
        current_symbol_to_print = current_symbol_to_print->next;
    }
    
    printf("\n");
}

/*
 * פונ' אשר משחררת את כל הזיכרון של טבלת הסמלים
 * 
 * malloc כל סמל בטבלה הוקצה דינמית בעזרת 
 * פעולת השחרור מונעת זליגת זיכרון
 * 
 * הפונקציה עוברת על כלל הסמלים ברשימה המקושרת
 * כדי לשחרר את הזיכרון שלו free-לכל סמל היא קוראת ל 
 * אי אפשר לגשת לנתונים free צריך לשמור את המצביע הבא לפני השחרור כי אחרי  
 * 
 * בסוף הפונ' מאפסת את הראש והמונה של הטבלה כדי שתהיה ריקה
 */ 
void symbol_table_free(SymbolTable *symbol_table_pointer) {
    Symbol *current_symbol_to_free;
    Symbol *next_symbol_in_list;
    
    if (symbol_table_pointer == NULL) {
        return;
    }
    
    /* מתחיל מהראש של הרשימה */
    current_symbol_to_free = symbol_table_pointer->head;
    
    /* עובר על כלל הסמלים ברשימה ומשחרר כל אחד */
    while (current_symbol_to_free != NULL) 
    {
        /* שומר את המצביע לסמל הבא לפני שמשחרר את הנוכחי */
        /* אי אפשר לעבור להבא בתורfree  זה קריטי כי אחרי */
        next_symbol_in_list = current_symbol_to_free->next;
        
        /* משחרר את הזיכרון של הסמל הנוכחי */
        free(current_symbol_to_free);
        
        /* עובר לסמל הבא שכבר שמרנו את המצביע אליו */
        current_symbol_to_free = next_symbol_in_list;
    }
    
    /* מאפס את הטבלה כדי שתהיה ריקה  */
    symbol_table_pointer->head = NULL;
    symbol_table_pointer->count = 0;
}