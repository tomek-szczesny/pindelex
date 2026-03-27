#ifndef VECTOR_PLOT_H
#define VECTOR_PLOT_H

#include <stdio.h>
#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-pdf.h>

typedef struct {
    FILE *f;
} stream_t;

static cairo_status_t write_to_stream(void *closure, const unsigned char *data, unsigned int length) {
    stream_t *s = (stream_t *)closure;
    if (fwrite(data, 1, length, s->f) == length) {
        return CAIRO_STATUS_SUCCESS;
    }
    return CAIRO_STATUS_WRITE_ERROR;
}

typedef struct {
    cairo_t *cr;
    cairo_surface_t *surface;
    stream_t *stream;
} plot_t;

static plot_t create_svg(const char *filename, float width, float height) {
    plot_t p;
    p.stream = malloc(sizeof(stream_t));
    p.stream->f = fopen(filename, "wb");

    p.surface = cairo_svg_surface_create_for_stream(write_to_stream, p.stream, width, height);
    cairo_svg_surface_set_document_unit(p.surface, CAIRO_SVG_UNIT_MM);
    p.cr = cairo_create(p.surface);

    return p;
}

static plot_t create_pdf(const char *filename, float width, float height) {
    plot_t p;
    p.stream = malloc(sizeof(stream_t));
    p.stream->f = fopen(filename, "wb");

    p.surface = cairo_pdf_surface_create_for_stream(write_to_stream, p.stream, width, height);
    p.cr = cairo_create(p.surface);

    return p;
}

static void close_plot(plot_t *p) {
    cairo_destroy(p->cr);
    cairo_surface_destroy(p->surface);
    fclose(p->stream->f);
    free(p->stream);
}

#endif
