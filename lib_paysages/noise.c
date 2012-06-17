#include "noise.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tools.h"

#define MAX_LEVEL_COUNT 30

struct NoiseLevel;

struct NoiseGenerator
{
    int size1;
    int size2;
    int size3;
    double height_offset;
    int level_count;
    struct NoiseLevel levels[MAX_LEVEL_COUNT];
    
    double _max_height;
};

static int _noise_pool_size;
static double* _noise_pool;

static inline double _cubicInterpolate(double* p, double x)
{
    return p[1] + 0.5 * x * (p[2] - p[0] + x * (2.0 * p[0] - 5.0 * p[1] + 4.0 * p[2] - p[3] + x * (3.0 * (p[1] - p[2]) + p[3] - p[0])));
}

void noiseInit()
{
    int i;
    
    _noise_pool_size = 1048576;
    _noise_pool = malloc(sizeof(double) * _noise_pool_size);

    for (i = 0; i < _noise_pool_size; i++)
    {
        _noise_pool[i] = toolsRandom() - 0.5;
    }
}

void noiseQuit()
{
    free(_noise_pool);
}

void noiseSave(PackStream* stream)
{
    int i;
    
    packWriteInt(stream, &_noise_pool_size);
    for (i = 0; i < _noise_pool_size; i++)
    {
        packWriteDouble(stream, _noise_pool + i);
    }
}

void noiseLoad(PackStream* stream)
{
    int i;
    
    packReadInt(stream, &_noise_pool_size);
    _noise_pool = realloc(_noise_pool, sizeof(double) * _noise_pool_size);
    for (i = 0; i < _noise_pool_size; i++)
    {
        packReadDouble(stream, _noise_pool + i);
    }
}

NoiseGenerator* noiseCreateGenerator()
{
    NoiseGenerator* result;

    /* initialize */
    result = malloc(sizeof(NoiseGenerator));
    result->size1 = 1;
    result->size2 = 1;
    result->size3 = 1;
    result->level_count = 0;
    result->height_offset = 0.0;
    
    noiseValidate(result);

    return result;
}

void noiseDeleteGenerator(NoiseGenerator* generator)
{
    free(generator);
}

void noiseSaveGenerator(PackStream* stream, NoiseGenerator* perlin)
{
    int x;

    packWriteInt(stream, &perlin->size1);
    packWriteInt(stream, &perlin->size2);
    packWriteInt(stream, &perlin->size3);
    packWriteDouble(stream, &perlin->height_offset);
    packWriteInt(stream, &perlin->level_count);

    for (x = 0; x < perlin->level_count; x++)
    {
        NoiseLevel* level = perlin->levels + x;
        
        packWriteDouble(stream, &level->scaling);
        packWriteDouble(stream, &level->height);
        packWriteDouble(stream, &level->xoffset);
        packWriteDouble(stream, &level->yoffset);
        packWriteDouble(stream, &level->zoffset);
    }
}

void noiseLoadGenerator(PackStream* stream, NoiseGenerator* perlin)
{
    int x;

    packReadInt(stream, &perlin->size1);
    packReadInt(stream, &perlin->size2);
    packReadInt(stream, &perlin->size3);
    packReadDouble(stream, &perlin->height_offset);
    packReadInt(stream, &perlin->level_count);

    for (x = 0; x < perlin->level_count; x++)
    {
        NoiseLevel* level = perlin->levels + x;
        
        packReadDouble(stream, &level->scaling);
        packReadDouble(stream, &level->height);
        packReadDouble(stream, &level->xoffset);
        packReadDouble(stream, &level->yoffset);
        packReadDouble(stream, &level->zoffset);
    }
    
    noiseValidate(perlin);
}

void noiseCopy(NoiseGenerator* source, NoiseGenerator* destination)
{
    destination->size1 = source->size1;
    destination->size2 = source->size2;
    destination->size3 = source->size3;
    destination->height_offset = source->height_offset;
    destination->level_count = source->level_count;

    memcpy(destination->levels, source->levels, sizeof(NoiseLevel) * destination->level_count);
    
    noiseValidate(destination);
}

void noiseValidate(NoiseGenerator* generator)
{
    int x;
    double max_height = generator->height_offset;

    for (x = 0; x < generator->level_count; x++)
    {
        max_height += generator->levels[x].height / 2.0;
    }

    generator->_max_height = max_height;
}

