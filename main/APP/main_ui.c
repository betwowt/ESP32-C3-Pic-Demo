#include "lvgl.h"
#include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
#include "main_ui.h"
#include "esp_random.h"

// 定义图片路径数组
const char *image_paths[] = {
    "A:storage/image_1_400_300.jpg",
    "A:storage/image_2_400_300.jpg",
    "A:storage/image_3_400_300.jpg",
};
#define WIDTH 142
#define HEIGH 428
#define IMAGE_COUNT 3
#define NUM_STRIPES 5
LV_IMG_DECLARE(image_1_400_300);
LV_IMG_DECLARE(image_2_400_300);
LV_IMG_DECLARE(image_3_400_300);

int random_num;
static uint8_t current_index = 0;    // 当前显示的图片索引
static lv_obj_t *current_img = NULL; // 当前显示的图片对象
static lv_obj_t *g_stripes_container = NULL;
static int completed_stripes = 0;
static lv_timer_t *switch_timer = NULL; // 记录定时器指针
static bool animation_running = false; // 动画是否正在进行
static lv_obj_t *last_animation_container = NULL; // 记录上一个动画的容器
static lv_obj_t *previous_animation_container = NULL; // 记录上一个动画的容器
static lv_obj_t *current_animation_container = NULL;  // 当前动画容器
static int last_animation_type = 3;  // -1表示初始状态，0~3对应动画类型

void fade_animation(lv_obj_t *old_img, lv_obj_t *new_img);
void slide_horizontal_animation(lv_obj_t *old_img, lv_obj_t *new_img);
void slide_vertical_animation(lv_obj_t *old_img, lv_obj_t *new_img);
void venetian_blinds_animation(lv_obj_t *old_img, lv_obj_t *new_img); // 新增：百叶窗效果动画
static void delay_switch_to_next_image_timer(lv_timer_t *timer);
void init_first_image();
void switch_to_next_image();
static void delay_delete_timer(lv_timer_t *t);
static void animation_complete_callback(lv_anim_t *a);

void lv_obj_set_style_image_opa_my(lv_obj_t *obj, lv_opa_t value)
{
    lv_obj_set_style_image_opa(obj, value, 0);
}

void lv_obj_delete_callback(lv_anim_t *a)
{

    lv_obj_t *cont = lv_obj_get_parent(a->var);
    if(cont){
        lv_obj_delete(cont); // 删除容器
    }

    lv_obj_t *obj = (lv_obj_t *)a->var;
    if(obj) { // 确保 obj 不是 NULL
        lv_obj_del(obj);
    }
}



// 初始化第一张图片
void init_first_image()
{
    current_img = lv_image_create(lv_screen_active());
    // lv_image_set_src(current_img, image_paths[current_index]);
    lv_image_set_src(current_img,&image_1_400_300);
    lv_obj_align(current_img, LV_ALIGN_CENTER, 0, 0);
}

// 切换到下一张图片（主循环中调用）
void switch_to_next_image()
{
    previous_animation_container = NULL;
    current_animation_container = NULL;
    if (animation_running) return; // 动画正在进行，等待完成
    animation_running = true; // 标记动画已开始

    // **如果存在上一个动画的容器，销毁它**
    if (previous_animation_container && lv_obj_is_valid(previous_animation_container) ) {
        lv_obj_del(previous_animation_container);
        previous_animation_container = NULL;
    }

    // **删除旧的定时器，防止创建多个定时器**
    if (switch_timer) {
        lv_timer_del(switch_timer);
        switch_timer = NULL;
    }
    // 计算下一张图片索引（循环模式）
    uint8_t next_index = (current_index + 1) % IMAGE_COUNT;

    // 创建新图片对象
    lv_obj_t *new_img = lv_image_create(lv_screen_active());
    //lv_image_set_src(new_img, image_paths[next_index]);

    if(next_index == 0){
        lv_image_set_src(new_img, &image_1_400_300);
    }else if(next_index == 1){
        lv_image_set_src(new_img, &image_2_400_300);
    }else if(next_index == 2){
        lv_image_set_src(new_img, &image_3_400_300);
    }

    lv_obj_add_flag(new_img, LV_OBJ_FLAG_HIDDEN); // 初始隐藏
    // 生成随机数时排除上一次是百叶窗的情况
    if (last_animation_type == 3) {  // 3是百叶窗动画的编号
        random_num = esp_random() % 3;  // 仅生成0~2（水平、垂直、淡入淡出）
    } else {
        random_num = esp_random() % 4;  // 正常生成0~3
    }

    // 启动切换动画
    switch(random_num) {
        case 0:
            slide_horizontal_animation(current_img, new_img);
            break;
        case 1:
            slide_vertical_animation(current_img, new_img);
            break;
        case 2:
            fade_animation(current_img, new_img);
            break;
        case 3:
            venetian_blinds_animation(current_img, new_img);
            break;
    }
    // 记录当前动画类型
    last_animation_type = random_num;
    // 更新状态
    current_index = next_index;
    current_img   = new_img;
}

