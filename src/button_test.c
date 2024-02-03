#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libdragon.h>

#include "range_test.h"
#include "button_test.h"
#include "drawing.h"
#include "text.h"
#include "colors.h"
#include "input.h"

static const char *btn_gfx[] =
{
    "/gfx/btn_a.sprite",
    "/gfx/btn_b.sprite",
    "/gfx/btn_s.sprite",
    "/gfx/btn_l.sprite",
    "/gfx/btn_r.sprite",
    "/gfx/btn_z.sprite",
    "/gfx/btn_c_up.sprite",
    "/gfx/btn_c_down.sprite",
    "/gfx/btn_c_left.sprite",
    "/gfx/btn_c_right.sprite",
    "/gfx/btn_d_up.sprite",
    "/gfx/btn_d_down.sprite",
    "/gfx/btn_d_left.sprite",
    "/gfx/btn_d_right.sprite",
};

static int exit_seq[] = {0, 2, 4, 5};

void draw_line(
    display_context_t ctx,
    uint32_t color,
    int x_offset,
    int y_offset,
    int y_height)
{
    draw_aa_line(
        ctx,
        x_offset,
        y_offset,
        x_offset,
        y_offset + y_height,
        color
    );
}

int marker = 0;
void set_marker()
{
    marker = 1;
}

