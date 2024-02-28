#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libdragon.h>

#include "range_test.h"
#include "colors.h"
#include "drawing.h"
#include "text.h"
#include "util.h"
#include "input.h"

struct StickAngles perfect_n64 =
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

struct StickAngles perfect_hori =
{
    .values = {
      0,  100,
      75, 75,
      100,0,
      75,-75,
      0, -100,
     -75,-75,
     -100,0,
     -75, 75
    }
};

enum Comparison
{
    COMP_NONE,
    COMP_N64,
    COMP_HORI,

    COMP_MAX,
};

struct StickAngles *comparisons[] =
{
    NULL,
    &perfect_n64,
    &perfect_hori,
};

const char *comparison_names[] =
{
    " benchmark result",
    ", ideal N64 OEM comparison",
    ", ideal Horipad Mini comparison",
};

const char *example_names[] =
{
    "Ideal N64 OEM example",
    "Ideal Horipad Mini example",
};

enum View
{
    VIEW_NONE,
    VIEW_NOTCH_CARD,
    VIEW_NOTCH_DIAG,
    VIEW_NOTCH_BOTH,
    VIEW_GRADE,
    VIEW_GRADE_LENIENT,

    VIEW_MAX,
};


void draw_stick_angles(display_context_t ctx, struct StickAngles a, uint32_t color, int zoomout, int x, int y)
{
    if (zoomout) {
        int factor = 3;
        if (zoomout == 2) factor = 2;

        for (int i = 0; i < 16; i++) {
            a.values[i] = (a.values[i] * factor) / 4;
        }
    }

    struct Vec2 *v = (struct Vec2*)&a;

    for (int i = 0; i < 8; i++) {
        int j = (i + 1) % 8;
        draw_aa_line(
            ctx,
              x + v[i].x,
            y - v[i].y,
              x + v[j].x,
            y - v[j].y,
            color);
    }
}

uint32_t get_offset_color(int a)
{
    if (a <= 1) {
        return graphics_make_color(0, 255, 64, 255);
    } else if (a <= 3) {
        return graphics_make_color(192, 255, 0, 255);
    } else if (a <= 5) {
        return graphics_make_color(255, 128, 64, 255);
    } else {
        return graphics_make_color(255, 64, 0, 255);
    }
}


void draw_diag_compare(display_context_t ctx, struct StickAngles a, int zoomout, int x, int y)
{
    int factor = 4;
    if (zoomout) {
        factor = 3;
        if (zoomout == 2) factor = 2;
        for (int i = 0; i < 16; i++) {
            a.values[i] = (a.values[i] * factor) / 4;
        }
    }

    struct Vec2 *v = (struct Vec2*)&a;

    for (int i = 1; i < 8; i += 2) {
        int j = (i + 2) % 8;

        uint32_t color;
        if (i % 4 == 1) {
            int delta = abs(v[i].x-v[j].x) * 4 / factor;
            color = get_offset_color(delta);
        } else {
            int delta = abs(v[i].y-v[j].y) * 4 / factor;
            color = get_offset_color(delta);
        }

        graphics_draw_line(
            ctx,
            x + v[i].x,
            y - v[i].y,
            x + v[j].x,
            y - v[j].y,
            color);

        color = get_offset_color(abs(v[i].x)-abs(v[i].y));
        graphics_draw_line(
            ctx,
              x,
              y,
              x + v[i].x,
              y - v[i].y,
              color);
    }
}