static void animation_complete_callback(lv_anim_t *a)
{
    lv_obj_t *obj = (lv_obj_t *)a->var;
    if (obj) {
        lv_obj_del(obj);
    }

    // **如果上一个动画的容器存在，现在删除它**
    if (previous_animation_container) {
        lv_obj_del(previous_animation_container);
        previous_animation_container = NULL;
    }

    // **当前动画容器成为 "上一个动画容器"**
    previous_animation_container = current_animation_container;
    current_animation_container = NULL;

    // **删除旧的定时器，确保只有一个定时器**
    if (switch_timer) {
        lv_timer_del(switch_timer);
        switch_timer = NULL;
    }

    // 等待2s
    switch_timer = lv_timer_create(delay_switch_to_next_image_timer,2000,NULL);
}

// 水平滑动动画（新图从右侧滑入，旧图从左侧滑出）
void slide_horizontal_animation(lv_obj_t *old_img, lv_obj_t *new_img)
{
    // 创建父容器（控制滑动区域）
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(cont); // 清除默认样式
    lv_obj_set_size(cont, WIDTH, HEIGH);

    // 将旧图和新图放入容器
    lv_obj_set_parent(old_img, cont);
    lv_obj_set_parent(new_img, cont);
    lv_obj_clear_flag(new_img, LV_OBJ_FLAG_HIDDEN); // 显示新图

    // 初始位置：旧图居中，新图在右侧
    lv_obj_align(old_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(new_img, LV_ALIGN_CENTER, 0, 0);

    // 旧图向左滑出动画
    lv_anim_t anim_old;
    lv_anim_init(&anim_old);
    lv_anim_set_var(&anim_old, old_img);
    lv_anim_set_values(&anim_old, 0, -LV_HOR_RES); // X坐标从0到-屏幕宽度
    lv_anim_set_duration(&anim_old, 500);
    lv_anim_set_exec_cb(&anim_old, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&anim_old, lv_anim_path_ease_out);

    // 新图向左滑入动画
    lv_anim_t anim_new;
    lv_anim_init(&anim_new);
    lv_anim_set_var(&anim_new, new_img);
    lv_anim_set_values(&anim_new, LV_HOR_RES, 0); // X坐标从屏幕宽度到0
    lv_anim_set_duration(&anim_new, 500);
    lv_anim_set_exec_cb(&anim_new, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&anim_new, lv_anim_path_ease_in);

    // 设置动画完成回调：删除旧图和容器
    lv_anim_set_completed_cb(&anim_old, animation_complete_callback);

    // 启动动画
    lv_anim_start(&anim_old);
    lv_anim_start(&anim_new);
}

// 垂直滑动动画（新图从底部滑入，旧图从顶部滑出）
void slide_vertical_animation(lv_obj_t *old_img, lv_obj_t *new_img)
{
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, WIDTH, HEIGH);

    lv_obj_set_parent(old_img, cont);
    lv_obj_set_parent(new_img, cont);
    lv_obj_clear_flag(new_img, LV_OBJ_FLAG_HIDDEN); // 显示新图

    // 初始位置：旧图居中，新图在底部
    lv_obj_align(old_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(new_img, LV_ALIGN_CENTER, 0, 0);

    // 旧图向上滑出
    lv_anim_t anim_old;
    lv_anim_init(&anim_old);
    lv_anim_set_var(&anim_old, old_img);
    lv_anim_set_values(&anim_old, 0, -LV_VER_RES);
    lv_anim_set_duration(&anim_old, 500);
    lv_anim_set_exec_cb(&anim_old, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&anim_old, lv_anim_path_ease_out);

    // 新图向上滑入
    lv_anim_t anim_new;
    lv_anim_init(&anim_new);
    lv_anim_set_var(&anim_new, new_img);
    lv_anim_set_values(&anim_new, LV_VER_RES, 0);
    lv_anim_set_duration(&anim_new, 500);
    lv_anim_set_exec_cb(&anim_new, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&anim_new, lv_anim_path_ease_in);

    // 动画完成回调
    lv_anim_set_completed_cb(&anim_old, animation_complete_callback);

    lv_anim_start(&anim_old);
    lv_anim_start(&anim_new);
}

// 淡入淡出动画
void fade_animation(lv_obj_t *old_img, lv_obj_t *new_img)
{
    // 创建父容器（替代旧版直接添加到屏幕）
    lv_obj_t *cont = lv_obj_create(lv_screen_active());

    lv_obj_remove_style_all(cont); // 清除默认样式
    lv_obj_set_size(cont, WIDTH, HEIGH);

    lv_obj_set_parent(old_img, cont);
    lv_obj_set_parent(new_img, cont);
    lv_obj_clear_flag(new_img, LV_OBJ_FLAG_HIDDEN); // 显示新图

    // 初始位置：旧图居中，新图在底部
    lv_obj_align(old_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(new_img, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_image_opa(old_img, LV_OPA_COVER, 0);
    lv_obj_set_style_image_opa(new_img, LV_OPA_TRANSP, 0); // 关键：初始透明

    // 图片1淡出动画（LVGL9动画API重构）
    lv_anim_t anim_img1;
    lv_anim_init(&anim_img1);
    lv_anim_set_var(&anim_img1, old_img);
    lv_anim_set_values(&anim_img1, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_duration(&anim_img1, 2000);
    lv_anim_set_exec_cb(&anim_img1, (lv_anim_exec_xcb_t)lv_obj_set_style_image_opa_my);
    lv_anim_set_path_cb(&anim_img1, lv_anim_path_linear);


    // lv_anim_set_repeat_count(&anim_img1, LV_ANIM_REPEAT_INFINITE); // 无限重复
    // lv_anim_set_playback_duration(&anim_img1, 2000); // 回程动画时长1秒
    // lv_anim_set_playback_time(&anim_img1, 1000);    // 回程延时（立即反向）

    

    // 图片2淡入动画
    lv_anim_t anim_img2;
    lv_anim_init(&anim_img2);
    lv_anim_set_var(&anim_img2, new_img);
    lv_anim_set_values(&anim_img2, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_duration(&anim_img2, 2000);
    lv_anim_set_exec_cb(&anim_img2, (lv_anim_exec_xcb_t)lv_obj_set_style_image_opa_my);
    lv_anim_set_path_cb(&anim_img2, lv_anim_path_linear);


    // 动画完成回调
    lv_anim_set_completed_cb(&anim_img1, animation_complete_callback);
    // lv_anim_set_repeat_count(&anim_img2, LV_ANIM_REPEAT_INFINITE); // 无限重复
    // lv_anim_set_playback_duration(&anim_img2, 2000); // 回程动画时长1秒
    // lv_anim_set_playback_time(&anim_img2, 1000);    // 回程延时（立即反向）
    lv_anim_start(&anim_img1);
    lv_anim_start(&anim_img2);
}

static void stripe_anim_done(lv_anim_t *a)
{
    (void)a;
    completed_stripes++;

    if (completed_stripes == NUM_STRIPES) {
        completed_stripes = 0;

        // 删除百叶窗容器及其子对象
        if (g_stripes_container && lv_obj_is_valid(g_stripes_container)) {
            lv_obj_del(g_stripes_container); // 删除容器会自动删除所有子对象（条纹）
            g_stripes_container = NULL;
        }

        // 显示新图片（此时动画已结束）
        lv_obj_clear_flag(current_img, LV_OBJ_FLAG_HIDDEN); // 确保新图片可见

        // 更新前一个动画容器引用（避免内存泄漏）
        previous_animation_container = g_stripes_container;

        // 解除动画阻塞并触发下一次切换
        animation_running = false;
        if (switch_timer) lv_timer_del(switch_timer);
        // 等待2s执行切换下一个图片
        switch_timer = lv_timer_create(delay_switch_to_next_image_timer, 2000, NULL);
    }
}

// 新增：百叶窗效果动画
// 将新图分成多个垂直条（这里设为10条），每条先从屏幕外上方滑入到正确位置，形成百叶窗打开的效果
void venetian_blinds_animation(lv_obj_t *old_img, lv_obj_t *new_img)
{
    // 创建一个父容器，用于存放各个条
    lv_obj_t *cont = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(cont);
    lv_obj_set_size(cont, WIDTH, HEIGH);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    g_stripes_container = cont;  // 保存外层容器指针

    // 新图直接附加到屏幕，并设为当前显示对象
    lv_obj_set_parent(new_img, lv_screen_active()); // 关键：父对象为屏幕
    lv_obj_align(new_img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(new_img, LV_OBJ_FLAG_HIDDEN);  // 初始隐藏
    current_img = new_img;

    // 获取新图的图片源
    const void *img_src = lv_img_get_src(new_img);

    // 定义条数量与宽度

    int stripe_width = WIDTH / NUM_STRIPES;


    for (int i = 0; i < NUM_STRIPES; i++) {
        // 为每个条创建一个容器（启用裁剪，以只显示对应区域）
        lv_obj_t *stripe_cont = lv_obj_create(cont);
        lv_obj_remove_style_all(stripe_cont);

        int stripe_height = HEIGH;  // 修改条容器高度

        lv_obj_set_size(stripe_cont, stripe_width, stripe_height);
        // 初始位置：条容器在屏幕上方（完全不可见）
        lv_obj_set_pos(stripe_cont, i * stripe_width, -stripe_height);
        // 启用裁剪（clip）使内部子对象只显示容器区域
        lv_obj_set_style_clip_corner(stripe_cont, true, 0);

        // 在条容器中创建图像对象，显示新图
        lv_obj_t *stripe_img = lv_image_create(stripe_cont);
        lv_obj_remove_style_all(stripe_img);
        // lv_obj_align(stripe_img, LV_ALIGN_CENTER, 0, 0);
        lv_image_set_src(stripe_img, img_src);
        // 通过设置偏移，使得条容器内仅显示新图对应的那一部分
        lv_obj_set_pos(stripe_img, -i * stripe_width, 0);

        // 为每个条容器添加动画：从 y = -HEIGH 滑动到 y = 0
        lv_anim_t anim;
        lv_anim_init(&anim);
        lv_anim_set_var(&anim, stripe_cont);
        lv_anim_set_values(&anim, -stripe_height, 0);
        lv_anim_set_duration(&anim, 500);
        // 每个条增加一个延时，制造依次出现的效果
        lv_anim_set_delay(&anim, i * 50);
        lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
        lv_anim_set_path_cb(&anim, lv_anim_path_ease_out);
        lv_anim_set_completed_cb(&anim, stripe_anim_done); // 仅最后一个条纹触发
        lv_anim_start(&anim);
    }

    // 立即删除旧图，无需等待动画
    lv_obj_t *old_cont = lv_obj_get_parent(old_img);
    lv_obj_del(old_img);
    if (old_cont && lv_obj_is_valid(old_cont)) {
        lv_obj_del(old_cont);  // 删除旧图片的父容器
    }
}

static void delay_delete_timer(lv_timer_t *t) {
    lv_obj_t *obj = (lv_obj_t *)lv_timer_get_user_data(t);
    if (obj) {
        lv_obj_del(obj);
    }
    lv_timer_del(t);
}

static void delay_switch_to_next_image_timer(lv_timer_t *timer)
{
    animation_running = false; // 动画完成，允许下一个动画开始
    switch_to_next_image();
}

// 预加载下一张图片到缓存
lv_obj_t *preload_next_image()
{
    uint8_t next_index = (current_index + 1) % IMAGE_COUNT;
    lv_obj_t *img      = lv_image_create(NULL); // 不附加到屏幕
    lv_image_set_src(img, image_paths[next_index]);
    return img;
}

// 修改切换函数使用预加载对象
void switch_to_preloaded(lv_obj_t *preloaded_img)
{
    lv_obj_set_parent(preloaded_img, lv_screen_active());
    slide_horizontal_animation(current_img, preloaded_img);
    current_img   = preloaded_img;
    current_index = (current_index + 1) % IMAGE_COUNT;
}

void    main_ui_create(void)
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN); // 确保不透明度生效

    // 初始化随机数种子
    // srand(time(NULL));

    init_first_image(); // 显示第一张图片

    lv_timer_t * timer1;
    /* 创建第一个timer */
    switch_timer = lv_timer_create(delay_switch_to_next_image_timer, 2000, NULL);

}