void display_buttons() {
    int line_height = 11,
        sz_history = 100,

        x_col_width = 156,
        x_offset = 24,
        x_offset_col1 = x_offset + sz_history,
        x_offset_col2 = x_offset + x_col_width + sz_history,

        btn_x_col1 = sz_history + x_offset + 8,
        btn_x_col2 = btn_x_col1 + x_col_width,

        btn_y_offset = 56,
        btn_y_spacer = 10,
        btn_y_height = 16,

        btn_y_offset_a       = btn_y_offset         + 0,
        btn_y_offset_b       = btn_y_offset_a       + btn_y_height,
        btn_y_offset_c_up    = btn_y_offset_b       + btn_y_height + btn_y_spacer,
        btn_y_offset_c_down  = btn_y_offset_c_up    + btn_y_height,
        btn_y_offset_c_left  = btn_y_offset_c_down  + btn_y_height,
        btn_y_offset_c_right = btn_y_offset_c_left  + btn_y_height,
        btn_y_offset_start   = btn_y_offset_c_right + btn_y_height + btn_y_spacer,

        btn_y_offset_r       = btn_y_offset         + 0,
        btn_y_offset_l       = btn_y_offset_r       + btn_y_height,
        btn_y_offset_d_up    = btn_y_offset_l       + btn_y_height + btn_y_spacer,
        btn_y_offset_d_down  = btn_y_offset_d_up    + btn_y_height,
        btn_y_offset_d_left  = btn_y_offset_d_down  + btn_y_height,
        btn_y_offset_d_right = btn_y_offset_d_left  + btn_y_height,
        btn_y_offset_z       = btn_y_offset_d_right + btn_y_height + btn_y_spacer,

        sprite_y_offset = -1 * (btn_y_height / 2),

        lbl_y_offset_row1 = 192,
        lbl_y_offset_row2 = lbl_y_offset_row1 + (line_height * 2),
        lbl_x_offset_col1 = btn_x_col1 + 8,
        lbl_x_offset_col2 = 180,
        count = 0;

    uint32_t c_blue = graphics_make_color(0, 192, 255, 255);
    uint32_t c_green = graphics_make_color(64, 255, 0, 255);
    uint32_t c_red = graphics_make_color(255, 0, 0, 255);
    uint32_t c_yellow = graphics_make_color(255, 255, 0, 255);
    uint32_t c_gray = graphics_make_color(128, 128, 128, 255);
    struct SI_condat history[sz_history];

    static sprite_t *btn_sprites[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < 14; i++) {
        int f = dfs_open(btn_gfx[i]);
        int size = dfs_size(f);
        btn_sprites[i] = malloc(size);
        dfs_read(btn_sprites[i], size, 1, f);
        dfs_close(f);
    }

    text_set_line_height(line_height);
    display_context_t ctx;

    timer_init();
    new_timer(TIMER_TICKS(1000000), TF_CONTINUOUS, set_marker);

    int fps = 0, last_fps = 0;
    for (;;) {
        while ((ctx = display_lock()) == 0) {}
        display_show(ctx);

        graphics_fill_screen(ctx, COLOR_BACKGROUND);
        graphics_set_color(COLOR_FOREGROUND, 0);
        // Guides for lining up elements
        // graphics_draw_line(ctx, 4, 4, 4, 236, c_blue);
        // graphics_draw_line(ctx, 4, 4, 316, 4, c_blue);
        // graphics_draw_line(ctx, 316, 4, 316, 236, c_blue);
        // graphics_draw_line(ctx, 4, 236, 316, 236, c_blue);
        // graphics_draw_line(ctx, 162, 4, 162, 236, c_green);

        controller_scan();
        struct controller_data cdata = get_keys_pressed();

        char buf[128];
        text_set_font(FONT_BOLD);
        snprintf(buf, sizeof(buf), "Button tester");
        text_draw(ctx, 160, 15, buf, ALIGN_CENTER);

        snprintf(buf, sizeof(buf), "%3d", cdata.c[0].x);
        text_draw(ctx, lbl_x_offset_col1, lbl_y_offset_row1, buf, ALIGN_RIGHT);

        snprintf(buf, sizeof(buf), "%3d", cdata.c[0].y);
        text_draw(ctx, lbl_x_offset_col2, lbl_y_offset_row1, buf, ALIGN_RIGHT);

        text_set_font(FONT_MEDIUM);
        snprintf(buf, sizeof(buf), "x");
        text_draw(ctx, lbl_x_offset_col1 + 8, lbl_y_offset_row1, buf, ALIGN_LEFT);

        snprintf(buf, sizeof(buf), "y");
        text_draw(ctx, lbl_x_offset_col2 + 8, lbl_y_offset_row1, buf, ALIGN_LEFT);

        text_set_font(FONT_MEDIUM);
        text_draw(ctx, 16, lbl_y_offset_row2, "Exit: ", ALIGN_LEFT);
        graphics_set_color(graphics_make_color(128, 128, 128, 255), 0);

        fps++;
        if (marker == 1) {
            draw_line(ctx, c_gray, x_offset_col1, btn_y_offset_a, line_height * -1.2);
            cdata.c[0].x = 1;
            marker = 0;
            last_fps = fps;

            fps = 0;
        } else {
            cdata.c[0].x = 0;
        }
        char fps_buf[128];
        snprintf(fps_buf, sizeof(fps_buf), "Rendering %d fps", last_fps);
        text_draw(ctx, 320 - 16, lbl_y_offset_row2, fps_buf, ALIGN_RIGHT);

        for (int i = count; i > 0; i--) {
            history[i] = history[i - 1];
            if (history[i].x == 1) {
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_a, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_b, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_start, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_c_up, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_c_down, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_c_left, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col1 - i, btn_y_offset_c_right, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_r, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_l, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_z, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_up, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_down, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_left, line_height * -1.2);
                draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_right, line_height * -1.2);
            }
            draw_line(ctx, c_blue, x_offset_col1 - i, btn_y_offset_a, history[i].A * line_height * -1);
            draw_line(ctx, c_green, x_offset_col1 - i, btn_y_offset_b, history[i].B * line_height * -1);
            draw_line(ctx, c_red, x_offset_col1 - i, btn_y_offset_start, history[i].start * line_height * -1);
            draw_line(ctx, c_yellow, x_offset_col1 - i, btn_y_offset_c_up, history[i].C_up * line_height * -1);
            draw_line(ctx, c_yellow, x_offset_col1 - i, btn_y_offset_c_down, history[i].C_down * line_height * -1);
            draw_line(ctx, c_yellow, x_offset_col1 - i, btn_y_offset_c_left, history[i].C_left * line_height * -1);
            draw_line(ctx, c_yellow, x_offset_col1 - i, btn_y_offset_c_right, history[i].C_right * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_r, history[i].R * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_l, history[i].L * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_z, history[i].Z * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_up, history[i].up * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_down, history[i].down * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_left, history[i].left * line_height * -1);
            draw_line(ctx, c_gray, x_offset_col2 - i, btn_y_offset_d_right, history[i].right * line_height * -1);

        }

        history[0] = cdata.c[0];
        draw_line(ctx, c_blue, x_offset_col1, btn_y_offset_a, cdata.c[0].A * line_height * -1);
        draw_line(ctx, c_green, x_offset_col1, btn_y_offset_b, cdata.c[0].B * line_height * -1);
        draw_line(ctx, c_red, x_offset_col1, btn_y_offset_start, cdata.c[0].start * line_height * -1);
        draw_line(ctx, c_yellow, x_offset_col1, btn_y_offset_c_up, cdata.c[0].C_up * line_height * -1);
        draw_line(ctx, c_yellow, x_offset_col1, btn_y_offset_c_down, cdata.c[0].C_down * line_height * -1);
        draw_line(ctx, c_yellow, x_offset_col1, btn_y_offset_c_left, cdata.c[0].C_left * line_height * -1);
        draw_line(ctx, c_yellow, x_offset_col1, btn_y_offset_c_right, cdata.c[0].C_right * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_r, cdata.c[0].R * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_l, cdata.c[0].L * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_z, cdata.c[0].Z * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_d_up, cdata.c[0].up * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_d_down, cdata.c[0].down * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_d_left, cdata.c[0].left * line_height * -1);
        draw_line(ctx, c_gray, x_offset_col2, btn_y_offset_d_right, cdata.c[0].right * line_height * -1);


        if (count < sz_history - 1) {
            count++;
        }

        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_a, btn_sprites[0]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_b, btn_sprites[1]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_start, btn_sprites[2]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_l, btn_sprites[3]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_r, btn_sprites[4]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_z, btn_sprites[5]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_c_up, btn_sprites[6]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_c_down, btn_sprites[7]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_c_left, btn_sprites[8]);
        graphics_draw_sprite(ctx, btn_x_col1, sprite_y_offset + btn_y_offset_c_right, btn_sprites[9]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_d_up, btn_sprites[10]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_d_down, btn_sprites[11]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_d_left, btn_sprites[12]);
        graphics_draw_sprite(ctx, btn_x_col2, sprite_y_offset + btn_y_offset_d_right, btn_sprites[13]);

        for (int i = 0; i < 4; i++) {
            graphics_draw_sprite(ctx, 42 + (i*10), lbl_y_offset_row2 + 2, btn_sprites[exit_seq[i]]);
        }

        if (cdata.c[0].start && cdata.c[0].A && cdata.c[0].Z && cdata.c[0].R) {
            break;
        }
    }

    for (int i = 0; i < 14; i++) {
        free(btn_sprites[i]);
    }
    timer_close();
}
