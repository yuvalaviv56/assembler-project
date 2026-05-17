/*
 * errors.c
 * ניהול וטיפול בשגיאות ואזהרות במהלך תהליך האסמבלי
 * 
 * קובץ זה אחראי על כל הטיפול בשגיאות שיכולות להתרחש במהלך עבודת האסמבלר
 * החל משגיאות פתיחת קבצים וכלה בשגיאות תחביריות ולוגיות בקוד האסמבלי
 * 
 * הקובץ מספק שתי פונקציות עיקריות:
 * 1. get_error_message - מחזירה את הודעת השגיאה המתאימה לפי סוג השגיאה
 * 2. print_error - מדפיסה הודעת שגיאה למסך עם מספר שורה ופרטים נוספים
 * 3. print_warning - מדפיסה אזהרה שאינה עוצרת את תהליך האסמבלי
 * 
 * כל סוגי השגיאות מוגדרים ב-ErrorType enum בקובץ ה-header המתאים
 */

#include <stdio.h>
#include "errors.h"
#include "globals.h"

/*
 * הפונקציה מקבלת סוג שגיאה ומחזירה את המחרוזת המתארת את השגיאה
 * הפונקציה משתמשת ב-switch case כדי לבחור את ההודעה המתאימה
 * פרמטרים:
 *   type - סוג השגיאה מטיפוס ErrorType
 * מחזירה: מצביע למחרוזת קבועה עם תיאור השגיאה
 */
const char* get_error_message(ErrorType error_type_code) 
{
    switch (error_type_code) 
    {
        case ERR_NONE:
            return "No error";
            
        case ERR_FILE_OPEN:
            return "Cannot open file";
            
        case ERR_LINE_TOO_LONG:
            return "Line exceeds maximum length (80 characters)";
            
        case ERR_INVALID_LABEL:
            return "Invalid label name";
            
        case ERR_DUPLICATE_LABEL:
            return "Label already defined";
            
        case ERR_UNDEFINED_LABEL:
            return "Undefined label";
            
        case ERR_INVALID_INSTRUCTION:
            return "Invalid instruction";
            
        case ERR_INVALID_DIRECTIVE:
            return "Invalid directive";
            
        case ERR_INVALID_OPERAND:
            return "Invalid operand";
            
        case ERR_ILLEGAL_ADDRESSING:
            return "Illegal addressing mode for this instruction";
            
        case ERR_OPERAND_COUNT:
            return "Wrong number of operands";
            
        case ERR_INVALID_REGISTER:
            return "Invalid register";
            
        case ERR_INVALID_NUMBER:
            return "Invalid number format";
            
        case ERR_MACRO_NAME:
            return "Invalid macro name (cannot be instruction/directive/register)";
            
        case ERR_MACRO_UNCLOSED:
            return "Macro definition not closed (missing 'mcroend')";
            
        case ERR_MEMORY_OVERFLOW:
            return "Memory overflow - program too large";
            
        case ERR_EXTERNAL_ENTRY_CONFLICT:
            return "Symbol cannot be both external and entry";
            
        case ERR_MISSING_COMMA:
            return "Missing comma between operands";
            
        case ERR_EXTRA_COMMA:
            return "Extraneous comma";
            
        case ERR_MISSING_QUOTE:
            return "Missing closing quote in string";
            
        case ERR_EMPTY_DIRECTIVE:
            return "Directive has no parameters";
            
        default:
            return "Unknown error";
    }
}

/*
 * מדפיסה הודעת שגיאה למסך השגיאות stderr
 * הפונקציה מקבלת את מספר השורה בה אירעה השגיאה את סוג השגיאה ומסר נוסף אופציונלי
 * אם יש מספר שורה תקין (גדול מ-0) הוא יודפס כחלק מההודעה
 * פרמטרים:
 *   line_number_in_source_file - מספר השורה בקובץ המקור (0 אם אין)
 *   error_type_code - סוג השגיאה
 *   additional_error_details -  NULL מסר נוסף או 
 */
void print_error(int line_number_in_source_file, ErrorType error_type_code, const char *additional_error_details) {
    const char *base_error_message;
    
    /* קודם כל נשיג את הודעת השגיאה הבסיסית */
    base_error_message = get_error_message(error_type_code);
    
    /* אם יש מספר שורה תקין נדפיס אותו */
    if (line_number_in_source_file > 0) 
    {
        fprintf(stderr, "Error (line %d): %s", line_number_in_source_file, base_error_message);
    } 
    else{
        fprintf(stderr, "Error: %s", base_error_message);
    }
    
    /* אם יש פרטים נוספים נוסיף אותם להודעה */
    if (additional_error_details != NULL && *additional_error_details != '\0') 
    {
        fprintf(stderr, " - %s", additional_error_details);
    }
    
    fprintf(stderr, "\n");
}

/*
 * מדפיסה אזהרה למסך
 * אזהרה היא הודעה שלא עוצרת את תהליך האסמבלי אבל מיידעת על בעיה פוטנציאלית
 */
void print_warning(int line_number_in_source_file, const char *warning_message_text) 
{
    if(line_number_in_source_file > 0) {
        fprintf(stderr, "Warning (line %d): %s\n", line_number_in_source_file, warning_message_text);
    } else
    
    {
        fprintf(stderr, "Warning: %s\n", warning_message_text);
    }
}