void noiseGenerateBaseNoise(NoiseGenerator* generator, int size)
{
    size = (size < 1) ? 1 : size;
    size = (size > _noise_pool_size) ? _noise_pool_size : size;

    generator->size1 = size;
    generator->size2 = (int)floor(sqrt((double)size));
    generator->size3 = (int)floor(cbrt((double)size));
}

int noiseGetBaseSize(NoiseGenerator* generator)
{
    return generator->size1;
}

double noiseGetMaxValue(NoiseGenerator* generator)
{
    return generator->_max_height;
}

int noiseGetLevelCount(NoiseGenerator* generator)
{
    return generator->level_count;
}

void noiseClearLevels(NoiseGenerator* generator)
{
    generator->level_count = 0;
    noiseValidate(generator);
}

void noiseAddLevel(NoiseGenerator* generator, NoiseLevel level)
{
    if (generator->level_count < MAX_LEVEL_COUNT)
    {
        generator->levels[generator->level_count] = level;
        generator->level_count++;
        noiseValidate(generator);
    }
}

void noiseAddLevelSimple(NoiseGenerator* generator, double scaling, double height)
{
    NoiseLevel level;

    level.scaling = scaling;
    level.height = height;
    level.xoffset = toolsRandom();
    level.yoffset = toolsRandom();
    level.zoffset = toolsRandom();

    noiseAddLevel(generator, level);
}

void noiseAddLevels(NoiseGenerator* generator, int level_count, NoiseLevel start_level, double scaling_factor, double height_factor, int randomize_offset)
{
    int i;

    for (i = 0; i < level_count; i++)
    {
        if (randomize_offset)
        {
            start_level.xoffset = toolsRandom();
            start_level.yoffset = toolsRandom();
            start_level.zoffset = toolsRandom();
        }
        noiseAddLevel(generator, start_level);
        start_level.scaling *= scaling_factor;
        start_level.height *= height_factor;
    }
}

void noiseAddLevelsSimple(NoiseGenerator* generator, int level_count, double scaling, double height)
{
    NoiseLevel level;

    level.scaling = scaling;
    level.height = height;
    noiseAddLevels(generator, level_count, level, 0.5, 0.5, 1);
}

void noiseRemoveLevel(NoiseGenerator* generator, int level)
{
    if (level >= 0 && level < generator->level_count)
    {
        if (generator->level_count > 1 && level < generator->level_count - 1)
        {
            memmove(generator->levels + level, generator->levels + level + 1, sizeof(NoiseLevel) * (generator->level_count - level - 1));
        }
        generator->level_count--;
        noiseValidate(generator);
    }
}

int noiseGetLevel(NoiseGenerator* generator, int level, NoiseLevel* params)
{
    if (level >= 0 && level < generator->level_count)
    {
        *params = generator->levels[level];
        return 1;
    }
    else
    {
        return 0;
    }
}

void noiseSetLevel(NoiseGenerator* generator, int level, NoiseLevel params)
{
    if (level >= 0 && level < generator->level_count)
    {
        generator->levels[level] = params;
        noiseValidate(generator);
    }
}

void noiseSetLevelSimple(NoiseGenerator* generator, int level, double scaling, double height)
{
    NoiseLevel params;

    params.scaling = scaling;
    params.height = height;
    params.xoffset = toolsRandom();
    params.yoffset = toolsRandom();
    params.zoffset = toolsRandom();

    noiseSetLevel(generator, level, params);
}

void noiseNormalizeHeight(NoiseGenerator* generator, double min_height, double max_height, int adjust_scaling)
{
    int level;
    double height = 0.0;
    double target_height = max_height - min_height;

    if (generator->level_count == 0)
    {
        return;
    }

    for (level = 0; level < generator->level_count; level++)
    {
        height += generator->levels[level].height;
    }
    for (level = 0; level < generator->level_count; level++)
    {
        generator->levels[level].height *= target_height / height;
        if (adjust_scaling)
        {
            generator->levels[level].scaling *= target_height / height;
        }
    }
    generator->height_offset = min_height + target_height / 2.0;
    noiseValidate(generator);
}




