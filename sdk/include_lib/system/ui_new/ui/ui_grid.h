#ifndef UI_GRID_H
#define UI_GRID_H


#include "ui/ui_core.h"
#include "ui/control.h"

enum {
    GRID_SCROLL_MODE,
    GRID_PAGE_MODE,
};

enum {
    SCROLL_DIRECTION_NONE,
    SCROLL_DIRECTION_LR,
    SCROLL_DIRECTION_UD,
};
typedef enum {
    NO_AUTO_CENTER,
    AUTO_CENTER_MODE1,//滑动惯性距离小于1/2屏幕时回弹
    AUTO_CENTER_MODE2,//滑动惯性较小时依然能滑向下一项
    AUTO_CENTER_MODE3,//高亮项居中，适用于列表
    AUTO_CENTER_MODE4,//基于触摸项的滑动
    AUTO_CENTER_MODE5,//匀减速
    AUTO_CENTER_CUSTOM,//自定义滑动处理
} AUTO_CENTER_MODE;
struct ui_grid_item_info {
    u8 row;
    u8 col;
    u8 page_mode;
    u8 highlight_index;
    u16 interval;
    struct layout_info *info;
};

struct scroll_area {
    int left;
    int top;
    int right;
    int bottom;
};


struct ui_grid_dynamic {
    int  dhi_index;
    int  dcol_num;
    int  drow_num;

    int  min_row_index;
    int  max_row_index;
    int  min_col_index;
    int  max_col_index;
    int  min_show_row_index;
    int  max_show_row_index;
    int  min_show_col_index;
    int  max_show_col_index;

    int  grid_xval;
    int  grid_yval;
    u8   grid_col_num;
    u8   grid_row_num;
    u8   grid_show_row;
    u8   grid_show_col;
    int  base_index_once;
    int init_step_once;
};

struct ui_grid {
    struct element elm;
    // char hi_num;
    char hi_index;
    char touch_index;
    char cur_dindex;
    char onfocus;
    u8   page_mode;
    u8   slide_direction;
    u8   col_num;
    u8   row_num;
    u8   show_row;
    u8   show_col;
    u8   avail_item_num;
    u8   pix_scroll;
    u8   ctrl_num;
    u8   page;
    u8   child_init;
    u8   rotate;	// 垂直列表转为旋转列表标志
    int  x_interval;
    int  y_interval;
    int  max_show_left;
    int  max_show_top;
    int  min_show_left;
    int  min_show_top;
    int  max_left;
    int  max_top;
    int  min_left;
    int  min_top;
    // int  scroll_step;
    // u8   ctrl_num;
    int energy_timer;
    float energy_vx0;
    float energy_vy0;
    float energy_a;
    float energy_val;
    u8 energy_xdir;
    u8 energy_ydir;
    u8 energy_status;
    u8 energy_tslide;


    u8 flick_endflag;
    u8 flick_status;
    u16 flick_cmpsize;
    int flick_timer;
    int flick_distance;
    int flick_overdis;
    int flick_resdis;
    float flick_v0;

    u16 center_target_line;		//居中目标中线位置：0-10000
    u16 center_next_threshold;	//滑入下一项阈值：0-10000
    u8 center_item_offset;	//居中项偏移阈值:0-(avail_item_num-1)
    u8 center_index_mode;		//高亮项or居中项
    u8 auto_center_enable;
    u8 flick_close;

    struct element_luascript_t *lua;
    struct scroll_area *area;
    struct layout *item;
    struct layout_info *item_info;
    // struct element elm2;
    struct ui_grid_dynamic *dynamic;
    struct position pos;
    struct draw_context dc;
    struct element_touch_event *e;
    const struct ui_grid_info *info;
    const struct element_event_handler *handler;
    int key_jump;
};

extern const struct element_event_handler grid_elm_handler;

static inline int ui_grid_touch_item(struct ui_grid *grid)
{
    return grid->touch_index;
}

static inline int ui_grid_cur_item(struct ui_grid *grid)
{
    if (grid->touch_index >= 0) {
        return grid->touch_index;
    }
    return grid->hi_index;
}

#define ui_grid_set_item(grid, index)  	(grid)->hi_index = index

void ui_grid_enable();
void ui_grid_on_focus(struct ui_grid *grid);
void ui_grid_lose_focus(struct ui_grid *grid);
void ui_grid_state_reset(struct ui_grid *grid, int highlight_item);
int ui_grid_highlight_item(struct ui_grid *grid, int item, bool yes);
int ui_grid_highlight_item_by_id(int id, int item, bool yes);
struct ui_grid *__ui_grid_new(struct element_css1 *css, int id, struct ui_grid_item_info *info, struct element *parent);
int ui_grid_slide(struct ui_grid *grid, int direction, int steps);
int ui_grid_set_item_num(struct ui_grid *grid, int item_num);
int ui_grid_set_slide_direction(struct ui_grid *grid, int dir);
int ui_grid_slide_with_callback(struct ui_grid *grid, int direction, int steps, void(*callback)(void *ctrl));
void ui_grid_child_init(struct ui_grid *grid, struct ui_grid_info *info);
int ui_grid_dynamic_slide(struct ui_grid *grid, int direction, int steps);//动态列表滚动
int ui_grid_dynamic_create(struct ui_grid *grid, int direction, int list_total, int (*event_handler_cb)(void *, int, int, int)); //动态列表创建
int ui_grid_dynamic_release(struct ui_grid *grid);//动态列表释放