void draw_cardinal_compare(display_context_t ctx, struct StickAngles a, int zoomout, int x)
{
    if (zoomout) {
        for (int i = 0; i < 16; i++) {
            a.values[i] = (a.values[i] * 3) / 4;
        }
    }

    struct Vec2 *v = (struct Vec2*)&a;

    int offset = 8;
    for (int i = 0; i < 8; i += 2) {
        offset = abs(offset);
        uint32_t color;
        if (abs(v[i].x) > abs(v[i].y)) { // Horizontal axis
            if (v[i].x < 0) offset = offset * -1;
            color = get_offset_color(abs(v[i].y));
            draw_aa_line(ctx, offset+x+v[i].x,   x-v[i].y, offset+x+v[i].x,   x,        color);
            draw_aa_line(ctx, offset+x+v[i].x+2, x-v[i].y, offset+x+v[i].x-2, x-v[i].y, color);
            draw_aa_line(ctx, offset+x+v[i].x+2, x,        offset+x+v[i].x-2, x,        color);
        } else { // Vertical axis
            if (v[i].y > 0) offset = offset * -1;
            color = get_offset_color(abs(v[i].x));
            draw_aa_line(ctx, x+v[i].x,   offset+x-v[i].y,   x,          offset+x-v[i].y,   color);
            draw_aa_line(ctx, x+v[i].x,   offset+x-v[i].y-2, x+v[i].x,   offset+x-v[i].y+2, color);
            draw_aa_line(ctx, x,          offset+x-v[i].y-2, x,          offset+x-v[i].y+2, color);
        }
    }
}


void draw_center_cross(display_context_t ctx, int x_origin, int y_origin)
{
    int x, y, offset;
    y = y_origin;
    offset = x_origin - 120;
    for (x = offset; x < 240+offset; x++) {
        int i = smin(240 - abs(240 - (x-offset)*2), 120);
        graphics_draw_pixel_trans(ctx, x, y, graphics_make_color(255, 255, 255, i));
    }

    x = x_origin;
    for (y = 0; y < 240; y++) {
        int i = smin(240 - abs(240 - y * 2), 120);
        graphics_draw_pixel_trans(ctx, x, y, graphics_make_color(255, 255, 255, i));
    }
}

uint32_t get_range_color_cardinal(int a)
{
    if (a >= 80) {
        return graphics_make_color(0, 255, 64, 255);
    } else if (a >= 75) {
        return graphics_make_color(192, 255, 0, 255);
    } else if (a >= 70) {
        return graphics_make_color(255, 128, 64, 255);
    } else {
        return graphics_make_color(255, 64, 0, 255);
    }
}

uint32_t get_range_color_diagonal(int x, int y)
{
    float euclidean = sqrtf(x*x + y*y);

    return get_range_color_cardinal(euclidean / 1.125);
}

uint32_t get_angle_color(float angle)
{

    float diff = fabsf(45.0f - angle);

    if (diff < 1) {
        return graphics_make_color(0, 255, 64, 255);
    } else if (diff < 3) {
        return graphics_make_color(192, 255, 0, 255);
    } else if (diff < 5) {
        return graphics_make_color(255, 128, 64, 255);
    } else {
        return graphics_make_color(255, 64, 0, 255);
    }
}

void print_stick_angles(display_context_t ctx, struct StickAngles a)
{
    char buf[1024];
    snprintf(buf, sizeof(buf),
        "up   \n"
        "down \n"
        "left \n"
        "right\n\n"
        "UR\n\n\n"
        "UL\n\n\n"
        "DR\n\n\n"
        "DL"
    );

    int y = 15;

    graphics_set_color(COLOR_FOREGROUND, 0);

    text_set_font(FONT_MEDIUM);
    text_draw(ctx, 256, y, buf, ALIGN_LEFT);

    text_set_font(FONT_BOLD);
    int cardinals[] = {a.u.y, -a.d.y, -a.l.x, a.r.x};

    for (int i = 0; i < 4; i++) {
        snprintf(buf, sizeof(buf), "%3d", cardinals[i]);
        uint32_t c = get_range_color_cardinal(cardinals[i]);
        graphics_set_color(c, 0);
        text_draw(ctx, 249, y, buf, ALIGN_RIGHT);
        y += 10;
    }

    int diagonals[] = {
         a.ur.x,  a.ur.y,
        -a.ul.x,  a.ul.y,
         a.dr.x, -a.dr.y,
        -a.dl.x, -a.dl.y,
    };

    y += 10;

    for (int i = 0; i < 8; i += 2) {
        snprintf(buf, sizeof(buf), "%3d\n%3d", diagonals[i], diagonals[i+1]);
        uint32_t c = get_range_color_diagonal(smax(0, diagonals[i]), smax(0, diagonals[i+1]));
        graphics_set_color(c, 0);
        text_draw(ctx, 249, y, buf, ALIGN_RIGHT);
        y += 30;
    }

    float angles[] = {
        get_angle(diagonals[0], diagonals[1]),
        get_angle(diagonals[2], diagonals[3]),
        get_angle(diagonals[4], diagonals[5]),
        get_angle(diagonals[6], diagonals[7]),
    };

    y = 15 + 60;
    text_set_font(FONT_MEDIUM);

    for (int i = 0; i < 4; i++) {
        snprintf(buf, sizeof(buf), "%2.1f" SYMBOL_DEGREES, angles[i]);
        uint32_t c = get_angle_color(angles[i]);
        graphics_set_color(c, 0);
        text_draw(ctx, 256, y, buf, ALIGN_LEFT);
        y += 30;
    }

    print_cardinal_offsets(ctx, a);
}

