/*
 * second_pass.c
 * המעבר השני של האסמבלר
 * 
 * המעבר השני הוא השלב שבו הסמלים מקבלים את כתובתם האמיתית
 * במעבר הראשון בנינו את טבלת הסמלים ולאחר המעבר השני מתקבלת תמונת המצב המדויקת של כלל הכתובות
 * כך שכל תווית/סמל תיוצג ע"י קוד מכונה
 * 
 * :השלבים שנעשים במעבר השני 
 * 1. עובר על הקובץ בשנית שורה אחרי שורה
 * 2. כאשר מזוהה פקודה עם סמל ממלא את הכתובת האמיתית שלו
 * 3.ומסמן את הסמלים המתאימים entry מטפל בהוראות מסוג 
 * 4. נשמרת רשימה של כלל המקומות שבהם השתמשנו בסמלים חיצוניים
 * 
 * בסוף המעבר השני הזיכרון מכיל את כלל המידע הנחוץ ואת הקוד מקודד ומוכן ליצירת קבצי הפלט
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "second_pass.h"
#include "symbol_table.h"
#include "parser.h"
#include "file_handler.h"
#include "operand_encoder.h"
#include "string_utils.h"
#include "errors.h"
#include "first_pass.h"
#include "globals.h"

/* משתנים גלובליים שמשמשים את כלל הפונקציות במעבר השני
מוגדרים כסטטיים ע"מ שלא תהיה אליהם גישה מקבצים אחרים */
static SymbolTable *symbol_table_pointer;
static MemoryImage *memory_image_pointer;
static ExternalList *externals_list_pointer;
static int instruction_counter;
static int error_flag = 0;
static ExternalRef *externals_linked_list_head = NULL;

/*
 * מוסיפה רשומה חדשה לרשימת ההפניות לסמלים חיצוניים
 * סמל חיצוני מוגדר בקובץ אחר אך התכנית שלנו משתמשת בו   
 * בכל פעם שאנו משתמשים בסמל חיצוני נדרש לשמור את מיקומו 
 * ext ע"מ שלאחר מכן נוכל לכתוב את מיקומו לקובץ 
 */
static void add_external_reference(const char *symbol_name, int memory_address) {
    ExternalRef *new_reference;
 /* מקצה זיכרון לרשומה החדשה */   
    new_reference = (ExternalRef*)malloc(sizeof(ExternalRef));
    if (new_reference == NULL) {
        return;
    }
 /* שומר את שם הסמל החיצוני שהשתמשנו בו */   
    strcpy(new_reference->symbol, symbol_name);
 
    /* שומר את הכתובת בזיכרון שבה השתמשנו בסמל הזה */  
    new_reference->address = memory_address;
 
    /* מוסיף את הרשומה לראש הרשימה המקושרת */ 
    new_reference->next = externals_linked_list_head;
    externals_linked_list_head = new_reference;

    /* מעדכן מספר הפניות חיצוניות אשר נדרשנו להשתמש בהם בקובץ */
    externals_list_pointer->count = externals_list_pointer->count + 1;
}

/*
 *  מקודדת כתובת ישירה של תווית לתוך הזיכרון
 * jmp LABEL השימוש בפונ' מתרחש כאשר יש פקודה כמו 
 * הפונקציה מחפשת את התווית בטבלה וממלאת את הכתובת העדכנית שלה
 */
