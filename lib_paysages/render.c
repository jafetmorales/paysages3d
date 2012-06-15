#include "render.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "IL/il.h"
#include "IL/ilu.h"

#include "shared/types.h"
#include "color.h"
#include "system.h"

struct RenderArea
{
    RenderParams params;
    int pixel_count;
    RenderFragment* pixels;
    RenderFragment* scanline_up;
    RenderFragment* scanline_down;
    int scanline_left;
    int scanline_right;
    Color background_color;
    volatile int dirty_left;
    volatile int dirty_right;
    volatile int dirty_up;
    volatile int dirty_down;
    volatile int dirty_count;
    Mutex* lock;
    RenderCallbackStart callback_start;
    RenderCallbackDraw callback_draw;
    RenderCallbackUpdate callback_update;
};

#define RENDER_INVERSE 1
#define RENDER_WIREFRAME 1

static void _callbackStart(int width, int height, Color background) {}
static void _callbackDraw(int x, int y, Color col) {}
static void _callbackUpdate(float progress) {}

void renderInit()
{
}

void renderQuit()
{
}

RenderArea* renderCreateArea()
{
    RenderArea* result;
    
    result = malloc(sizeof(RenderArea));
    result->params.width = 1;
    result->params.height = 1;
    result->params.antialias = 1;
    result->params.quality = 5;
    result->pixel_count = 1;
    result->pixels = malloc(sizeof(RenderFragment));
    result->scanline_up = malloc(sizeof(RenderFragment));
    result->scanline_down = malloc(sizeof(RenderFragment));
    result->scanline_left = 0;
    result->scanline_right = 0;
    result->background_color = COLOR_TRANSPARENT;
    result->dirty_left = 1;
    result->dirty_right = -1;
    result->dirty_down = 1;
    result->dirty_up = -1;
    result->dirty_count = 0;
    result->lock = mutexCreate();
    result->callback_start = _callbackStart;
    result->callback_draw = _callbackDraw;
    result->callback_update = _callbackUpdate;
    
    return result;
}

void renderDeleteArea(RenderArea* area)
{
    mutexDestroy(area->lock);
    free(area->pixels);
    free(area->scanline_up);
    free(area->scanline_down);
    free(area);
}

void renderSetParams(RenderArea* area, RenderParams params)
{
    int width, height;
    
    width = params.width * params.antialias;
    height = params.height * params.antialias;

    area->params = params;
    area->pixels = realloc(area->pixels, sizeof(RenderFragment) * width * height);
    area->pixel_count = width * height;

    area->scanline_left = 0;
    area->scanline_right = width - 1;
    area->scanline_up = realloc(area->scanline_up, sizeof(RenderFragment) * width);
    area->scanline_down = realloc(area->scanline_down, sizeof(RenderFragment) * width);

    area->dirty_left = width;
    area->dirty_right = -1;
    area->dirty_down = height;
    area->dirty_up = -1;
    area->dirty_count = 0;

    renderClear(area);
}

void renderSetBackgroundColor(RenderArea* area, Color* col)
{
    area->background_color = *col;
}

void renderClear(RenderArea* area)
{
    RenderFragment* pixel;
    int x;
    int y;

    for (x = 0; x < area->params.width * area->params.antialias; x++)
    {
        for (y = 0; y < area->params.height * area->params.antialias; y++)
        {
            pixel = area->pixels + (y * area->params.width * area->params.antialias + x);
            pixel->z = -100000000.0;
            pixel->vertex.color = area->background_color;
        }
    }

    area->scanline_left = 0;
    area->scanline_right = area->params.width * area->params.antialias - 1;

    area->callback_start(area->params.width, area->params.height, area->background_color);

    area->dirty_left = area->params.width * area->params.antialias;
    area->dirty_right = -1;
    area->dirty_down = area->params.height * area->params.antialias;
    area->dirty_up = -1;
    area->dirty_count = 0;
}

