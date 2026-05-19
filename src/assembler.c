/*
 * assembler.c
 * נקודת הכניסה הראשית לתוכנית האסמבלר
 * 
 * הקובץ מכיל את הפונקציה הראשית שמנהלת את כל תהליך האסמבלי
 * התוכנית עוברת על רשימת קבצים שהמשתמש נותן ומעבדת כל אחד דרך 4 שלבים
 * 
 * 1) פריסת מקרואים- קוראת את קובץ המקור ופורסת את המקרואים בהתאם
 * 2)מעבר ראשון- בונה טבלת סמלים ומחשבת גודל כל פקודה
 * 3) מעבר שני- מילוי הכתובות הסופיות של הסמלים וקידוד הפקודות
 * 4)יצירת פלט- נוצרים הקבצים הסופיים ob,ent,ext
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "structures.h"
#include "errors.h"
#include "macro.h"
#include "first_pass.h"
#include "second_pass.h"
#include "output.h"
#include "globals.h"

int process_single_assembly_file(const char *base_filename);

/*
 * פונקציה ראשית
 * מקבלת רשימת קבצים מהטרמינל ועוברת על כל אחד בתורו
 * בסוף מדפיסה סיכום של כמה קבצים הצליחו וכמה נכשלו
 */
int main(int argc, char *argv[]) {
    int i;
    int errors_count = 0;
    int successful_count;
    
    /* בודק שהמשתמש נתן לפחות קובץ אחד */
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <file1> <file2> ... <fileN>\n", argv[0]);
        fprintf(stderr, "Note: Provide filenames without the .as extension\n");
        return ERROR;
    }
    
    /* מדפיס כותרת בהתחלה כדי שהמשתמש יראה שהתוכנית התחילה לרוץ */
    printf("Assembler started. Processing %d file(s)...\n\n", argc - 1);
    
    /* לולאה שעוברת על כלל הקבצים שהתקבלו ע"י המשתמש */
    for (i=1; i < argc; i++) 
    {
        printf("Processing: %s.as\n", argv[i]);
        
        /* עיבוד הקובץ ע"י התכנית- ונבדק סטאטוס (הצלחה כישלון) */
        if(process_single_assembly_file(argv[i]) == ERROR) {
            errors_count = errors_count + 1; 
            printf(" Failed to process %s.as\n\n", argv[i]);
        } else 
        {
            printf(" Successfully processed %s.as\n\n", argv[i]);
        }
    }
    
    /* חישוב כמות קבצים שעבדו ע"י הפחתת מספר השגיאות מהסכום */
    successful_count = argc - 1 - errors_count;
    
    /* הדפסת הסיכום הסופי עבור המשתמש  */
    printf("Assembly complete.\n" ) ;
    printf("Files processed: %d\n", argc-1 );
    printf("Successful: %d\n", successful_count );
    printf("Failed: %d\n", errors_count);
    
    /* החזרת קוד שגיאה למערכת הפעלה במידה והיו שגיאות */
    return (errors_count > 0) ? ERROR : SUCCESS;
}

/*
 * פונקציה אשר מעבדת קובץ אסמבלי יחיד דרך כל ארבעת השלבים
 *  אם אחד הפונקציה נכשלת באחד מהשלבים היא עוצרת ומחזירה שגיאה
 */
int process_single_assembly_file(const char *filename) {
    char source_file_name[256];
    char after_macro_file_name[256];
    int stage_result;
    SymbolTable symbol_table;
    MemoryImage memory;
    ExternalList ext_list;
    
    /* אתחול כל המבנים שצריכים לעבודה */
    symbol_table_init(&symbol_table);
    ext_list.head = NULL;
    
    ext_list.count = 0;
    
    /* מייצרת שמות לקבצים המלאים עם הסיומות הנכונות */
    sprintf(source_file_name, "%s%s", filename, EXT_SOURCE);
    sprintf(after_macro_file_name, "%s%s", filename, EXT_MACRO);
    
    /* שלב ראשון: פריסת מקרואים */
    printf("Stage 1: Macro expansion...\n");
    stage_result = expand_macros(source_file_name, after_macro_file_name);
    if(stage_result == ERROR) {
        fprintf(stderr, "Error: Macro expansion failed for %s\n", source_file_name);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Generated %s\n", after_macro_file_name);
    
    /* שלב שני: מעבר ראשון שבונה את טבלת הסמלים */
    printf("Stage 2: First pass...\n");
    stage_result = first_pass(after_macro_file_name, &symbol_table, &memory);
    if (stage_result == ERROR){
        fprintf(stderr, "Error: First pass failed for %s\n", after_macro_file_name);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Symbol table built\n");
    printf("  → IC = %d, DC = %d\n", memory.IC, memory.DC);
    
    /* הדפסת טבלת הסמלים  */
    symbol_table_print(&symbol_table);
    
    /* שלב שלישי: מעבר שני מילוי כתובות וקידוד */
    printf("Stage 3: Second pass...\n");
    stage_result = execute_second_pass(after_macro_file_name, &symbol_table, &memory, &ext_list);
    if(stage_result == FALSE) 
    {
        fprintf(stderr, "Error: Second pass failed for %s\n", after_macro_file_name);
        symbol_table_free(&symbol_table);
        return ERROR;
    }
    printf("  → Encoding completed\n");
    
    /* שלב רביעי: יצירת כלל קבצי הפלט הסופיים */
    printf("Stage 4: Generating output files...\n");
    create_object_file(filename, &memory) ;
    create_entries_file(filename, &symbol_table) ;
    create_externals_file(filename, &ext_list);
    printf ("  → Output files generated\n");
    
    /* שחרור הזיכרון שהקצינו ע"מ למנוע זליגות זיכרון */
    symbol_table_free(&symbol_table);
    
    return SUCCESS;
}