char get_sign(int val) {
    char sign = ' ';
    if (val > 0) {
        sign = '+';
    } else if (val < 0) {
        sign = '-';
    }
    return sign;
}

void print_cardinal_offsets(display_context_t ctx, struct StickAngles a) {
    int line_height = 10,
        y_offset = 15;

    struct Vec2 *v = (struct Vec2*)&a;

    text_set_font(FONT_MEDIUM);
    graphics_set_color(graphics_make_color(192, 192, 192, 255), 0);
    char buf[128];

    snprintf(buf, sizeof(buf), "%c%d", get_sign(v[0].x), abs(v[0].x));
    text_draw(ctx, 300, y_offset + (0 * line_height), buf, ALIGN_RIGHT);

    snprintf(buf, sizeof(buf), "%c%d", get_sign(v[4].x), abs(v[4].x));
    text_draw(ctx, 300, y_offset + (1 * line_height), buf, ALIGN_RIGHT);

    snprintf(buf, sizeof(buf), "%c%d", get_sign(v[6].y), abs(v[6].y));
    text_draw(ctx, 300, y_offset + (2 * line_height), buf, ALIGN_RIGHT);

    snprintf(buf, sizeof(buf), "%c%d", get_sign(v[2].y), abs(v[2].y));
    text_draw(ctx, 300, y_offset + (3 * line_height), buf, ALIGN_RIGHT);

    graphics_set_color(COLOR_FOREGROUND, 0);
}

void test_angles(struct StickAngles *a, int testnum)
{
    const int reset_cmd = 0xFF;

    static const char *angles[] =
    {
        "Neutral",
        "Up",
        "Up-Right",
        "Right",
        "Down-Right",
        "Down",
        "Down-Left",
        "Left",
        "Up-Left",
    };

    static const char *gfx[] =
    {
        "/gfx/stick_neutral.sprite",
        "/gfx/stick_0.sprite",
        "/gfx/stick_1.sprite",
        "/gfx/stick_2.sprite",
        "/gfx/stick_3.sprite",
        "/gfx/stick_4.sprite",
        "/gfx/stick_5.sprite",
        "/gfx/stick_6.sprite",
        "/gfx/stick_7.sprite",
    };

    struct Vec2 *v = (struct Vec2*)a;

    graphics_set_color(COLOR_FOREGROUND, 0);
    text_set_line_height(11);

    for (int i = 0; i < 9; i++) {
        int f = dfs_open(gfx[i]);
        int size = dfs_size(f);
        sprite_t *stick = malloc(size);
        dfs_read(stick, size, 1, f);
        dfs_close(f);

        display_context_t ctx;
        while ((ctx = display_lock()) == 0) {}
        graphics_fill_screen(ctx, COLOR_BACKGROUND);
        graphics_draw_sprite(ctx, (320-128)/2, (240-128)/2, stick);

        char buf[128];
        snprintf(buf, sizeof(buf), "Test %d\nHold %s and press A", testnum, angles[i]);
        text_set_font(FONT_BOLD);
        text_draw(ctx, 320/2, 24, buf, ALIGN_CENTER);

        display_show(ctx);

        for (;;) {
            controller_scan();
            struct controller_data cdata = get_keys_down_filtered();
            if (cdata.c[0].A) {
                if (i > 0 ) {
                    cdata = get_keys_pressed();
                    v[i-1].x = cdata.c[0].x;
                    v[i-1].y = cdata.c[0].y;
                } else {
                    // raphnetraw needs some bytes for input or won't work,
                    // dunno about real console
                    uint8_t data[4];
                    execute_raw_command(0, reset_cmd, 0, 4, NULL, data);
                }
                break;
            }
        }

        free(stick);
    }
}