static void encode_direct_address(const char *label_name, int line_number) {
    Symbol *found_symbol;

    /* בדיקת תקינות שם התווית*/
    if (label_name == NULL || *label_name == '\0') {
        return;
    }
    
    /* שלב 1: חיפוש התווית בטבלת הסמלים */
    found_symbol = symbol_table_find(symbol_table_pointer, label_name);
    if (found_symbol == NULL) 
    {
        /* אם לא מצאנו את הסמל מדובר בשגיאה */
        print_error(line_number, ERR_UNDEFINED_LABEL, label_name);
        error_flag = 1;
        return;
    }
    
    /* שלב 2: מקודד את הכתובת לפי סוג הסמל */
    if (found_symbol->attributes & ATTR_EXTERNAL) {
        
        /* אם מדובר בסמל חיצוני - שמים 0 כי הכתובת תמולא בזמן הקישור */
        memory_image_pointer->code[instruction_counter].word = 0;
        memory_image_pointer->code[instruction_counter].are = ARE_EXTERNAL;
        
        /* מציינים שהשתמשנו בסמל החיצוני במיקום זה   */
        add_external_reference(label_name, instruction_counter);
    } else 
    {
        /* במידה ומדובר בסמל רגיל מהקובץ שמים את הכתובת המקורית שלו */
        memory_image_pointer->code[instruction_counter].word = found_symbol->value & 0xFFF;
        memory_image_pointer->code[instruction_counter].are = ARE_RELOCATABLE;
    }
   
    /* שלב 3: מקדם את המונה למילה הבאה בזיכרון */
    instruction_counter = instruction_counter + 1;
}
/*
 * מקודדת כתובת יחסית, כאשר כתובת יחסית מציינת את ההפרש מהמיקום הנוכחי
 * כתובת יחסית מסומת ע"י סימן % בתחילת התווית
 * נשתמש בכתובת יחסית מכיוון שניתן להריץ אותה בכל מקום בזיכרון
 */
static void encode_relative_address(const char *label_with_percent, int line_number) {
    Symbol *found_symbol;
    char clean_label_name[MAX_LABEL_LENGTH + 1];
    int offset_value;
    
    if (label_with_percent == NULL || *label_with_percent == '\0') {
        return;
    }
    
    /* מוריד את ה-% מההתחלה ע"מ לקבל את שם הסמל הנקי */
    strcpy(clean_label_name, label_with_percent + 1);
   
    /* מחפש את הסמל בטבלה */
    found_symbol = symbol_table_find(symbol_table_pointer, clean_label_name);
    if (found_symbol == NULL) {
        print_error(line_number, ERR_UNDEFINED_LABEL, clean_label_name);
        error_flag = 1;
        return;
    }
    
    /* מחשב את ההפרש בין הכתובת של הסמל לכתובת הנוכחית */
    offset_value = found_symbol->value - instruction_counter;
    
    /* שומר את ההפרש בזיכרון */
    memory_image_pointer->code[instruction_counter].word = offset_value & 0xFFF;
    memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
    
     /* מתקדם למילה הבאה */
    instruction_counter = instruction_counter + 1;
}

/*
 * entry מטפלת בהוראת 
 * ההוראה הזאת מסמנת סמל כציבורי כדי שקבצים אחרים יוכלו להשתמש בו
 */
static void handle_entry_directive(const char *parameters, int line_number) {
    char label_name[MAX_LABEL_LENGTH + 1];
    Symbol *found_symbol;
    
    /* בודק שקיים פרמטר עבור ההוראה */
    if (parameters == NULL || *parameters == '\0') 
    {
        print_error(line_number, ERR_EMPTY_DIRECTIVE, NULL);
        error_flag = 1;
        return;
    }
    
    /* מעתיק את שם הסמל ומנקה רווחים */
    strcpy(label_name, parameters);
    trim(label_name);
    
    /*מחפש את הסמל בטבלת הסמלים */
    found_symbol = symbol_table_find(symbol_table_pointer, label_name);
    if (found_symbol == NULL) {
        print_error(line_number, ERR_UNDEFINED_LABEL, label_name);
        error_flag = 1;
        return;
    }
    
    /* בודק שהסמל לא מסומן כחיצוני 
     extern וגם entry אין אפשרות שסמל יסומן  */
    if (found_symbol->attributes & ATTR_EXTERNAL) 
    {
        print_error(line_number, ERR_NONE, "Cannot mark external symbol as entry");
        error_flag = 1;
        return;
    }
    
    /* entry-מסמן את הסמל כ */
    symbol_table_update_attributes(symbol_table_pointer, label_name, ATTR_ENTRY);
}

/*
 * מטפלת בקידוד פקודת אסמבלי 
 * פונ' מרכזית של המעבר השני, אשר מקבלת פקודה ומקודדת את האופרנדים שלה
 * הפונקציה הזאת היא הלב של המעבר השני
 */