static inline double _get1DRawNoiseValue(NoiseGenerator* generator, double x)
{
    int size = generator->size1;

    int xbase = (int)floor(x);

    double xinternal = x - (double)xbase;

    int x0 = (xbase - 1) % size;
    if (x0 < 0)
    {
        x0 += size;
    }
    int x1 = xbase % size;
    if (x1 < 0)
    {
        x1 += size;
    }
    int x2 = (xbase + 1) % size;
    if (x2 < 0)
    {
        x2 += size;
    }
    int x3 = (xbase + 2) % size;
    if (x3 < 0)
    {
        x3 += size;
    }

    double buf_cubic_x[4];

    buf_cubic_x[0] = _noise_pool[x0 % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[x1 % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[x2 % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[x3 % _noise_pool_size];

    return _cubicInterpolate(buf_cubic_x, xinternal);
}

static inline double _get1DLevelValue(NoiseGenerator* generator, NoiseLevel* level, double x)
{
    return _get1DRawNoiseValue(generator, x / level->scaling + level->xoffset * generator->size1) * level->height;
}

double noiseGet1DLevel(NoiseGenerator* generator, int level, double x)
{
    if (level >= 0 && level < generator->level_count)
    {
        return _get1DLevelValue(generator, generator->levels + level, x);
    }
    else
    {
        return 0.0;
    }
}

double noiseGet1DTotal(NoiseGenerator* generator, double x)
{
    int level;
    double result;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        result += _get1DLevelValue(generator, generator->levels + level, x);
    }
    return result + generator->height_offset;
}

double noiseGet1DDetail(NoiseGenerator* generator, double x, double detail)
{
    int level;
    double result, height, factor;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        height = generator->levels[level].height;
        factor = 1.0;
        if (height < detail * 0.25)
        {
            break;
        }
        else if (height < detail * 0.5)
        {
            factor = (detail * 0.5 - height) / 0.25;
        }

        result += _get1DLevelValue(generator, generator->levels + level, x) * factor;
    }
    return result + generator->height_offset;
}




static inline double _get2DRawNoiseValue(NoiseGenerator* generator, double x, double y)
{
    int size = generator->size2;

    int xbase = (int)floor(x);
    int ybase = (int)floor(y);

    double xinternal = x - (double)xbase;
    double yinternal = y - (double)ybase;

    int x0 = (xbase - 1) % size;
    if (x0 < 0)
    {
        x0 += size;
    }
    int x1 = xbase % size;
    if (x1 < 0)
    {
        x1 += size;
    }
    int x2 = (xbase + 1) % size;
    if (x2 < 0)
    {
        x2 += size;
    }
    int x3 = (xbase + 2) % size;
    if (x3 < 0)
    {
        x3 += size;
    }

    int y0 = (ybase - 1) % size;
    if (y0 < 0)
    {
        y0 += size;
    }
    int y1 = ybase % size;
    if (y1 < 0)
    {
        y1 += size;
    }
    int y2 = (ybase + 1) % size;
    if (y2 < 0)
    {
        y2 += size;
    }
    int y3 = (ybase + 2) % size;
    if (y3 < 0)
    {
        y3 += size;
    }

    double buf_cubic_x[4];
    double buf_cubic_y[4];

    buf_cubic_x[0] = _noise_pool[(y0 * size + x0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y0 * size + x1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y0 * size + x2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y0 * size + x3) % _noise_pool_size];
    buf_cubic_y[0] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y1 * size + x0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y1 * size + x1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y1 * size + x2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y1 * size + x3) % _noise_pool_size];
    buf_cubic_y[1] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y2 * size + x0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y2 * size + x1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y2 * size + x2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y2 * size + x3) % _noise_pool_size];
    buf_cubic_y[2] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y3 * size + x0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y3 * size + x1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y3 * size + x2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y3 * size + x3) % _noise_pool_size];
    buf_cubic_y[3] = _cubicInterpolate(buf_cubic_x, xinternal);

    return _cubicInterpolate(buf_cubic_y, yinternal);
}

static inline double _get2DLevelValue(NoiseGenerator* generator, NoiseLevel* level, double x, double y)
{
    return _get2DRawNoiseValue(generator, x / level->scaling + level->xoffset * generator->size2, y / level->scaling + level->yoffset * generator->size2) * level->height;
}

double noiseGet2DLevel(NoiseGenerator* generator, int level, double x, double y)
{
    if (level >= 0 && level < generator->level_count)
    {
        return _get2DLevelValue(generator, generator->levels + level, x, y);
    }
    else
    {
        return 0.0;
    }
}