struct StickAngles find_median(struct StickAngles a[], int n)
{
    struct StickAngles median;

    for (int i = 0; i < 16; i++) {
        int sorted[n];

        for (int j = 0; j < n; j++) {
            sorted[j] = a[j].values[i];
        }

        for (int j = 0; j < n; j++) {
            int lowest_idx = j;

            for (int k = j; k < n; k++) {
                if (sorted[k] < sorted[lowest_idx]) {
                    lowest_idx = k;
                }
            }

            int tmp = sorted[j];
            sorted[j] = sorted[lowest_idx];
            sorted[lowest_idx] = tmp;
        }

        if (n % 2) {
            median.values[i] = sorted[(n-1)/2];
        } else {
            median.values[i] = (sorted[n/2-1] + sorted[n/2]) / 2;
        }
    }

    return median;
}

float find_standard_deviation(struct StickAngles a[], int n)
{
    if (n < 2) return -1;

    float values[16*n];

    for (int i = 0; i < 16; i++) {
        float mean = 0;
        float *p = &values[n*i];

        for (int j = 0; j < n; j++) {
            p[j] = a[j].values[i];
            mean += p[j];
        }

        mean = mean / (float)n;

        // the values need to be normalized so each notch's axis
        // becomes comparable to each other
        for (int j = 0; j < n; j++) {
            p[j] -= mean;
        }
    }

    float variance = 0;

    for (int i = 0; i < 16 * n; i++) {
        float v = values[i];
        variance += v * v;
    }
    variance = variance / (float)(16*n - 1);

    return sqrtf(variance);
}

int should_enable_zoomout(struct StickAngles a[], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 16; j++) {
            if (abs(a[i].values[j]) > 108) {
                return 1;
            }
        }
    }

    return 0;
}

static sprite_t *test_sprites[] = {0, 0};

static const char *test_gfx[] =
{
    "/gfx/pass.sprite",
    "/gfx/fail.sprite",
};

