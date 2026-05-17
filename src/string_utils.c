/*
 * string_utils.c
 * קובץ אשר מכיל פונקציות עזר לעבודה עם מחרוזות
 * פונקציות מבצעות הסרת רווחים בדיקת תקינות של מילים שמורות ותוויות
 * פונקציות אלו משמשות קבצים אחרים בכלל תהליך האסמבלר
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string_utils.h"
#include "constants.h"
#include "globals.h"

/* בודק אם תו הוא רווח */
int is_whitespace(char character_to_check) {
    return (character_to_check == ' ' || character_to_check == '\t');
}

/*
 * פונקציה אשר מסירה רווחים מההתחלה והסוף של מחרוזת
 * הפונקציה משנה את המחרוזת ומחזירה אותה מתוקנת
 */
char* trim(char *str) {
    char *start_position;
    char *end_position;
    
    if (str == NULL || *str == '\0') {
        return str;
    }
    
    /* פונקציה אשר מזהה את התו הראשון אשר מסמל את המיקום האמיתי של תחילת המחרוזת  */
    start_position = str;
    while (is_whitespace(*start_position) || *start_position == '\n' || *start_position == '\r') {
        start_position++;
    }
    
    /* בודק אם כלל התווים הם רווחים */
    if (*start_position == '\0') {
        *str = '\0';
        return str;
    }
    
    /* מוצא את התו האחרון של המחרוזת, אשר מסמל את סופה */
    end_position = start_position + strlen(start_position) - 1;
    while (end_position > start_position && (is_whitespace(*end_position) || *end_position == '\n' || *end_position == '\r')) 
    {
        end_position--;
    }
    
    /*null ע"מ לסמן את סוף המחרוזת, שם תו */
    *(end_position + 1) = '\0';
    
    /* במידה והטקסט מתחיל ברווח,מזיז את הטקסט לנק' ההתחלה */
    if (start_position != str) {
        memmove(str, start_position, strlen(start_position) + 1);
    }
    
    return str;
}

/*הפו' מקבלת מחרוזת ומחזירה את מיקום המצביע לתו הראשון שאינו רווח *
*הפו' מדלגת על רווחים מבלי לשנות את טיב המחרוזת המקורית */
char* skip_whitespace(const char *str) {
    if (str == NULL) {
        return NULL;
    }
    /*עובר על כלל הרווחים עד שנמצא תו אמיתי*/
    while (is_whitespace(*str)) {
        str++;
    }
    
    return (char*)str;
}

/* פונ' אשר בודקת אם מדובר בשורה ריקה או בהערה*
 *שורה נחשבת לריקה אם אין בה תוים או שיש בה רק רווחים*
 *שורת הערה מתחילה בתו נקודה פסיק*/
int is_empty_or_comment(const char *line) {
    const char *pointer_to_line;
    
    if (line == NULL) {
        return TRUE;
    }
    /*מדלג על רווחים בהתחלה*/
    pointer_to_line = skip_whitespace(line);
    
    /* מדובר בשורה ריקה או בהערה */
    return (*pointer_to_line == '\0' || *pointer_to_line == '\n' || *pointer_to_line == '\r' || *pointer_to_line == COMMENT_CHAR);
}

/* פונ' אשר בודקת אם  שם התווית תקין*
 * תווית תקינה צריכה להתחיל באות ולהכיל רק אותיות מספרים וקו תחתון*
 * לא יכולה להכיל מילה שמורה כמו שם פקודה או רגיסטר*/
int is_valid_label(const char *str) {
    int string_length;
    int loop_index;
    
    if (str == NULL || *str == '\0') {
        return FALSE;
    }
    
    /* התו הראשון חייב להיות אות */
    if (!isalpha(*str)) {
        return FALSE;
    }
    
    /* בודק את אורך המחרוזת שלא עובר את המקסימום המותר */
    string_length = strlen(str);
    if (string_length > MAX_LABEL_LENGTH) {
        return FALSE;
    }
    
    /* עובר על כלל התווים ובודק שהם אותיות מספרים או קו תחתון */
    for (loop_index=0; loop_index < string_length; loop_index = loop_index + 1) 
    {
        if (!isalnum(str[loop_index]) && str[loop_index] != '_') {
            return FALSE;
        }
    }
    
    /* בודק שלא מדובר במילה שמורה שמוגדרת בשפה */
    if (is_reserved_word(str)) {
        return FALSE;
    }
    
    return TRUE;
}