static void handle_instruction_encoding(const char *operation_name, const char *operands_string, int line_number) {
    char source_operand[MAX_LINE_LENGTH];
    char destination_operand[MAX_LINE_LENGTH];
    char operands_copy[MAX_LINE_LENGTH];
    int number_of_operands, opcode_value, funct_value, expected_operands, parsed_value;
    int source_register_number, destination_register_number;
    AddressingMode source_addressing, destination_addressing;

    /* שלב 1: מקבל מידע על הפקודה - קוד הפעולה ומספר אופרנדים */
    if (!get_operation_info(operation_name, &opcode_value, &funct_value, &expected_operands)) {
        return;
    }
    
    /* מדלג על המילה הראשונה אשר כבר קודדה במעבר הראשון */
    instruction_counter = instruction_counter + 1;
    
    /* שלב 2: מנתח את האופרנדים ומפריד אותם */
    strcpy(operands_copy, operands_string);
    number_of_operands = parse_operands(operands_copy, source_operand, destination_operand, MAX_LINE_LENGTH);
    
    /* בודק שמספר האופרנדים תואם למספר שהפקודה צריכה */
    if (number_of_operands != expected_operands) {
        return;
    }
    
    /* אם אין אופרנדים סיימנו */
    if (expected_operands == 0) {
        return;
    }

    /* שלב 3: מקודד את האופרנדים לפי מספרם
    מטפל בפקודה עם 2 אופרנדים */
    if (expected_operands == 2) 
    {
        /* בודק את סוג הכתובת של כל אופרנד */
        source_addressing = identify_addressing_mode(source_operand);
        destination_addressing = identify_addressing_mode(destination_operand);
        
        /* מקרה מיוחד - שני רגיסטרים */
        /* במקרה זהע"פ הוראות הפרויקט שני הרגיסטרים מקודדים בשתי מילים נפרדות */
        if (source_addressing == MODE_REGISTER && destination_addressing == MODE_REGISTER) 
        {
            /* מנתח את מספרי הרגיסטרים */
            parse_register(source_operand, &source_register_number);
            parse_register(destination_operand, &destination_register_number);
            
            /* כותב את רגיסטר המקור במילה אחת */
            memory_image_pointer->code[instruction_counter].word = encode_register(source_register_number);
            memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
            instruction_counter = instruction_counter + 1;
            
            /* כותב את רגיסטר היעד במילה השניה */
            memory_image_pointer->code[instruction_counter].word = encode_register(destination_register_number);
            memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
            instruction_counter = instruction_counter + 1;
        } 
        else 
        {
            /* מקודד את אופרנד המקור לפי סוג הכתובת שלום */
            /*מדובר בכתובת ישירה סמל רגל*/
            if (source_addressing == MODE_DIRECT) {
                encode_direct_address(source_operand, line_number);
            } 
            /*% מדובר בכתובת יחסית עם סמל */
            else if (source_addressing == MODE_RELATIVE) {
                encode_relative_address(source_operand, line_number);
            } 
            /*מדובר במספר קבוע ,ערך קבוע שלא משתנה ואינו תלוי בכתובת*/
            else if (source_addressing == MODE_IMMEDIATE) 
            {
                /*מוריד את סימן הסולמית ומפענח את המספר*/
                parse_integer(source_operand + 1, &parsed_value);
                
                /* מקודד את המספר לפורמט של 12 ביט */
                memory_image_pointer->code[instruction_counter].word = encode_immediate(parsed_value);
                
                /* מסמן שמדובר בערך מוחלט שלא צריך שינוי */
                memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
                
                /* מתקדם למילה הבאה בזיכרון */
                instruction_counter = instruction_counter + 1;
              /*כאשר מדובר רק ברגיסטר 1 ולא 2*/  
            } else if (source_addressing == MODE_REGISTER) {
                
                /* מפענח את מספר הרגיסטר */
                parse_register(source_operand, &parsed_value);
                
                /* מקודד את הרגיסטר לביטים המתאימים */
                memory_image_pointer->code[instruction_counter].word = encode_register(parsed_value);
                
                /* מסמן שמדובר בערך מוחלט שלא צריך שינוי */
                memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
                
                /* מתקדם למילה הבאה בזיכרון */
                instruction_counter = instruction_counter + 1;
            }

        /*  מקודד את האופרנד היעד באופן זהה לקידוד אופרנד המקור */
            /* כתובת ישירה */
            if (destination_addressing == MODE_DIRECT) {
                encode_direct_address(destination_operand, line_number);
              /* כתובת יחסית */  
            } else if (destination_addressing == MODE_RELATIVE) {
                encode_relative_address(destination_operand, line_number);

              /* האופרנד הוא ערך מיידי */  
            } else if (destination_addressing == MODE_IMMEDIATE) 
            {
                /* מוריד את סימן ה-# ומפענח את המספר */
                parse_integer(destination_operand + 1, &parsed_value);
                
            /* מקודד את המספר בפורמט 12 ביט */
                memory_image_pointer->code[instruction_counter].word = encode_immediate(parsed_value);
                
                /* מסמן שמדובר בערך מוחלט שלא צריך שינוי */
                memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
                
                /* מתקדם למילה הבאה בזיכרון */
                instruction_counter = instruction_counter + 1;

              /*המקרה האחרון מדובר ברגיסטר
              אם האופרנד לא ישיר/יחסי/מיידי אז הוא בהכרח רגיסטר*/  
            } else if (destination_addressing == MODE_REGISTER) 
            {
                /* מפענח את שם הרגיסטר ומקבל את מספרו */
                parse_register(destination_operand, &parsed_value);
                
                /* מקודד את הרגיסטר */
                memory_image_pointer->code[instruction_counter].word = encode_register(parsed_value);
                
                /* מסמן שמדובר בערך מוחלט שלא צריך שינוי */
                memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
                 /* מתקדם למילה הבאה בזיכרון */
                instruction_counter = instruction_counter + 1;
            }
        }
    } 
    /* מטפל בפקודה עם אופרנד אחד 
    בודק את סוג הכתובת של האופרנד היחיד*/
    else if (expected_operands == 1) {
        destination_addressing = identify_addressing_mode(destination_operand);
        /*מקודד את האופרנד לפי סוגו
        קידוד מתבצע באופן זהה,לדרך שבה מימשנו את אופרנד היעד בפקודה עם 2 אופרנדים*/
        
        /*כתובת ישירה-סמל רגיל
        קוראים לפונקציה אשר מחפשת את הסמל בטבלה וממלאת את כתובתה*/
        if (destination_addressing == MODE_DIRECT) {
            encode_direct_address(destination_operand, line_number);
        }
        /*מדובר בכתובת יחסית, קוראים לפונקציה שמחשבת את ההפרש מהמיקום הנוכחי*/
        else if (destination_addressing == MODE_RELATIVE) {
            encode_relative_address(destination_operand, line_number);
        } 
        /* ערך מיידי - מספר קבוע */
        else if (destination_addressing == MODE_IMMEDIATE) {
            /* מוריד את סימן הסולמית ומפענח את המספר */
            parse_integer(destination_operand + 1, &parsed_value);
             /* מקודד את המספר לפורמט של 12 ביט */
            memory_image_pointer->code[instruction_counter].word = encode_immediate(parsed_value);
            /* מסמן שזה ערך מוחלט שלא משתנה */
            memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
            /* מתקדם למילה הבאה */
            instruction_counter = instruction_counter + 1;

        /*מדובר ברגיסטר*/
        } else if (destination_addressing == MODE_REGISTER) 
        {
            /* מפענח את מספר הרגיסטר */
            parse_register(destination_operand, &parsed_value);
            /* מקודד את הרגיסטר */
            memory_image_pointer->code[instruction_counter].word = encode_register(parsed_value);
            /* מסמן שזה ערך מוחלט שלא משתנה */
            memory_image_pointer->code[instruction_counter].are = ARE_ABSOLUTE;
            /* מתקדם למילה הבאה */
            instruction_counter = instruction_counter + 1;
        }
    }
}