/*static int _sortRenderFragment(void const* a, void const* b)
{
    float za, zb;
    za = ((RenderFragment*)a)->z;
    zb = ((RenderFragment*)b)->z;
    if (za > zb)
    {
        return 1;
    }
    else if (za < zb)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}*/

static inline void _setDirtyPixel(RenderArea* area, RenderFragment* fragment, int x, int y)
{
    if (x < area->dirty_left)
    {
        area->dirty_left = x;
    }
    if (x > area->dirty_right)
    {
        area->dirty_right = x;
    }
    if (y < area->dirty_down)
    {
        area->dirty_down = y;
    }
    if (y > area->dirty_up)
    {
        area->dirty_up = y;
    }

    area->dirty_count++;
}

static inline Color _getFinalPixel(RenderArea* area, int x, int y)
{
    Color result, col;
    int sx, sy;
    RenderFragment* pixel_data;
    
    result.r = result.g = result.b = 0.0;
    result.a = 1.0;
    for (sx = 0; sx < area->params.antialias; sx++)
    {
        for (sy = 0; sy < area->params.antialias; sy++)
        {
            pixel_data = area->pixels + (y * area->params.antialias + sy) * area->params.width * area->params.antialias + (x * area->params.antialias + sx);
            col = pixel_data->vertex.color;
            result.r += col.r / (float)(area->params.antialias * area->params.antialias);
            result.g += col.g / (float)(area->params.antialias * area->params.antialias);
            result.b += col.b / (float)(area->params.antialias * area->params.antialias);
        }
    }
    
    return result;
}

static void _processDirtyPixels(RenderArea* area)
{
    int x, y;
    int down, up, left, right;
    
    down = area->dirty_down / area->params.antialias;
    up = area->dirty_up / area->params.antialias;
    left = area->dirty_left / area->params.antialias;
    right = area->dirty_right / area->params.antialias;

    for (y = down; y <= up; y++)
    {
        for (x = left; x <= right; x++)
        {
            area->callback_draw(x, y, _getFinalPixel(area, x, y));
        }
    }

    area->callback_update(0.0);

    area->dirty_left = area->params.width * area->params.antialias;
    area->dirty_right = -1;
    area->dirty_down = area->params.height * area->params.antialias;
    area->dirty_up = -1;
    area->dirty_count = 0;
}

void renderUpdate(RenderArea* area)
{
    mutexAcquire(area->lock);
    _processDirtyPixels(area);
    mutexRelease(area->lock);
}

static void _setAllDirty(RenderArea* area)
{
    area->dirty_left = 0;
    area->dirty_right = area->params.width * area->params.antialias - 1;
    area->dirty_down = 0;
    area->dirty_up = area->params.height * area->params.antialias - 1;
}

void renderAddFragment(RenderArea* area, RenderFragment* fragment)
{
    RenderFragment* pixel_data;
    int x = fragment->x;
    int y = fragment->y;
    float z = fragment->z;

    if (x >= 0 && x < area->params.width * area->params.antialias && y >= 0 && y < area->params.height * area->params.antialias && z > 1.0)
    {
        pixel_data = area->pixels + (y * area->params.width * area->params.antialias + x);

        if (z > pixel_data->z)
        {
            *pixel_data = *fragment;
            _setDirtyPixel(area, pixel_data, x, y);
        }
    }
}

void renderPushFragment(RenderArea* area, int x, int y, float z, Vertex* vertex)
{
    RenderFragment fragment;

    fragment.x = x;
    fragment.y = y;
    fragment.z = z;
    fragment.vertex = *vertex;

    renderAddFragment(area, &fragment);
}

static void __vertexGetDiff(Vertex* v1, Vertex* v2, Vertex* result)
{
    result->location.x = v2->location.x - v1->location.x;
    result->location.y = v2->location.y - v1->location.y;
    result->location.z = v2->location.z - v1->location.z;
    result->color.r = v2->color.r - v1->color.r;
    result->color.g = v2->color.g - v1->color.g;
    result->color.b = v2->color.b - v1->color.b;
    result->color.a = v2->color.a - v1->color.a;
    result->callback = v1->callback;
    result->callback_data = v1->callback_data;
}

