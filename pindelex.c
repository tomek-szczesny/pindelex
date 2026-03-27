#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vector_plot.h"

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s [-w WIDTH] [-h HEIGHT]\n", program_name);
    fprintf(stderr, "  WIDTH and HEIGHT in mm (default: 110mm)\n");
    fprintf(stderr, "  Range: 80-150mm, rounded to nearest 5mm\n");
}

float round_to_5mm(float value) {
    return roundf(value / 5.0f) * 5.0f;
}

int parse_arguments(int argc, char *argv[], float *w, float *h) {
    *w = 110.0f;
    *h = 110.0f;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -w requires a value\n");
                return 0;
            }
            *w = atof(argv[++i]);
        } else if (strcmp(argv[i], "-h") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: -h requires a value\n");
                return 0;
            }
            *h = atof(argv[++i]);
        } else {
            fprintf(stderr, "Error: unknown option '%s'\n", argv[i]);
            return 0;
        }
    }

    *w = round_to_5mm(*w);
    *h = round_to_5mm(*h);

    if (*w < 80.0f || *w > 150.0f) {
        fprintf(stderr, "Error: width %.0fmm out of range [80-150mm]\n", *w);
        return 0;
    }
    if (*h < 80.0f || *h > 150.0f) {
        fprintf(stderr, "Error: height %.0fmm out of range [80-150mm]\n", *h);
        return 0;
    }

    printf("Final dimensions: width=%.0fmm, height=%.0fmm\n", *w, *h);
    return 1;
}

