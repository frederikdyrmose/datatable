#include "py_types.h"
#include "py_utils.h"

static PyObject *pyBoolFalse = NULL;
static PyObject *pyBoolTrue = NULL;

PyObject* py_ltype_names[DT_LTYPES_COUNT];
PyObject* py_stype_names[DT_STYPES_COUNT];
stype_formatter* py_stype_formatters[DT_STYPES_COUNT];



static PyObject* stype_boolean_i8_tostring(Column *col, int64_t row)
{
    int8_t x = ((int8_t*)col->data)[row];
    return x == 0? incref(pyBoolFalse) :
           x == 1? incref(pyBoolTrue) : none();
}

static PyObject* stype_integer_i8_tostring(Column *col, int64_t row)
{
    int8_t x = ((int8_t*)col->data)[row];
    return x == NA_I8? none() : PyLong_FromLong(x);
}

static PyObject* stype_integer_i16_tostring(Column *col, int64_t row)
{
    int16_t x = ((int16_t*)col->data)[row];
    return x == NA_I16? none() : PyLong_FromLong(x);
}

static PyObject* stype_integer_i32_tostring(Column *col, int64_t row)
{
    int32_t x = ((int32_t*)col->data)[row];
    return x == NA_I32? none() : PyLong_FromLong(x);
}

static PyObject* stype_integer_i64_tostring(Column *col, int64_t row)
{
    int64_t x = ((int64_t*)col->data)[row];
    return x == NA_I64? none() : PyLong_FromLongLong(x);
}

static PyObject* stype_real_f32_tostring(Column *col, int64_t row)
{
    float x = ((float*)col->data)[row];
    return x == NA_F32? none() : PyFloat_FromDouble((double)x);
}

static PyObject* stype_real_f64_tostring(Column *col, int64_t row)
{
    double x = ((double*)col->data)[row];
    return x == NA_F64? none() : PyFloat_FromDouble(x);
}

static PyObject* stype_real_i16_tostring(Column *col, int64_t row)
{
    int16_t x = ((int16_t*)col->data)[row];
    if (x == NA_I16) return none();
    DecimalMeta *meta = col->meta;
    double s = pow(10, meta->scale);
    return PyFloat_FromDouble(x / s);
}

static PyObject* stype_real_i32_tostring(Column *col, int64_t row)
{
    int32_t x = ((int32_t*)col->data)[row];
    if (x == NA_I32) return none();
    DecimalMeta *meta = col->meta;
    double s = pow(10, meta->scale);
    return PyFloat_FromDouble(x / s);
}

static PyObject* stype_real_i64_tostring(Column *col, int64_t row)
{
    int64_t x = ((int64_t*)col->data)[row];
    if (x == NA_I64) return none();
    DecimalMeta *meta = col->meta;
    double s = pow(10, meta->scale);
    return PyFloat_FromDouble(x / s);
}

static PyObject* stype_vchar_i32_tostring(Column *col, int64_t row)
{
    int32_t offoff = (int32_t) ((VarcharMeta*) col->meta)->offoff;
    int32_t *offsets = (int32_t*)(col->data + offoff);
    if (offsets[row] < 0)
        return none();
    int32_t start = row == 0? 0 : abs(offsets[row - 1]) - 1;
    int32_t len = offsets[row] - 1 - start;
    return PyUnicode_FromStringAndSize(col->data + start, len);
}

static PyObject* stype_object_pyptr_tostring(Column *col, int64_t row)
{
    return incref(((PyObject**)col->data)[row]);
}

static PyObject* stype_notimpl(Column *col, int64_t row)
{
    PyErr_Format(PyExc_NotImplementedError,
                 "Cannot stringify column of type %d", col->stype);
    return NULL;
}


int init_py_types(PyObject *module)
{
    init_types();

    pyBoolFalse = PyLong_FromLong(0);
    pyBoolTrue  = PyLong_FromLong(1);
    if (pyBoolFalse == NULL || pyBoolTrue == NULL) return 0;

    py_ltype_names[DT_MU]       = PyUnicode_FromString("mu");
    py_ltype_names[DT_BOOLEAN]  = PyUnicode_FromString("bool");
    py_ltype_names[DT_INTEGER]  = PyUnicode_FromString("int");
    py_ltype_names[DT_REAL]     = PyUnicode_FromString("real");
    py_ltype_names[DT_STRING]   = PyUnicode_FromString("str");
    py_ltype_names[DT_DATETIME] = PyUnicode_FromString("time");
    py_ltype_names[DT_DURATION] = PyUnicode_FromString("duration");
    py_ltype_names[DT_OBJECT]   = PyUnicode_FromString("obj");

    for (int i = 0; i < DT_LTYPES_COUNT; i++) {
        if (py_ltype_names[i] == NULL) return 0;
    }

    for (int i = 0; i < DT_STYPES_COUNT; i++) {
        py_stype_names[i] = PyUnicode_FromString(stype_info[i].code);
        if (py_stype_names[i] == NULL) return 0;
    }

    py_stype_formatters[DT_VOID]               = stype_notimpl;
    py_stype_formatters[DT_BOOLEAN_I8]         = stype_boolean_i8_tostring;
    py_stype_formatters[DT_INTEGER_I8]         = stype_integer_i8_tostring;
    py_stype_formatters[DT_INTEGER_I16]        = stype_integer_i16_tostring;
    py_stype_formatters[DT_INTEGER_I32]        = stype_integer_i32_tostring;
    py_stype_formatters[DT_INTEGER_I64]        = stype_integer_i64_tostring;
    py_stype_formatters[DT_REAL_F32]           = stype_real_f32_tostring;
    py_stype_formatters[DT_REAL_F64]           = stype_real_f64_tostring;
    py_stype_formatters[DT_REAL_I16]           = stype_real_i16_tostring;
    py_stype_formatters[DT_REAL_I32]           = stype_real_i32_tostring;
    py_stype_formatters[DT_REAL_I64]           = stype_real_i64_tostring;
    py_stype_formatters[DT_STRING_I32_VCHAR]   = stype_vchar_i32_tostring;
    py_stype_formatters[DT_STRING_I64_VCHAR]   = stype_notimpl;
    py_stype_formatters[DT_STRING_FCHAR]       = stype_notimpl;
    py_stype_formatters[DT_STRING_U8_ENUM]     = stype_notimpl;
    py_stype_formatters[DT_STRING_U16_ENUM]    = stype_notimpl;
    py_stype_formatters[DT_STRING_U32_ENUM]    = stype_notimpl;
    py_stype_formatters[DT_DATETIME_I64_EPOCH] = stype_notimpl;
    py_stype_formatters[DT_DATETIME_I64_PRTMN] = stype_notimpl;
    py_stype_formatters[DT_DATETIME_I32_TIME]  = stype_notimpl;
    py_stype_formatters[DT_DATETIME_I32_DATE]  = stype_notimpl;
    py_stype_formatters[DT_DATETIME_I16_MONTH] = stype_notimpl;
    py_stype_formatters[DT_OBJECT_PYPTR]       = stype_object_pyptr_tostring;

    return 1;
}