double noiseGet2DTotal(NoiseGenerator* generator, double x, double y)
{
    int level;
    double result;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        result += _get2DLevelValue(generator, generator->levels + level, x, y);
    }
    return result + generator->height_offset;
}

double noiseGet2DDetail(NoiseGenerator* generator, double x, double y, double detail)
{
    int level;
    double result, height, factor;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        height = generator->levels[level].height;
        factor = 1.0;
        if (height < detail * 0.25)
        {
            break;
        }
        else if (height < detail * 0.5)
        {
            factor = (detail * 0.5 - height) / 0.25;
        }

        result += _get2DLevelValue(generator, generator->levels + level, x, y) * factor;
    }
    return result + generator->height_offset;
}




static inline double _get3DRawNoiseValue(NoiseGenerator* generator, double x, double y, double z)
{
    int size = generator->size3;

    int xbase = (int)floor(x);
    int ybase = (int)floor(y);
    int zbase = (int)floor(z);

    double xinternal = x - (double)xbase;
    double yinternal = y - (double)ybase;
    double zinternal = z - (double)zbase;

    int x0 = (xbase - 1) % size;
    if (x0 < 0)
    {
        x0 += size;
    }
    int x1 = xbase % size;
    if (x1 < 0)
    {
        x1 += size;
    }
    int x2 = (xbase + 1) % size;
    if (x2 < 0)
    {
        x2 += size;
    }
    int x3 = (xbase + 2) % size;
    if (x3 < 0)
    {
        x3 += size;
    }

    int y0 = (ybase - 1) % size;
    if (y0 < 0)
    {
        y0 += size;
    }
    int y1 = ybase % size;
    if (y1 < 0)
    {
        y1 += size;
    }
    int y2 = (ybase + 1) % size;
    if (y2 < 0)
    {
        y2 += size;
    }
    int y3 = (ybase + 2) % size;
    if (y3 < 0)
    {
        y3 += size;
    }

    int z0 = (zbase - 1) % size;
    if (z0 < 0)
    {
        z0 += size;
    }
    int z1 = zbase % size;
    if (z1 < 0)
    {
        z1 += size;
    }
    int z2 = (zbase + 1) % size;
    if (z2 < 0)
    {
        z2 += size;
    }
    int z3 = (zbase + 2) % size;
    if (z3 < 0)
    {
        z3 += size;
    }

    double buf_cubic_x[4];
    double buf_cubic_y[4];
    double buf_cubic_z[4];

    buf_cubic_x[0] = _noise_pool[(y0 * size * size + x0 * size + z0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y0 * size * size + x1 * size + z0) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y0 * size * size + x2 * size + z0) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y0 * size * size + x3 * size + z0) % _noise_pool_size];
    buf_cubic_y[0] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y1 * size * size + x0 * size + z0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y1 * size * size + x1 * size + z0) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y1 * size * size + x2 * size + z0) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y1 * size * size + x3 * size + z0) % _noise_pool_size];
    buf_cubic_y[1] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y2 * size * size + x0 * size + z0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y2 * size * size + x1 * size + z0) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y2 * size * size + x2 * size + z0) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y2 * size * size + x3 * size + z0) % _noise_pool_size];
    buf_cubic_y[2] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y3 * size * size + x0 * size + z0) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y3 * size * size + x1 * size + z0) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y3 * size * size + x2 * size + z0) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y3 * size * size + x3 * size + z0) % _noise_pool_size];
    buf_cubic_y[3] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_z[0] =  _cubicInterpolate(buf_cubic_y, yinternal);

    buf_cubic_x[0] = _noise_pool[(y0 * size * size + x0 * size + z1) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y0 * size * size + x1 * size + z1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y0 * size * size + x2 * size + z1) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y0 * size * size + x3 * size + z1) % _noise_pool_size];
    buf_cubic_y[0] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y1 * size * size + x0 * size + z1) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y1 * size * size + x1 * size + z1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y1 * size * size + x2 * size + z1) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y1 * size * size + x3 * size + z1) % _noise_pool_size];
    buf_cubic_y[1] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y2 * size * size + x0 * size + z1) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y2 * size * size + x1 * size + z1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y2 * size * size + x2 * size + z1) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y2 * size * size + x3 * size + z1) % _noise_pool_size];
    buf_cubic_y[2] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y3 * size * size + x0 * size + z1) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y3 * size * size + x1 * size + z1) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y3 * size * size + x2 * size + z1) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y3 * size * size + x3 * size + z1) % _noise_pool_size];
    buf_cubic_y[3] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_z[1] =  _cubicInterpolate(buf_cubic_y, yinternal);

    buf_cubic_x[0] = _noise_pool[(y0 * size * size + x0 * size + z2) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y0 * size * size + x1 * size + z2) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y0 * size * size + x2 * size + z2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y0 * size * size + x3 * size + z2) % _noise_pool_size];
    buf_cubic_y[0] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y1 * size * size + x0 * size + z2) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y1 * size * size + x1 * size + z2) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y1 * size * size + x2 * size + z2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y1 * size * size + x3 * size + z2) % _noise_pool_size];
    buf_cubic_y[1] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y2 * size * size + x0 * size + z2) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y2 * size * size + x1 * size + z2) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y2 * size * size + x2 * size + z2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y2 * size * size + x3 * size + z2) % _noise_pool_size];
    buf_cubic_y[2] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y3 * size * size + x0 * size + z2) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y3 * size * size + x1 * size + z2) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y3 * size * size + x2 * size + z2) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y3 * size * size + x3 * size + z2) % _noise_pool_size];
    buf_cubic_y[3] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_z[2] =  _cubicInterpolate(buf_cubic_y, yinternal);

    buf_cubic_x[0] = _noise_pool[(y0 * size * size + x0 * size + z3) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y0 * size * size + x1 * size + z3) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y0 * size * size + x2 * size + z3) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y0 * size * size + x3 * size + z3) % _noise_pool_size];
    buf_cubic_y[0] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y1 * size * size + x0 * size + z3) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y1 * size * size + x1 * size + z3) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y1 * size * size + x2 * size + z3) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y1 * size * size + x3 * size + z3) % _noise_pool_size];
    buf_cubic_y[1] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y2 * size * size + x0 * size + z3) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y2 * size * size + x1 * size + z3) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y2 * size * size + x2 * size + z3) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y2 * size * size + x3 * size + z3) % _noise_pool_size];
    buf_cubic_y[2] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_x[0] = _noise_pool[(y3 * size * size + x0 * size + z3) % _noise_pool_size];
    buf_cubic_x[1] = _noise_pool[(y3 * size * size + x1 * size + z3) % _noise_pool_size];
    buf_cubic_x[2] = _noise_pool[(y3 * size * size + x2 * size + z3) % _noise_pool_size];
    buf_cubic_x[3] = _noise_pool[(y3 * size * size + x3 * size + z3) % _noise_pool_size];
    buf_cubic_y[3] = _cubicInterpolate(buf_cubic_x, xinternal);

    buf_cubic_z[3] =  _cubicInterpolate(buf_cubic_y, yinternal);

    return _cubicInterpolate(buf_cubic_z, zinternal);
}

