#pragma once

extern PyObject* _PyMarshal_ReadObjectFromFile(FILE* fp);
extern PyObject* _PyMarshal_ReadLastObjectFromFile(FILE* fp);
extern long _PyMarshal_ReadLongFromFile(FILE *fp);