static void __vertexInterpolate(Vertex* v1, Vertex* diff, float value, Vertex* result)
{
    result->location.x = v1->location.x + diff->location.x * value;
    result->location.y = v1->location.y + diff->location.y * value;
    result->location.z = v1->location.z + diff->location.z * value;
    result->color.r = v1->color.r + diff->color.r * value;
    result->color.g = v1->color.g + diff->color.g * value;
    result->color.b = v1->color.b + diff->color.b * value;
    result->color.a = v1->color.a + diff->color.a * value;
    result->callback = v1->callback;
    result->callback_data = v1->callback_data;
}

static void __pushScanLinePoint(RenderArea* area, RenderFragment point)
{
    if (point.x < 0 || point.x >= area->params.width * area->params.antialias)
    {
        return;
    }

    if (point.x > area->scanline_right)
    {
        area->scanline_right = point.x;
        area->scanline_up[area->scanline_right] = point;
        area->scanline_down[area->scanline_right] = point;
        if (point.x < area->scanline_left)
        {
            area->scanline_left = point.x;
        }
    }
    else if (point.x < area->scanline_left)
    {
        area->scanline_left = point.x;
        area->scanline_up[area->scanline_left] = point;
        area->scanline_down[area->scanline_left] = point;
    }
    else
    {
        if (point.y > area->scanline_up[point.x].y)
        {
            area->scanline_up[point.x] = point;
        }
        if (point.y < area->scanline_down[point.x].y)
        {
            area->scanline_down[point.x] = point;
        }
    }
}

static void __pushScanLineEdge(RenderArea* area, Vector3 v1, Vector3 v2, Vertex* vertex1, Vertex* vertex2)
{
    float dx, dy, dz, fx;
    Vertex diff;
    int startx = lround(v1.x);
    int endx = lround(v2.x);
    int curx;
    RenderFragment fragment;

    if (endx < startx)
    {
        __pushScanLineEdge(area, v2, v1, vertex2, vertex1);
    }
    else if (endx < 0 || startx >= area->params.width * area->params.antialias)
    {
        return;
    }
    else if (startx == endx)
    {
        fragment.x = startx;
        fragment.y = lround(v1.y);
        fragment.z = v1.z;
        fragment.vertex = *vertex1;

        __pushScanLinePoint(area, fragment);

        fragment.x = endx;
        fragment.y = lround(v2.y);
        fragment.z = v2.z;
        fragment.vertex = *vertex2;

        __pushScanLinePoint(area, fragment);
    }
    else
    {
        if (startx < 0)
        {
            startx = 0;
        }
        if (endx >= area->params.width * area->params.antialias)
        {
            endx = area->params.width * area->params.antialias - 1;
        }

        dx = v2.x - v1.x;
        dy = v2.y - v1.y;
        dz = v2.z - v1.z;
        __vertexGetDiff(vertex1, vertex2, &diff);
        for (curx = startx; curx <= endx; curx++)
        {
            fx = (float)curx + 0.5;
            if (fx < v1.x)
            {
                fx = v1.x;
            }
            else if (fx > v2.x)
            {
                fx = v2.x;
            }
            fx = fx - v1.x;
            fragment.x = curx;
            fragment.y = lround(v1.y + dy * fx / dx);
            fragment.z = v1.z + dz * fx / dx;
            __vertexInterpolate(vertex1, &diff, fx / dx, &(fragment.vertex));

            __pushScanLinePoint(area, fragment);
        }
    }
}

static void __clearScanLines(RenderArea* area)
{
    int x;
    for (x = area->scanline_left; x <= area->scanline_right; x++)
    {
        area->scanline_up[x].y = -1;
        area->scanline_down[x].y = area->params.height * area->params.antialias;
    }
    area->scanline_left = area->params.width * area->params.antialias;
    area->scanline_right = -1;
}

