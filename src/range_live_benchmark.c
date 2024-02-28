#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libdragon.h>

#include "range_test.h"
#include "range_live.h"
#include "range_live_benchmark.h"
#include "text.h"
#include "colors.h"
#include "input.h"

struct Vec4
{
    int x1;
    int y1;
    int x2;
    int y2;
};

struct StickAngles last_notch_values =
{
    .values = {
      0, 0,
      0, 0,
      0, 0,
      0, 0,
      0, 0,
      0, 0,
      0, 0,
      0, 0
    }
};

struct StickAngles benchmark_compare_oem =
{
    .values = {
      0,  85,
      70, 70,
      85, 0 ,
      70,-70,
      0, -85,
     -70,-70,
     -85, 0 ,
     -70, 70
    }
};

enum View
{
    VIEW_NONE,
    VIEW_NOTCH_CARD,
    VIEW_NOTCH_DIAG,
    VIEW_NOTCH_BOTH,

    VIEW_MAX,
};

void display_live_benchmark() {
    int pos_count = 0,
        box_size = 20,
        line_height = 10,
        x_offset = 120,
        y_offset = 120,
        zoomout = 0,
        current_view = 0;
    text_set_line_height(line_height);
    display_context_t ctx;

    int f = dfs_open("/gfx/point.sprite");
    int size = dfs_size(f);
    char * title_str = "Live Benchmark";
    sprite_t *point = malloc(size);
    dfs_read(point, size, 1, f);
    dfs_close(f);

    struct Vec4 target_ranges[8];

    uint32_t comparison_color = graphics_make_color(64, 255, 0, 255),
//           target_color = graphics_make_color(128, 128, 128, 64),
             history_color = graphics_make_color(0, 192, 255, 255);

    struct Vec2 *target = (struct Vec2*)&benchmark_compare_oem;
    struct Vec2 *last_values = (struct Vec2*)&last_notch_values;
    struct Vec2 last_v = {0, 0};

    for (int i = 0; i < 8; i++) {
        int x = target[i].x - ((box_size / 2) - 1),
            y = target[i].y - ((box_size / 2) - 1);
        struct Vec4 v = { x, y, x + box_size, y + box_size };
        target_ranges[i] = v;
    }

    for (;;) {
        while ((ctx = display_lock()) == 0) {}
        display_show(ctx);

        graphics_fill_screen(ctx, COLOR_BACKGROUND);
        graphics_set_color(COLOR_FOREGROUND, 0);

        controller_scan();
        struct controller_data cdata = get_keys_pressed();
        char buf[128];

        struct Vec2 v = { cdata.c[0].x, cdata.c[0].y };

        snprintf(buf, sizeof(buf), "x\ny");
        text_set_font(FONT_MEDIUM);
        text_draw(ctx, 256, 182, buf, ALIGN_LEFT);

        text_set_font(FONT_BOLD);
        snprintf(buf, sizeof(buf), "%3d\n%3d", v.x, v.y);
        text_draw(ctx, 249, 182, buf, ALIGN_RIGHT);

        draw_center_cross(ctx, x_offset, y_offset);

        int collision = 0;
        for (int i = 0; i < 8; i++) {
//          int x = x_offset + target_ranges[i].x1,
//              y = y_offset + target_ranges[i].y1;

//          graphics_draw_box(ctx, x, y, box_size, box_size, target_color);

            if (v.x >= target_ranges[i].x1 
                    && v.x <= target_ranges[i].x2
                    && v.y >= target_ranges[i].y1
                    && v.y <= target_ranges[i].y2)
            {
                collision = 1;
                if (pos_count > 24) {
                    last_values[i].x = v.x;
                    last_values[i].y = v.y;
                    pos_count = 0;
                }
                break;
            }

        }

        if (current_view > VIEW_NONE) {
            if (current_view == VIEW_NOTCH_DIAG || current_view == VIEW_NOTCH_BOTH) {
                draw_diag_compare(ctx, last_notch_values, 0, x_offset, y_offset);

            }
            if (current_view == VIEW_NOTCH_CARD || current_view == VIEW_NOTCH_BOTH) {
                draw_cardinal_compare(ctx, last_notch_values, 0, x_offset);
            }
        } else {
            draw_stick_angles(
                ctx,
                benchmark_compare_oem,
                comparison_color,
                zoomout,
                x_offset,
                y_offset
            );
            draw_stick_angles(
                ctx,
                last_notch_values,
                history_color,
                zoomout,
                x_offset,
                y_offset
            );
        }

        if (collision) {
            if (v.x != last_v.x || v.y != last_v.y) {
                if (pos_count > 0) {
                    pos_count = 0;
                }
                last_v = v;
            } else {
                pos_count++;
            }
        } 

        int x = smax(0, smin(320, v.x + (x_offset - 2)));
        int y = smax(0, smin(240, (v.y * -1) + (y_offset - 2)));
        graphics_draw_sprite(ctx, x, y, point);

        print_stick_angles(ctx, last_notch_values);

        if (cdata.c[0].start) {
            break;
        }

        if (cdata.c[0].B) {
            for (int i = 0; i < 8; i++) {
                last_values[i].x = 0;
                last_values[i].y = 0;
            }
        }

//      if (cdata.c[0].C_left) {
//          if (current_view == 0) {
//              current_view = VIEW_MAX - 1;
//          } else {
//              current_view--;
//          }
//      } else if (cdata.c[0].C_right) {
//          current_view++;
//          if (current_view >= VIEW_MAX) {
//              current_view = 0;
//          }
//      }

        text_set_font(FONT_MEDIUM);
        graphics_set_color(COLOR_FOREGROUND, 0);
        snprintf(buf, sizeof(buf), "%s", title_str);
        text_draw(ctx, x_offset, 15, buf, ALIGN_CENTER);

        text_set_font(FONT_MEDIUM);
        graphics_set_color(graphics_make_color(128, 128, 128, 255), 0);
        text_draw(ctx, 320 - 16, 213, REPO_URL, ALIGN_RIGHT);
    }

    free(point);

}