static inline double _get3DLevelValue(NoiseGenerator* generator, NoiseLevel* level, double x, double y, double z)
{
    return _get3DRawNoiseValue(generator, x / level->scaling + level->xoffset * generator->size3, y / level->scaling + level->yoffset * generator->size3, z / level->scaling + level->zoffset * generator->size3) * level->height;
}

double noiseGet3DLevel(NoiseGenerator* generator, int level, double x, double y, double z)
{
    if (level >= 0 && level < generator->level_count)
    {
        return _get3DLevelValue(generator, generator->levels + level, x, y, z);
    }
    else
    {
        return 0.0;
    }
}

double noiseGet3DTotal(NoiseGenerator* generator, double x, double y, double z)
{
    int level;
    double result;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        result += _get3DLevelValue(generator, generator->levels + level, x, y, z);
    }
    return result + generator->height_offset;
}

double noiseGet3DDetail(NoiseGenerator* generator, double x, double y, double z, double detail)
{
    int level;
    double result, height, factor;

    result = 0.0;
    for (level = 0; level < generator->level_count; level++)
    {
        height = generator->levels[level].height;
        factor = 1.0;
        if (height < detail * 0.25)
        {
            break;
        }
        else if (height < detail * 0.5)
        {
            factor = (detail * 0.5 - height) / 0.25;
        }

        result += _get3DLevelValue(generator, generator->levels + level, x, y, z) * factor;
    }
    return result + generator->height_offset;
}