void print_benchmark_grade(display_context_t ctx, struct StickAngles a, int lenient, float std_dev)
{
    int cardinal_range = 0,
        cardinal_delta = 0,
        cardinal_axes = 0,
        cardinal_tests = 0,
        diagonal_delta = 0,
        diagonal_angles = 0,
        diagonal_tests = 0,
        consistency = 0,
        consistency_tests = 0,
        cardinal_min = 128,
        cardinal_max = 0,
        diagonal_min = 128,
        diagonal_max = 0,
        cardinal_range_threshold = 84,
        cardinal_delta_threshold = 5,
        cardinal_axes_threshold = 5,
        diagonal_delta_threshold = 8,
        diagonal_angles_min_threshold = 43,
        diagonal_angles_max_threshold = 47;
    float std_dev_max_threshold = 1.0;

    if (lenient) {
        int lenient_offset = 5;
        cardinal_range_threshold -= lenient_offset;
        cardinal_delta_threshold += lenient_offset;
        cardinal_axes_threshold += lenient_offset;
        diagonal_delta_threshold += lenient_offset;
        diagonal_angles_min_threshold -= (lenient_offset / 2);
        diagonal_angles_max_threshold += (lenient_offset / 2);
        std_dev_max_threshold += (lenient_offset/10);
    }

    char buf[1024];
    snprintf(buf, sizeof(buf),
        "cardinal notches:\n"
        "  values >= %d?\n"
        "  values +/- %d?\n"
        "  true to axis?\n"
        "\n"
        "diagonal notches:\n"
        "  values +/- %d?\n"
        "  angles %d-%d" SYMBOL_DEGREES "?\n"
        "\n"
        "consistency:\n"
        "  std dev < %.1f?\n",
        cardinal_range_threshold,
        cardinal_delta_threshold,
        diagonal_delta_threshold,
        diagonal_angles_min_threshold,
        diagonal_angles_max_threshold,
        std_dev_max_threshold
    );

    int line_height = 16;

    uint32_t c_gray = graphics_make_color(128, 128, 128, 255);
    for (int x = 22; x < (line_height * 13); x += line_height) {
        graphics_draw_line(
            ctx,
            20,
            x,
            140,
            x,
            c_gray);
    }

    graphics_draw_line(ctx, 20,  23,  20,  85,  c_gray);
    graphics_draw_line(ctx, 119, 23,  119, 85,  c_gray);
    graphics_draw_line(ctx, 140, 23,  140, 85,  c_gray);
    graphics_draw_line(ctx, 20,  102, 20,  150, c_gray);
    graphics_draw_line(ctx, 119, 102, 119, 150, c_gray);
    graphics_draw_line(ctx, 140, 102, 140, 150, c_gray);
    graphics_draw_line(ctx, 20,  166, 20,  198, c_gray);
    graphics_draw_line(ctx, 119, 166, 119, 198, c_gray);
    graphics_draw_line(ctx, 140, 166, 140, 198, c_gray);

    text_set_font(FONT_MEDIUM);
    text_set_line_height(line_height);
    graphics_set_color(COLOR_FOREGROUND, 0);
    text_draw(ctx, 24, 25, buf, ALIGN_LEFT);


    struct Vec2 *v = (struct Vec2*)&a;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) {                   // Cardinal notch
            if (i % 4 == 0) {               // U/D notch
                if (abs(v[i].y) < cardinal_range_threshold) { // Check range
                    cardinal_range = 1;
                }

                if (abs(v[i].y) < cardinal_min) {
                    cardinal_min = abs(v[i].y);
                }

                if (abs(v[i].y) > cardinal_max) {
                    cardinal_max = abs(v[i].y);
                }

                if (abs(v[i].x) >= cardinal_axes_threshold) { // True to axis?
                    cardinal_axes = 1;
                }
            } else {                        // L/R notch
                if (abs(v[i].x) < cardinal_range_threshold) { // Check range
                    cardinal_range = 1;
                }

                if (abs(v[i].x) < cardinal_min) {
                    cardinal_min = abs(v[i].x);
                }

                if (abs(v[i].x) > cardinal_max) {
                    cardinal_max = abs(v[i].x);
                }

                if (abs(v[i].y) >= cardinal_axes_threshold) { // True to axis?
                    cardinal_axes = 1;
                }
            }
        } else {                            // Diagonal notch
            if (abs(v[i].x < diagonal_min)) {
                diagonal_min = abs(v[i].x);
            }
            if (abs(v[i].y < diagonal_min)) {
                diagonal_min = abs(v[i].y);
            }
            if (abs(v[i].x > diagonal_max)) {
                diagonal_max = abs(v[i].x);
            }
            if (abs(v[i].y > diagonal_max)) {
                diagonal_max = abs(v[i].y);
            }
        }

    }

    if (cardinal_max - cardinal_min > cardinal_delta_threshold) {
        cardinal_delta = 1;
    }

    if (diagonal_max - diagonal_min > diagonal_delta_threshold) {
        diagonal_delta = 1;
    }

    int diagonals[] = {
         a.ur.x,  a.ur.y,
        -a.ul.x,  a.ul.y,
         a.dr.x, -a.dr.y,
        -a.dl.x, -a.dl.y,
    };

    float angles[] = {
        get_angle(diagonals[0], diagonals[1]),
        get_angle(diagonals[2], diagonals[3]),
        get_angle(diagonals[4], diagonals[5]),
        get_angle(diagonals[6], diagonals[7]),
    };

    for (int i = 0; i < 4; i++) {
        if (angles[i] < diagonal_angles_min_threshold || angles[i] > diagonal_angles_max_threshold) {
            diagonal_angles = 1;
        }
    }

    if (std_dev > std_dev_max_threshold) {
        consistency = 1;
    }

    if (cardinal_range + cardinal_delta + cardinal_axes > 0) {
        cardinal_tests = 1;
    }

    if (diagonal_delta + diagonal_angles > 0) {
        diagonal_tests = 1;
    }

    if (consistency > 0) {
        consistency_tests = 1;
    }


    graphics_draw_sprite(ctx, 124, 25 + (line_height *  0), test_sprites[cardinal_tests]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  1), test_sprites[cardinal_range]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  2), test_sprites[cardinal_delta]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  3), test_sprites[cardinal_axes]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  5), test_sprites[diagonal_tests]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  6), test_sprites[diagonal_delta]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  7), test_sprites[diagonal_angles]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height *  9), test_sprites[consistency_tests]);
    graphics_draw_sprite(ctx, 124, 25 + (line_height * 10), test_sprites[consistency]);

    graphics_set_color(COLOR_FOREGROUND, 0);
    text_set_line_height(10);

    uint32_t c_blue = graphics_make_color(0, 192, 255, 255);
    uint32_t c_green = graphics_make_color(0, 255, 0, 255);
    int zoomout = 2,
        x_origin = 194,
        y_origin = 60;
    draw_stick_angles(ctx, perfect_n64, c_green, zoomout, x_origin,  y_origin);
    draw_stick_angles(ctx, a, c_blue, zoomout, x_origin, y_origin);
    y_origin = 160;
    draw_diag_compare(ctx, a, zoomout, x_origin, y_origin);

}