int main(int argc, char *argv[]) {

    float w, h;     // Total width and height
    float fh = 65;  // Flap height
    float re = 20;  // Rounded edge diameter
    float rs = 15;  // Diameter of a rounded portion of the stitching path
    float rh = 0.6; // Radius of a stitching hole
    int rs_num = 6; // Number of stitching holes along the 90deg path arc
    int holes = 0;  // Stitching holes counter, we'll use it later
    
    if (!parse_arguments(argc, argv, &w, &h)) {
        print_usage(argv[0]);
        return 1;
    }

    plot_t front = create_svg("pindelex_front.svg", w, h);

    /* Common settings */
    cairo_set_line_cap      (front.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (front.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (front.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (front.cr, 0.2);

    // Actual design
    // The outline
    cairo_move_to           (front.cr,   0.0,   0.0);
    cairo_line_to           (front.cr,   0.0,  h-re);
    cairo_arc_negative      (front.cr,    re,  h-re,   re, M_PI, M_PI / 2);
    cairo_line_to           (front.cr,  w-re,     h);
    cairo_arc_negative      (front.cr,  w-re,  h-re,   re, M_PI/2, 0);
    cairo_line_to           (front.cr,     w,   0.0);
    cairo_close_path        (front.cr);
    cairo_stroke            (front.cr);

    // Stitching holes
    // Left and right side
    for (int i = 0; i < ((h - re) / 5); i++) {
        float y = 0.0f + (i * 5.0f);
        cairo_arc           (front.cr,   5, y, rh, 0, 2 * M_PI);
        cairo_stroke        (front.cr);
        cairo_arc           (front.cr, w-5, y, rh, 0, 2 * M_PI);
        cairo_stroke        (front.cr);
        holes += 2;
    }
    // Bottom
    for (int i = 1; i < ((w - re - re) / 5); i++) {
        float x = re + (i * 5.0f);
        cairo_arc           (front.cr,   x, h-5, rh, 0, 2 * M_PI);
        cairo_stroke        (front.cr);
        holes++;
    }
    // Arcs
    for (int i = 0; i < rs_num; i++) {
        float angle = M_PI - (i * M_PI / (2.0f * (rs_num - 1)));
        float x =       re + (re - 5.0f) * cosf(angle);
        float y = (h - re) + (re - 5.0f) * sinf(angle);
        cairo_arc           (front.cr,   x, y, rh, 0, 2 * M_PI);
        cairo_stroke        (front.cr);
        cairo_arc           (front.cr, w-x, y, rh, 0, 2 * M_PI);
        cairo_stroke        (front.cr);
        holes += 2;
    }
    // Cutout for a magnetic lock
    cairo_move_to           (front.cr, (w/2)-4,   8.0);
    cairo_rel_line_to       (front.cr,     0.0,   4.0);
    cairo_stroke            (front.cr);
    cairo_move_to           (front.cr, (w/2)+4,   8.0);
    cairo_rel_line_to       (front.cr,     0.0,   4.0);
    cairo_stroke            (front.cr);

    close_plot(&front);

// ------------------------------------------------------------------------- //

    plot_t back = create_svg("pindelex_back.svg", w, h+fh);
    cairo_translate         (back.cr,   0.0,   fh);

    /* Common settings */
    cairo_set_line_cap      (back.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (back.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (back.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (back.cr, 0.2);

    // Actual design
    // The outline
    cairo_move_to           (back.cr,    0.0,   0.0);
    cairo_line_to           (back.cr,    0.0,  h-re);
    cairo_arc_negative      (back.cr,     re,  h-re,   re, M_PI, M_PI / 2);
    cairo_line_to           (back.cr,   w-re,     h);
    cairo_arc_negative      (back.cr,   w-re,  h-re,   re, M_PI/2, 0);
    cairo_line_to           (back.cr,      w, 20-fh);
    cairo_curve_to          (back.cr,      w, -fh, w/2+10,    -fh, w/2+10,  -fh);
    cairo_line_to           (back.cr, w/2-10,   -fh);
    cairo_curve_to          (back.cr, w/2-10,   -fh,    0, -fh,      0, -fh+20);
    cairo_close_path        (back.cr);
    cairo_stroke            (back.cr);

    // Stitching holes
    // Left and right side
    for (int i = 0; i < ((h - re) / 5); i++) {
        float y = 0.0f + (i * 5.0f);
        cairo_arc           (back.cr,   5, y, rh, 0, 2 * M_PI);
        cairo_stroke        (back.cr);
        cairo_arc           (back.cr, w-5, y, rh, 0, 2 * M_PI);
        cairo_stroke        (back.cr);
    }
    // Bottom
    for (int i = 1; i < ((w - re - re) / 5); i++) {
        float x = re + (i * 5.0f);
        cairo_arc           (back.cr,   x, h-5, rh, 0, 2 * M_PI);
        cairo_stroke        (back.cr);
    }
    // Arcs
    for (int i = 0; i < rs_num; i++) {
        float angle = M_PI - (i * M_PI / (2.0f * (rs_num - 1)));
        float x =       re + (re - 5.0f) * cosf(angle);
        float y = (h - re) + (re - 5.0f) * sinf(angle);
        cairo_arc           (back.cr,   x,  y, rh, 0, 2 * M_PI);
        cairo_stroke        (back.cr);
        cairo_arc           (back.cr, w-x,  y, rh, 0, 2 * M_PI);
        cairo_stroke        (back.cr);
    }
    // Strap holes
    cairo_arc               (back.cr, w/2, 10,  2,  0, 2 * M_PI);
    cairo_stroke            (back.cr);
    cairo_arc               (back.cr, w/2, 45,  2,  0, 2 * M_PI);
    cairo_stroke            (back.cr);

    cairo_rectangle         (back.cr,(w/2)-7, -4, 14,  4);
    cairo_stroke            (back.cr);

    close_plot(&back);

// ------------------------------------------------------------------------- //

    float sidew = 5.0 * (holes-1);
    float sideh = 50.0;
    plot_t side = create_svg("pindelex_side.svg", sidew, sideh);

    /* Common settings */
    cairo_set_line_cap      (side.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (side.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (side.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (side.cr, 0.2);

    // Actual design
    // The outline
    cairo_rectangle         (side.cr, 0, 0, sidew, sideh);
    cairo_stroke        (side.cr);

    // Holes
    for (int i = 0; i < (holes); i++) {
        float x = 0.0f + (i * 5.0f);
        cairo_arc           (side.cr,   x,       5.0, rh, 0, 2 * M_PI);
        cairo_stroke        (side.cr);
        cairo_arc           (side.cr,   x, sideh-5.0, rh, 0, 2 * M_PI);
        cairo_stroke        (side.cr);
    }

    close_plot(&side);

// ------------------------------------------------------------------------- //

    float strapr = 6;
    float straph = 2*strapr;
    float strapw = 110 + 2*strapr;
    plot_t strap = create_svg("pindelex_strap.svg", strapw, straph);

    /* Common settings */
    cairo_set_line_cap      (strap.cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join     (strap.cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_source_rgb    (strap.cr, 0.0, 0.0, 0.0);
    cairo_set_line_width    (strap.cr, 0.2);

    // Actual design
    // The outline
    cairo_move_to           (strap.cr,  strapr, 0);
    cairo_arc_negative      (strap.cr,  strapr, strapr, strapr, 1.5 * M_PI, 0.5 * M_PI);
    cairo_line_to           (strap.cr,  strapw-strapr, straph);
    cairo_arc_negative      (strap.cr,  strapw-strapr, strapr, strapr, 0.5 * M_PI, 1.5 * M_PI);
    cairo_close_path        (strap.cr);
    cairo_stroke            (strap.cr);


    // Holes
    cairo_arc               (strap.cr,   strapr      , strapr, 2.0, 0, 2 * M_PI);
    cairo_stroke            (strap.cr);
    cairo_arc               (strap.cr,   strapr +  35, strapr, 2.0, 0, 2 * M_PI);
    cairo_stroke            (strap.cr);
    cairo_arc               (strap.cr,   strapw - strapr, strapr, 2.0, 0, 2 * M_PI);
    cairo_stroke            (strap.cr);
    cairo_arc               (strap.cr,   strapw - strapr - 35, strapr, 2.0, 0, 2 * M_PI);
    cairo_stroke            (strap.cr);

    close_plot(&strap);

    return 0;
}