/*בודק אם מחרוזת היא מילה שמורה בשפה אשר אסור להשתמש בה כשם תווית*
*המילים השמורות כוללות פקודות הוראות מקרואים ורגיסטרים */
int is_reserved_word(const char *str) {
    /* רשימת פקודות שמורות */
    static const char *instructions[] = {
        "mov", "cmp", "add", "sub", "lea",
        "clr", "not", "inc", "dec",
        "jmp", "bne", "jsr", "red", "prn",
        "rts", "stop",
        NULL
    };
    
    /* רשימת הוראות שמורות */
    static const char *directives[] = {
        ".data", ".string", ".entry", ".extern",
        NULL
    };
    
    /* מילות מפתח של פקודות מקרואים */
    static const char *macro_keywords[] = {
        "mcro", "mcroend",
        NULL
    };
    
    /* רשימת הרגיסטרים השמורים */
    static const char *registers[] = {
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        NULL
    };
    
    int i;
    
    if (str == NULL) {
        return FALSE;
    }
    
    /* עובר על רשימת הפקודות ובודק תקינות */
    for (i=0; instructions[i] != NULL; i++) {
        if (strcmp(str, instructions[i]) == 0) {
            return TRUE;
        }
    }
    /* עובר על רשימת ההוראות ובודק תקינות */
    for (i=0; directives[i] != NULL; i++) 
    {
        if (strcmp(str, directives[i]) == 0) {
            return TRUE;
        }
    }
    /* עובר על מילות המקרו ובודק תקינות */
    for (i=0; macro_keywords[i] != NULL; i = i + 1) {
        if (strcmp(str, macro_keywords[i]) == 0) {
            return TRUE;
        }
    }
    /* עובר על הרגיסטרים השמורים ובודק תקינות */
    for (i=0; registers[i] != NULL; i++) 
    {
        if (strcmp(str, registers[i]) == 0) {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* פונ' אשר מוציאה את המילה הראשונה מתוך המחרוזת*
*המילה מסתיימת כאשר יש רווח פסיק או נקודותיים*
*הפו' מחזירה מצביע להמשך המחרוזת  */
char* extract_word(char *str, char *destination_buffer, int maximum_length) {
    int character_index;
    char *start_of_word;
    
    if (str == NULL || destination_buffer == NULL) {
        return NULL;
    }
    
    /* מדלג על רווחים בתחילת המחרוזת */
    start_of_word = skip_whitespace(str);
    
    /*  מעתיק את המילה תו אחי תו לתוך מערך יעד שהפונ' קיבלה כפרמטר
     destination_buffer המילה תועתק למערך היעד
      *הלואלה מפסיקה עד אשר המילה מסתיימת ומגיעים לרווח או תו מיוחד*/
    character_index = 0;
    while (start_of_word[character_index] != '\0' && 
        !is_whitespace(start_of_word[character_index]) && 
        start_of_word[character_index] != '\n' &&
        start_of_word[character_index] != '\r' &&
        start_of_word[character_index] != LABEL_END &&
        start_of_word[character_index] != OPERAND_SEPARATOR &&
        character_index < maximum_length - 1) 
    {
        destination_buffer[character_index] = start_of_word[character_index];
        character_index++;
    }
    
    destination_buffer[character_index] = '\0';
    
    /* מחזיר מצביע לשאר המחרוזת */
    return start_of_word + character_index;
}

/* מעתיק מחרוזת בצורה בטוחה ממקום למקום
  שבמידה והמחרוזת ארוכה מדי היא תמשיך לכתוב מעבר לסוף המערך strcpy הבעיה עם הפונקציה 
   בסוף null פונקציה זו דואגת לעצור בזמן ולשים ערך*/
void safe_strcpy(char *destination, const char *source, int max_size) {
    if (destination == NULL || source == NULL || max_size <= 0) {
        return;
    }
    
    strncpy(destination, source, max_size - 1);
    destination[max_size - 1] = '\0';
}
