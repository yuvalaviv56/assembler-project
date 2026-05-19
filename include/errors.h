/*
 * errors.h
 * ניהול והצגת שגיאות ואזהרות
 * 
 * קובץ זה מגדיר את כלל סוגי השגיאות שעלולים להתרחש באסמבלר
 * ואת הפונקציות שמדפיסות את סוג השגיאה למשתמש
 * 
 * כלל השלבים באסמבלר משתמשים בפונקציות אלה כדי לדווח על בעיות
 */

#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>

/*
 * שמגדיר את כל סוגי השגיאות האפשריות enum טיפוס  
 * 
 * כל שגיאה מקבלת מספר ייחודי שמאפשר לזהות אותה
 * כלל השגיאות מחולקות לקטגוריות: שגיאות קובץ, תחביר, לוגיות
 */
typedef enum {
    ERR_NONE,                      /* אין שגיאה */
    ERR_FILE_OPEN,                 /* לא הצלחנו לפתוח קובץ */
    ERR_LINE_TOO_LONG,             /* שורה עוברת את 80 תווים */
    ERR_INVALID_LABEL,             /* שם תווית לא חוקי */
    ERR_DUPLICATE_LABEL,           /* תווית מוגדרת פעמיים */
    ERR_UNDEFINED_LABEL,           /* תווית לא מוגדרת */
    ERR_INVALID_INSTRUCTION,       /* פקודה לא קיימת */
    ERR_INVALID_DIRECTIVE,         /* הוראה לא קיימת */
    ERR_INVALID_OPERAND,           /* אופרנד לא תקין */
    ERR_ILLEGAL_ADDRESSING,        /* מצב כתובת לא מותר לפקודה הזאת */
    ERR_OPERAND_COUNT,             /* מספר אופרנדים שגוי */
    ERR_INVALID_REGISTER,          /* רגיסטר לא קיים */
    ERR_INVALID_NUMBER,            /* מספר בפורמט שגוי */
    ERR_MACRO_NAME,                /* שם מקרו לא חוקי */
    ERR_MACRO_UNCLOSED,            /* מקרו לא נסגר */
    ERR_MEMORY_OVERFLOW,           /* התוכנית גדולה מדי */
    ERR_EXTERNAL_ENTRY_CONFLICT,   /* entry וגם extern סמל לא יכול להיות   */
    ERR_MISSING_COMMA,             /* חסר פסיק בין אופרנדים */
    ERR_EXTRA_COMMA,               /* פסיק מיותר */
    ERR_MISSING_QUOTE,             /* חסר גרש סוגר במחרוזת */
    ERR_EMPTY_DIRECTIVE            /* הוראה ללא פרמטרים */
} ErrorType;

/*
 * מדפיסה הודעת שגיאה עם מספר שורה
 * 
 * הפונקציה מקבלת את סוג השגיאה ומדפיסה את ההודעה המתאימה
 * אם קיים מספר שורה הוא מודפס כחלק מההודעה
 * message ניתן להוסיף מידע נוסף דרך הפרמטר 
 * 
 * פרמטרים:
 *   line_num - מספר השורה שבה אירעה השגיאה או 0 אם אין
 *   type - סוג השגיאה מהאופציות שלמעלה
 *   message -  אם אין שגיאהNULL הודעה נוספת או 
 */
void print_error(int line_num, ErrorType type, const char *message);

/*
 * מדפיסה הודעת אזהרה עם מספר שורה
 * 
 * אזהרה שונה משגיאה בכך שהיא לא עוצרת את תהליך האסמבלי
 * היא רק מיידעת את המשתמש על משהו שעשוי להיות בעייתי
 * 
 * פרמטרים:
 *   line_num - מספר השורה של האזהרה
 *   message - טקסט האזהרה
 */
void print_warning(int line_num, const char *message);

/*
 *פונק אשר מחזירה את טקסט השגיאה לפי סוגה
 * 
 * פונקציה זו ממירה את מספר השגיאה למחרוזת טקסט אשר מתארת את השגיאה
 * :פרמטרים
 *  type - סוג השגיאה
 * 
 * מחזירה: מצביע למחרוזת קבועה עם תיאור השגיאה
 */
const char* get_error_message(ErrorType type);

#endif
