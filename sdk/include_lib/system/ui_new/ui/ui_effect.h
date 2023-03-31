#ifndef __UI_EFFECT_H__
#define __UI_EFFECT_H__


#include "generic/typedef.h"


//注意!!!当有两个页面的时候，也就是滑屏的时候
//1.一个页面需要做特效，另外一个不需要特效，那么
//只需要配置做特效的那个页面的(effect_mode + copy)
//2.两个页面都需要做特效，那么两个页面配置的
//copy方式必须一致，但effect_mode可以不同
//3.如果两个页面都不需要做特效，effect_mode = 0,
//那么会默认以EFFECT_COPY_SPLIC方式拼接成一张大图




#define EFFECT_COPY_SIGLE			2//单独存放页面,滑屏的时候有两个页面单独存放
#define EFFECT_COPY_SPLIC			1//拼接成一个页面,不滑屏的时候只有一个页面

//effect_mode:高8位是effect_mode，低8位是进行特效前需要
//将合成的页面拼接成一张大图还是单独存放
//新加的特效宏要按顺序加1 !!!
#define EFFECT_MODE_NONE		(u16)(0)
#define EFFECT_MODE_SCA			(u16)((1 << 8) | EFFECT_COPY_SIGLE)//屏幕的宽和高都小于或者等于240时才可使用
#define EFFECT_MODE_USER0		(u16)((2 << 8) | EFFECT_COPY_SPLIC)
#define EFFECT_MODE_USER1		(u16)((3 << 8) | EFFECT_COPY_SIGLE)
#define EFFECT_MODE_FLIP_SCA_ALPHA			(u16)((4 << 8) | EFFECT_COPY_SIGLE)
#define EFFECT_MODE_3D			(u16)((5 << 8) | EFFECT_COPY_SIGLE)
#define EFFECT_MODE_SCA_ALPHA			(u16)((6 << 8) | EFFECT_COPY_SIGLE)
#define EFFECT_MODE_REDRAW_PART			(u16)((7 << 8) | EFFECT_COPY_SIGLE)

#define EFFECT_MODE_MAX				(u16)7

//注意！！！这里高8要随着最大值加1而动态改变
#define EFFECT_MODE_LIMIT		(u16)(((EFFECT_MODE_MAX + 1) << 8) | EFFECT_COPY_SPLIC)



struct effect_ctrl {
    // u8 copy_to_psram;//EFFECT_COPY_SIGLE / EFFECT_COPY_SPLIC
    u8 *splicing_buf; //拼接buf
    u8 *psbuf[2]; //单独buf

    u8 *sca_obuf; //硬件缩放的输出buf,内部缩放特效使用
    u8 *cache_buf[2]; //内部特效使用

    int not_refresh;//特效处理完之后，是否推屏，置一表示不推屏

    //由特效处理函数赋值,有特效时必须在特效处理函数里面赋值
    //1.滑屏的时候，如果ps_obuf = splicing_buf,表示特效处理完
    //之后还是放到原来的空间(对于那种不需要随着滑动而改变的特
    //效，只做一次就可以不断来回滑动)，否则就是新的一块空间(该
    //空间只有一个页面大小)
    //2.不滑屏的时候，该值由特效本身决定
    u8 *ps_obuf;
};

struct effect_part {
    int group;
    u8 *bg_addr;
};

//用户自定义回调原型
//ctrl:特效控制结构体 详细看结构体说明
//cur_head:当前需要做特效的页面结构体(struct imb_task_head)
//other_head:另外一个页面结构体，另外页面不一定需要做特效
typedef void *(*effect_cb)(struct effect_ctrl *ctrl, void *cur_head,  void *other_head);


//EFFECT_MODE_SCA
//1.取值范围0.3~1
//2.可以不传该参数，内部使用1
//3.参数要静态或者全局变量
struct effect_ratio {
    float ratio_w;
    float ratio_h;
};


//EFFECT_MODE_SCA_ALPHA & EFFECT_MODE_FLIP_SCA_ALPHA
//1.当作为EFFECT_MODE_FLIP_SCA_ALPHA模式的参数时，表示起始
//的值 start_alpha,start_scale
//2.可以不传该参数，内部使用默认的值(sca = 0.7, alpha = 64)
//3.参数要静态或者全局变量
struct effect_sca_alpha {
    float sca;
    int alpha;
};

//EFFECT_MODE_3D
//1.focal取值100~1000，值越小变化越明显
//2.可以不传该参数，内部使用默认的值(focal = 500)
//3.参数要静态或者全局变量
struct effect_3d {
    int focal;
};

//EFFECT_MODE_REDRAW_PART
//1.只合成第一部分的那一次存放的地址，内部使用
//2.表示第一部分是否只合成一次，置一就只合成一次，然后做第一部分的特效时使用合成的那次作为输入
//3.reflesh表示做完该特效后是否刷新显示
//4.part1_effect_mode:该特效第一部分的特效模式
//5.part1_effect_priv:该特效第一部分的特效参数
//6.part1:第一部分的分组,分组是ui_core_set_group()函数分的组,只针对全屏布局
//7.patr2:第二部分的分组,分组是ui_core_set_group()函数分的组,只针对全屏布局
//8.参数要静态或者全局变量
struct effect_redraw_part {
    u8 *part1_kick_buf;
    u8 part1_one_kick;
    u8 reflesh;

    u16 part1_effect_mode;
    void *part1_effect_priv;
    struct effect_part part1;
    struct effect_part part2;
};

//页面设置特效的唯一入口,在页面的图层或者全局的布局初始化的时候(ON_CHANGE_INIT)可以调用,如果id为-1,则可以在滑屏之前调用:
//id:单个页面的id，如果为-1，则表示设置页面间滑动的特效
//effect_mode:某个页面需要的特效，宏定义在ui_effect.h里面
//user_effect:用户自定义特效处理的回调函数，可以为空，如果有值，库里面
//会优先使用这个函数去处理该页面的特效,一般配对EFFECT_MODE_USER 0~N 使用
//effect_priv:特效的私有值，可以为空，库里面会传递给特效处理函数
//或者自定义特效回调函数
int ui_window_effect(int id, u16 effect_mode, void *user_effect, void *effect_priv);


//做特效时，可以控制页面停止刷新
void ui_window_stop_redraw(int stop_redraw);


#endif