static void __renderScanLines(RenderArea* area)
{
    int x, starty, endy, cury;
    Vertex diff;
    float dy, dz, fy;
    RenderFragment up, down, current;

    if (area->scanline_right > 0)
    {
        for (x = area->scanline_left; x <= area->scanline_right; x++)
        {
            up = area->scanline_up[x];
            down = area->scanline_down[x];

            starty = down.y;
            endy = up.y;

            if (endy < 0 || starty >= area->params.height * area->params.antialias)
            {
                continue;
            }

            if (starty < 0)
            {
                starty = 0;
            }
            if (endy >= area->params.height * area->params.antialias)
            {
                endy = area->params.height * area->params.antialias - 1;
            }

            dy = (float)(up.y - down.y);
            dz = up.z - down.z;
            __vertexGetDiff(&down.vertex, &up.vertex, &diff);

            current.x = x;
            for (cury = starty; cury <= endy; cury++)
            {
                fy = (float)cury - down.y;

                current.y = cury;
                current.z = down.z + dz * fy / dy;
                __vertexInterpolate(&down.vertex, &diff, fy / dy, &current.vertex);

#ifdef RENDER_WIREFRAME
                if (cury == starty || cury == endy)
                {
                    current.vertex.color = COLOR_RED;
                }
#endif

                renderAddFragment(area, &current);
            }
        }
    }
}

void renderPushTriangle(RenderArea* area, Vertex* v1, Vertex* v2, Vertex* v3, Vector3 p1, Vector3 p2, Vector3 p3)
{
    float limit_width = (float)(area->params.width * area->params.antialias - 1);
    float limit_height = (float)(area->params.height * area->params.antialias - 1);

    /* Filter if outside screen */
    if (p1.z < 1.0 || p2.z < 1.0 || p3.z < 1.0 || (p1.x < 0.0 && p2.x < 0.0 && p3.x < 0.0) || (p1.y < 0.0 && p2.y < 0.0 && p3.y < 0.0) || (p1.x > limit_width && p2.x > limit_width && p3.x > limit_width) || (p1.y > limit_height && p2.y > limit_height && p3.y > limit_height))
    {
        return;
    }

    __clearScanLines(area);
    __pushScanLineEdge(area, p1, p2, v1, v2);
    __pushScanLineEdge(area, p2, p3, v2, v3);
    __pushScanLineEdge(area, p3, p1, v3, v1);
    mutexAcquire(area->lock);
    __renderScanLines(area);
    mutexRelease(area->lock);
}

typedef struct {
    int startx;
    int endx;
    int starty;
    int endy;
    int finished;
    int interrupt;
    Thread* thread;
    RenderArea* area;
    Renderer* renderer;
} RenderChunk;

void* _renderPostProcessChunk(void* data)
{
    int x, y;
    RenderFragment* fragment;
    RenderChunk* chunk = (RenderChunk*)data;

#ifdef RENDER_INVERSE
    for (y = chunk->area->params.height * chunk->area->params.antialias - 1 - chunk->starty; y >= chunk->area->params.height * chunk->area->params.antialias - 1 - chunk->endy; y--)
#else
    for (y = chunk->starty; y <= chunk->endy; y++)
#endif
    {
        for (x = chunk->startx; x <= chunk->endx; x++)
        {
            fragment = chunk->area->pixels + (y * chunk->area->params.width * chunk->area->params.antialias + x);
            if (fragment->vertex.callback)
            {
                if (fragment->vertex.callback(fragment, chunk->renderer, fragment->vertex.callback_data))
                {
                    colorNormalize(&fragment->vertex.color);
                    _setDirtyPixel(chunk->area, fragment, x, y);
                }
            }
            /* chunk->area->progress_pixels++; */
        }
        if (chunk->interrupt)
        {
            break;
        }
    }

    chunk->finished = 1;
    return NULL;
}