int ui_grid_dynamic_cur_item(struct ui_grid *grid);//动态列表获取选项
int ui_grid_dynamic_set_item_by_id(int id, int count);//修改动态列表数
int ui_grid_dynamic_reset(struct ui_grid *grid, int index); //重置动态列表
void ui_grid_set_scroll_area(struct ui_grid *grid, struct scroll_area *area);

int ui_grid_init_dynamic(struct ui_grid *grid, int *row, int *col);
int ui_grid_add_dynamic(struct ui_grid *grid, int *row, int *col, int redraw);
int ui_grid_del_dynamic(struct ui_grid *grid, int *row, int *col, int redraw);
int ui_grid_set_hi_index(struct ui_grid *grid, int hi_index);
int ui_grid_set_pix_scroll(struct ui_grid *grid, int enable);
int ui_grid_get_hindex(struct ui_grid *grid);

int ui_grid_get_hindex_dynamic(struct ui_grid *grid);
/*******
*该函数主要是联合ui_grid_set_base_dynamic函数,用于动态
*列表记忆,在初始化时调用
dhindex:初始化时需要跳转到的动态高亮项
init:是否是初始化
hi_index:init为1时有效,表示要将dhindex设置到真实列表项的第几项
*******/
int ui_grid_set_hindex_dynamic(struct ui_grid *grid, int dhindex, int init, int hi_index);
/*******
*该函数主要是联合ui_grid_set_hindex_dynamic函数,用于动态
*列表记忆,在初始化时调用
base_index_once:0~max_dynamic_index,动态列表初始化时一次性索引基础值，一般配置为ui_grid_set_hindex_dynamic函数参数的(dhindex - hi_index),如果dhindex < hi_index,则需要根据列表项本身来调整
init_step:初始化记忆之后，在第一次刷新之前，需要移动的步进
*******/
int ui_grid_set_base_dynamic(struct ui_grid *grid, u32 base_index_once, int init_step_once);
// int ui_grid_update_by_id_dynamic(int id, int redraw);
int ui_grid_update_by_id_dynamic(int id, int item_sel, int redraw);
int ui_grid_add_dynamic_by_id(int id, int *row, int *col, int redraw);
int ui_grid_del_dynamic_by_id(int id, int *row, int *col, int redraw);
int ui_grid_cur_item_dynamic(struct ui_grid *grid);
//回弹功能开关
void ui_grid_flick_ctrl_close(struct ui_grid *grid, u8 flag);
//惯性滑动时间间隔
int ui_grid_set_tslide(struct ui_grid *grid, u8 tslide);
//惯性滚动的负加速度值、滑动速度参数，a参考值-0.010,val参考值0.3
int ui_grid_set_energy(struct ui_grid *grid, float a, float val);

/****************************************************
//滑动居中参数说明
	*item_offset :最大偏移项的数量，
		- 默认值: 0
		- ex:设为1时，每次滑动只能滑动1项
	*target_line :居中目标线，
		- 默认值:为父控件宽/高的一半
		- ex: 屏高454，项高112，间隔2，第二项高亮居中，则target_line = (112+2+(112/2))
	*next_threshold :偏移到下一项阈值
		- 默认值：0 跟随模式配置
		- ex：1000 移动距离超过父控件宽/高的10%时，居中下一项，否则回弹
	*index_mode:居中计算的数据源自高亮项/触摸项
		- 默认值：0 高亮项 1：触摸项
		- note：项宽/高接近或等于列表宽/高时，用触摸项效果好,如表盘选择；
				项宽/高小于列表宽/高一半时，用高亮项效果好，如菜单列表；
*****************************************************/
#define grid_energy_auto_center(a,b) ui_grid_energy_auto_center(a,b)//兼容旧接口命名
int ui_grid_energy_auto_center(struct ui_grid *grid, AUTO_CENTER_MODE auto_center_mode);
int ui_grid_set_energy_item_offset(struct ui_grid *grid, int item_offset);
int ui_grid_set_energy_target_line(struct ui_grid *grid, int target_line);
int ui_grid_set_energy_next_threshold(struct ui_grid *grid, int next_threshold);
int ui_grid_auto_center_set_custom_param(struct ui_grid *grid, float a, float val, u8 tslide, int target_line, int item_offset, int next_offset, int index_mode);





/**
 * @brief grid_key_jump:静态垂直或者水平列表才能使用，用于使用按键时，
 * 没有高亮项，但按下按键能看到列表跳动
 * 注意：必须要初始化的回调里面，并且设置了方向之后才能调用
 *
 * @param grid 列表句柄
 * @param en 是否使能该模式
 *
 * @return 0设置成功 其他设置失败
 */
int grid_key_jump(struct ui_grid *grid, int en);
#endif