/*
 * הפונקציה הראשית של המעבר השני
 במעבר הראשון עברנו על כלל הקובץ ובנינו טבלת סמלים, כאשר קידדנו רק את המילה הראשונה של כל פקודה
 אך נשארו חורים בכתובות, כעת במעבר השני אמו ממלאים את החורים
 * הפונ' עוברת על כל הקובץ שורה אחרי שורה ומקודדת את כל הסמלים
 */
bool execute_second_pass(const char *filename, SymbolTable *symbols, MemoryImage *mem, ExternalList *externals) {
    FILE *input_file;
    char line_buffer[MAX_LINE_LENGTH];
    char label_from_line[MAX_LABEL_LENGTH + 1];
    char directive_name[MAX_LABEL_LENGTH + 1];
    char operation_name[MAX_LABEL_LENGTH + 1];
    char directive_parameters[MAX_LINE_LENGTH];
    char instruction_operands[MAX_LINE_LENGTH];
    char *remaining_line_after_label, *trimmed_line;
    int current_line_number = 0;
    int found_error = FALSE;
    LineType type_of_line;

    /*אתחול המשתנים הגלובליים , אשר מותפים לכלל הפונקציות בקובץ זה */
    symbol_table_pointer = symbols;
    memory_image_pointer = mem;
    externals_list_pointer = externals;
    /* מאתחל את מונה הפקודות לכתובת ההתחלה */
    instruction_counter = INITIAL_IC;
    /* מאתחל רשימה ריקה להפניות חיצוניות */
    externals_linked_list_head = NULL;
    /* מאתחל את דגל השגיאות */
    error_flag = 0;

    /* לקריאה am-פותח את קובץ ה 
    הקובץ שמתקבל אחרי פריסת המקרואים*/
    input_file = fopen(filename, "r");
    if (input_file == NULL) 
    {
        /*אם הקובץ לא נפתח נחזיר שגיאה*/
        print_error(0, ERR_FILE_OPEN, filename);
        return FALSE;
    }

    /* לולאה שעוברת על כל שורות הקובץ */
    while (fgets(line_buffer, MAX_LINE_LENGTH, input_file)) {
        /*מעדכן את מונה השורות,במידה ויש שגיאה שנוכל להחזיר את מספר השורה העדכני */
        current_line_number = current_line_number + 1;
        
        /* מצביע על השורה שקראנו */
        trimmed_line = line_buffer;
        /* מנקה רווחים טאבים ושורות חדשות מההתחלה והסוף */
        trim(trimmed_line);
        
        /* מדלג על שורות ריקות והערות */
        if (is_empty_or_comment(trimmed_line)) {
            continue;
        }
        
        /* מזהה את סוג השורה - האם זו הוראה פקודה או משהו אחר */
        type_of_line = parse_line_type(trimmed_line);
        if (type_of_line == LINE_EMPTY) {
            continue;
        }
        
        /* מנתח את התווית אם יש כזאת בשורה
         *תוויות שכבר טופלו במעבר הראשון נדלג עליהן
         * הפונ' מחזירה מצביע למיקום לאחר התווית*/
        remaining_line_after_label = parse_label(trimmed_line, label_from_line, MAX_LABEL_LENGTH + 1);
        trimmed_line = remaining_line_after_label;
        trim(trimmed_line);

        /* בודק האם מדובר בשורת הוראה*/
        if (type_of_line == LINE_DIRECTIVE) 
        {
            /* מפרק את ההוראה לשם ופרמטרים */
            if (parse_directive(trimmed_line, directive_name, directive_parameters, MAX_LINE_LENGTH)) {
                /* רלוונטי רק למעבר השני entry
                (תווית חיצונית)*/
                if (strcmp(directive_name, DIR_ENTRY) == 0) 
                {
                    /* מסמן את הסמל בטבלהentry מטפל בהוראת */
                    handle_entry_directive(directive_parameters, current_line_number);
                }
            }
        } 
        /* בודק אם מדובר בשורת פקודה*/
        else if (type_of_line == LINE_INSTRUCTION) 
        {
            /*מקודד את כל האופרנדים של הפקודה
             *פונ' זו ממלאת את כל הכתובות האמיתיות*/
            if (parse_instruction(trimmed_line, operation_name, instruction_operands, MAX_LINE_LENGTH)) {
                handle_instruction_encoding(operation_name, instruction_operands, current_line_number);
            }
        }
    }
    /*סוגר את הקובץ אחרי שסיימנו לעבוד איתו*/
    fclose(input_file);
    
    /* שומר את רשימת ההפניות החיצוניות */
    externals_list_pointer->head = externals_linked_list_head;

    /* בודק אם במהלך המעבר השני היו שגיאות
    במידה וכן מתעדכן המשתנה אשר מסמל דגל בדיקה לשגיאות*/
    if (error_flag) {
        found_error = TRUE;
    }
    /*אם היו שגיאות תחזיר שהמעבר נכשל אחרת תחזיר שהכל תקין*/
    return found_error ? FALSE : TRUE;
}