#define MAX_CHUNKS 8
void renderPostProcess(RenderArea* area, Renderer* renderer, int nbchunks)
{
    volatile RenderChunk chunks[MAX_CHUNKS];
    int i;
    int x, y, dx, dy, nx, ny;
    int loops, running;

    if (nbchunks > MAX_CHUNKS)
    {
        nbchunks = MAX_CHUNKS;
    }
    if (nbchunks < 1)
    {
        nbchunks = 1;
    }

    nx = 10;
    ny = 10;
    dx = area->params.width * area->params.antialias / nx;
    dy = area->params.height * area->params.antialias / ny;
    x = 0;
    y = 0;
    /*_progress_pixels = 0;*/

    for (i = 0; i < nbchunks; i++)
    {
        chunks[i].thread = NULL;
        chunks[i].area = area;
        chunks[i].renderer = renderer;
    }

    running = 0;
    loops = 0;
    while ((y < ny && !renderer->render_interrupt) || running > 0)
    {
        timeSleepMs(100);

        for (i = 0; i < nbchunks; i++)
        {
            if (chunks[i].thread)
            {
                if (chunks[i].finished)
                {
                    threadJoin(chunks[i].thread);
                    chunks[i].thread = NULL;
                    running--;
                }
                else if (renderer->render_interrupt)
                {
                    chunks[i].interrupt = 1;
                }
            }

            if (y < ny && !chunks[i].thread && !renderer->render_interrupt)
            {
                chunks[i].finished = 0;
                chunks[i].interrupt = 0;
                chunks[i].startx = x * dx;
                if (x == nx - 1)
                {
                    chunks[i].endx = area->params.width * area->params.antialias - 1;
                }
                else
                {
                    chunks[i].endx = (x + 1) * dx - 1;
                }
                chunks[i].starty = y * dy;
                if (y == ny - 1)
                {
                    chunks[i].endy = area->params.height * area->params.antialias - 1;
                }
                else
                {
                    chunks[i].endy = (y + 1) * dy - 1;
                }

                chunks[i].thread = threadCreate(_renderPostProcessChunk, (void*)(chunks + i));
                running++;

                if (++x >= nx)
                {
                    y++;
                    x = 0;
                }
            }
        }

        if (++loops >= 10)
        {
            mutexAcquire(area->lock);
            /*_progress = (float)_progress_pixels / (float)_pixel_count;*/
            _processDirtyPixels(area);
            mutexRelease(area->lock);

            loops = 0;
        }
    }

    /*_progress = 1.0;*/
    _processDirtyPixels(area);
    area->callback_update(1.0);
}

int renderSaveToFile(RenderArea* area, const char* path)
{
    ILuint image_id;
    ilGenImages(1, &image_id);
    ilBindImage(image_id);
    Color result;
    ILuint x, y;
    ILuint rgba;
    ILuint data[area->params.height * area->params.width];
    ILenum error;
    int error_count;

    for (y = 0; y < area->params.height; y++)
    {
        for (x = 0; x < area->params.width; x++)
        {
            result = _getFinalPixel(area, x, y);
            rgba = colorTo32BitRGBA(&result);
            data[y * area->params.width + x] = rgba;
        }
    }

    ilTexImage((ILuint)area->params.width, (ILuint)area->params.height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, data);
    remove(path);
    ilSaveImage(path);

    ilDeleteImages(1, &image_id);

    error_count = 0;
    while ((error=ilGetError()) != IL_NO_ERROR)
    {
        fprintf(stderr, "IL ERROR : %s\n", iluErrorString(error));
        error_count++;
    }
    return !error_count;
}

void renderSetPreviewCallbacks(RenderArea* area, RenderCallbackStart start, RenderCallbackDraw draw, RenderCallbackUpdate update)
{
    area->callback_start = start ? start : _callbackStart;
    area->callback_draw = draw ? draw : _callbackDraw;
    area->callback_update = update ? update : _callbackUpdate;

    area->callback_start(area->params.width, area->params.height, area->background_color);

    _setAllDirty(area);
    _processDirtyPixels(area);

    area->callback_update(0.0);
}