void display_angles(struct StickAngles a[], int sample_count)
{
    enum Comparison current_comparison = COMP_NONE;
    display_context_t ctx;
    int current_view = 0;
    int current_measurement = 0;
    int current_example = 0;

    uint32_t c_blue = graphics_make_color(0, 192, 255, 255);
    uint32_t c_green = graphics_make_color(64, 255, 0, 255);
    uint32_t c_gray = graphics_make_color(64, 64, 64, 255);
    uint32_t c_magenta = graphics_make_color(255, 0, 192, 255);
    uint32_t c_current = c_blue;

    struct StickAngles median = find_median(a, sample_count);
    int zoomout = should_enable_zoomout(a, sample_count);
    int x_origin = 120,
        y_origin = 120;


    for (int i = 0; i < 2; i++) {
        int f = dfs_open(test_gfx[i]);
        int size = dfs_size(f);
        test_sprites[i] = malloc(size);
        dfs_read(test_sprites[i], size, 1, f);
        dfs_close(f);
    }

    text_set_line_height(10);
    for (;;) {
        while ((ctx = display_lock()) == 0) {}

        graphics_fill_screen(ctx, COLOR_BACKGROUND);

        struct StickAngles *current;
        if (current_measurement > 0) {
            current = &a[current_measurement - 1];
        } else {
            current = &median;
        }

        if (current_view != VIEW_GRADE && current_view != VIEW_GRADE_LENIENT) {
            draw_center_cross(ctx, x_origin, y_origin);
            for (int i = 0; i < sample_count; i++) {
                draw_stick_angles(ctx, a[i], c_gray, zoomout, x_origin, y_origin);
            }
            if (comparisons[current_comparison]) {
                draw_stick_angles(ctx, *comparisons[current_comparison], c_green, zoomout, x_origin, y_origin);
            }

            if (current_example > 0) {
                current = comparisons[current_example];
                c_current = c_magenta;
            } else {
                c_current = c_blue;
            }

            if (current_view > VIEW_NONE) {
                if (current_view == VIEW_NOTCH_DIAG || current_view == VIEW_NOTCH_BOTH) {
                    c_current = c_gray;
                    draw_diag_compare(ctx, *current, zoomout, x_origin, y_origin);
                }

                if (current_view == VIEW_NOTCH_CARD || current_view == VIEW_NOTCH_BOTH) {
                    draw_cardinal_compare(ctx, *current, zoomout, x_origin);
                }
            }

            draw_stick_angles(ctx, *current, c_current, zoomout, x_origin, y_origin);
        } else if (current_view == VIEW_GRADE || current_view == VIEW_GRADE_LENIENT) {
            float sd = 0;
            if (sample_count > 1) {
                sd = find_standard_deviation(a, sample_count);
            }

            if (current_view == VIEW_GRADE) {
                print_benchmark_grade(ctx, *current, 0, sd);
            } else {
                print_benchmark_grade(ctx, *current, 1, sd);
            }
        }

        print_stick_angles(ctx, *current);
        graphics_set_color(COLOR_FOREGROUND, 0);
        int y = 15 + 10*17;

        char buf[128];
        snprintf(buf, sizeof(buf), "%d", sample_count);
        text_set_font(FONT_BOLD);
        text_draw(ctx, 249, y, buf, ALIGN_RIGHT);

        text_set_font(FONT_MEDIUM);
        if (sample_count == 1) {
            text_draw(ctx, 256, y, "test", ALIGN_LEFT);
        } else {
            text_draw(ctx, 256, y, "tests", ALIGN_LEFT);
        }

        y += 10;
        if (sample_count > 1) {
            float sd = find_standard_deviation(a, sample_count);
            snprintf(buf, sizeof(buf), "%.2f", sd);

            text_set_font(FONT_BOLD);
            text_draw(ctx, 249, y, buf, ALIGN_RIGHT);

            text_set_font(FONT_MEDIUM);
            text_draw(ctx, 256, y, "std dev", ALIGN_LEFT);
        }

        if (current_view != VIEW_GRADE && current_view != VIEW_GRADE_LENIENT) {
            if (sample_count == 1) {
                current_measurement = 1;
            }
            if (current_example == 0) {
                if (current_measurement > 0) {
                    snprintf(buf, sizeof(buf), "Test %d%s",
                        current_measurement, comparison_names[current_comparison]);
                } else {
                    snprintf(buf, sizeof(buf), "Median%s",
                        comparison_names[current_comparison]);
                }
            } else {
                if (current_comparison > 0) {
                    snprintf(buf, sizeof(buf), "Example%s",
                        comparison_names[current_comparison]);
                } else {
                    snprintf(buf, sizeof(buf), "%s",
                        example_names[current_example-1]);
                }
            }

            text_draw(ctx, 120, 15, buf, ALIGN_CENTER);
        }

        if (zoomout) {
            text_draw(ctx, 16, 213, "75\% scale", ALIGN_LEFT);
        }

        text_set_font(FONT_MEDIUM);
        graphics_set_color(graphics_make_color(128, 128, 128, 255), 0);
        text_draw(ctx, 320 - 16, 213, REPO_URL, ALIGN_RIGHT);

        display_show(ctx);

        controller_scan();
        struct controller_data cdata = get_keys_down_filtered();
        if (cdata.c[0].start) {
            return;
        }

        if (cdata.c[0].L) {
            if (current_comparison == 0) {
                current_comparison = COMP_MAX - 1;
            } else {
                current_comparison--;
            }
        } else if (cdata.c[0].R) {
            current_comparison++;
            if (current_comparison >= COMP_MAX) {
                current_comparison = 0;
            }
        }

        if (cdata.c[0].left) {
            if (current_example == 0) {
                current_example = COMP_MAX - 1;
            } else {
                current_example--;
            }
        } else if (cdata.c[0].right) {
            current_example++;
            if (current_example >= COMP_MAX) {
                current_example = 0;
            }
        }

        if (cdata.c[0].up) {
            if (current_measurement <= 0) {
                current_measurement = sample_count;
            } else {
                current_measurement--;
            }
        } else if (cdata.c[0].down) {
            current_measurement++;
            if (current_measurement > sample_count) {
                current_measurement = 0;
            }
        }

        if (cdata.c[0].C_left) {
            if (current_view == 0) {
                current_view = VIEW_MAX - 1;
            } else {
                current_view--;
            }
        } else if (cdata.c[0].C_right) {
            current_view++;
            if (current_view >= VIEW_MAX) {
                current_view = 0;
            }
        }

        if (cdata.c[0].Z) {
            zoomout ^= 1;
        }
    }

    for (int i = 0; i < 2; i++) {
        free(test_sprites[i]);
    }
}
