#ifndef __UI_PAGE_SWITCH_H__
#define __UI_PAGE_SWITCH_H__

enum {
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
};

void ui_page_set_param(int threshold, int step, int delay);
int ui_page_move(int curr_win, int xoffset, int yoffset, int mode);
int ui_page_switch(int curr_win, int next_win, int xoffset, int mode);
int ui_page_move_auto(int curr_win, int mode);
u8 ui_page_move_callback_run(void);
int get_direction(int startx, int starty, int endx, int endy);


#endif
