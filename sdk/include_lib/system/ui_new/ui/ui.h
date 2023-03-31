#ifndef UI_CORE_H
#define UI_CORE_H

#include "window.h"
#include "ui_button.h"
#include "ui_grid.h"
#include "ui_time.h"
#include "ui_camera.h"
#include "ui_pic.h"
#include "ui_text.h"
#include "ui_battery.h"
#include "ui_browser.h"
#include "ui_slider.h"
#include "ui_slider_vert.h"
#include "ui_number.h"
#include "ui_watch.h"
#include "ui_progress.h"
#include "ui_progress_multi.h"
#include "ui_rotate.h"
#include "ui_page_manager.h"
#include "ui_compass.h"
#include <stdarg.h>

struct uimsg_handl {
    const char *msg;
    int (*handler)(const char *type, u32 args);
};

enum {
    HIDE_WITH_REDRAW,
    HIDE_WITHOUT_REDRAW,
};

int ui_framework_init(void *);

int ui_set_style_file(struct ui_style *style);

int ui_style_file_version_compare(int version);

int ui_redraw(int id);

int ui_show(int id);

int ui_hide(int id);

int ui_set_call(int (*func)(int), int param);

int ui_event_onkey(struct element_key_event *e);

int ui_event_ontouch(struct element_touch_event *e);

int ui_card_ontouch(struct element_touch_event *e);

struct element *ui_core_get_root();

struct element *ui_get_highlight_child_by_id(int id);
int ui_invert_element_by_id(int id);

int ui_no_highlight_element(struct element *elm);
int ui_no_highlight_element_by_id(int id);
int ui_highlight_element(struct element *elm);
int ui_highlight_element_by_id(int id);

int ui_get_current_window_id();

int ui_page_switch(int curr_win, int next_win, int xoffset, int mode);

int ui_register_msg_handler(int id, const struct uimsg_handl *handl);

int ui_message_handler(int id, const char *msg, va_list);

const char *str_substr_iter(const char *str, char delim, int *iter);

int ui_get_child_by_id(int id, int (*event_handler_cb)(void *, int, int));

int ui_set_default_handler(struct element *elm, int (*ontouch)(void *, struct element_touch_event *),
                           int (*onkey)(void *, struct element_key_event *),
                           int (*onchange)(void *, enum element_change_event, void *));

/*
 * 锁定元素elm之外的区域，所有的触摸消息都发给elm
 */
void ui_ontouch_lock(void *elm);
void ui_ontouch_unlock(void *elm);

/*
 * 锁定控件的夫图层，先不推向imb显示
 */
int ui_lock_layer(int id);
int ui_unlock_layer(int id);

int ui_get_disp_status_by_id(int id);

int create_control_by_id(char *tabfile, int page_id, int id, int parent_id);
int delete_control_by_id(int id);


void ui_remove_backcolor(struct element *elm);
void ui_remove_backimage(struct element *elm);
void ui_remove_border(struct element *elm);

int ui_fill_rect(struct draw_context *dc, int left, int top, int width, int height, u32 acolor);
int ui_draw_image(struct draw_context *dc, int page, int id, int x, int y);
/*----------------------------------------------------------------------------*/
/**@brief    大图片截取显示
   @param    dc:显示上下文
   @param    page:页面号
   @param    id:图片索引
   @param    x:显示的x坐标(相对屏幕)
   @param    y:显示的y坐标(相对屏幕)
   @param    width:显示的宽度
   @param    height:显示的高度
   @param    image_x:图片的x坐标(相对图片)
   @param    image_y:图片的y坐标(相对图片)
   @return   -1:失败 0：正常
   @note     大图片显示具有尺寸限制，请留意工具或者代码提示信息
*/
/*----------------------------------------------------------------------------*/
int ui_draw_image_large(struct draw_context *dc, int page, int id, int x, int y, int width, int height, int image_x, int image_y);
int ui_draw_ascii(struct draw_context *dc, char *str, int strlen, int x, int y, int color);
int ui_draw_text(struct draw_context *dc, int encode, int endian, char *str, int strlen, int x, int y, int color);
int ui_draw_strpic(struct draw_context *dc, int id, int x, int y, int color);
void ui_draw_line(void *_dc, int x0, int y0, int x1, int y1, int color);
void ui_draw_line_by_angle(void *_dc, int x, int y, int length, int angle, int color);
void ui_draw_rect(void *_dc, int x, int y, int width, int height, int color);
void ui_draw_circle(struct draw_context *dc, int center_x, int center_y,
                    int radius_big, int radius_small, int angle_begin,
                    int angle_end, int color, int percent);
void ui_draw_ring(void *_dc, int x, int y, int radius_big, int radius_small, int angle_begin, int angle_end, int color, int percent);
void ui_draw_bar(void *_dc, int x, int y, int width, int height, int color, int percent);
int ui_draw_set_pixel(struct draw_context *dc, int x, int y, int pixel);
u32 ui_draw_get_pixel(struct draw_context *dc, int x, int y);
u16 ui_draw_get_mixed_pixel(u16 backcolor, u16 forecolor, u8 alpha);
void ui_custom_draw_clear(struct draw_context *dc);
void *load_control_info_by_id(char *tabfile, u32 page_id, u32 id);
void *ui_control_new(void *_pos, void *parent);
int ui_draw_begin(struct draw_context *dc);
int ui_draw_end(struct draw_context *dc);
int ui_draw(struct draw_context *dc, u8 *buf, int x, int y, int width, int height, void *cb, void *priv, int priv_len, int id);
void ui_page_set_param(int threshold, int step, int delay);
int ui_hide_set(int id, int redraw);

#define UI_SLIDE_EN 1

#define RGB565(r,g,b) (((((u8)r)>>3)<<11)|((((u8)g)>>2)<<5)|(((u8)b)>>3))

#endif

