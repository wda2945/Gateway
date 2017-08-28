//
//  main_debug.h
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef main_debug_h
#define main_debug_h

extern FILE *mainDebugFile;

void print_debug_message_to_file(FILE *dbgfile, const char *text);

#define DEBUGPRINT(...) {char tmp[PS_MAX_LOG_TEXT];\
    snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
    tmp[PS_MAX_LOG_TEXT-1] = 0;\
    print_debug_message_to_file(mainDebugFile, tmp);\
    print_debug_message_to_file(stdout, tmp);}

#define ERRORPRINT(...) {char tmp[PS_MAX_LOG_TEXT];\
    snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
    tmp[PS_MAX_LOG_TEXT-1] = 0;\
    print_debug_message_to_file(mainDebugFile, tmp);\
    print_debug_message_to_file(stderr, tmp);}

#endif /* main_debug_h */
